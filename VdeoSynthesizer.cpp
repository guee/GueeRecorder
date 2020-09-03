#include "VdeoSynthesizer.h"
#include "InputSource/ScreenLayer.h"
#include "InputSource/CameraLayer.h"
#include "InputSource/PictureLayer.h"
#include <QOffscreenSurface>
#include <QOpenGLContext>

VideoSynthesizer::VideoSynthesizer()
{
    memset(&m_params, 0, sizeof(m_params));
    memset(&m_frameData, 0, sizeof(m_frameData));
    resetDefaultOption();
}

VideoSynthesizer::~VideoSynthesizer()
{
    close();
    uninit();
}

void VideoSynthesizer::init(QOpenGLContext* shardContext)
{
    if (m_threadWorking == false)
    {
        if ( shardContext )
        {
            auto mainSur = shardContext->surface();
            m_surface = new QOffscreenSurface(nullptr, this);

//            QSurfaceFormat fmt = mainSur->format();
//            fprintf(stderr, "alphaBufferSize:%d\n"
//                            "redBufferSize:%d\n"
//                            "greenBufferSize:%d\n"
//                            "blueBufferSize:%d\n"
//                            "depthBufferSize:%d\n"
//                            "stencilBufferSize:%d\n"
//                            "colorSpace:%d\n"
//                            "hasAlpha:%d\n"
//                            "majorVersion:%d\n"
//                            "minorVersion:%d\n"
//                            "options:%d\n"
//                            "profile:%d\n"
//                            "renderableType:%d\n"
//                            "samples:%d\n"
//                            "stereo:%d\n"
//                            "swapBehavior:%d\n"
//                            "swapInterval:%d\n",
//                    fmt.alphaBufferSize(), fmt.redBufferSize(), fmt.greenBufferSize(), fmt.blueBufferSize(), fmt.depthBufferSize(), fmt.stencilBufferSize(),
//                    fmt.colorSpace(), fmt.hasAlpha(), fmt.majorVersion(), fmt.minorVersion(), fmt.options(), fmt.profile(),
//                    fmt.renderableType(), fmt.samples(), fmt.stereo(), fmt.swapBehavior(), fmt.swapInterval() );
            m_surface->setFormat(mainSur->format());
            m_surface->create();
            shardContext->doneCurrent();

            m_context = new QOpenGLContext();
            m_context->setFormat(shardContext->format());
            m_context->setShareContext(shardContext);
            m_context->create();

            if ( !m_context->makeCurrent(m_surface) )
            {
                delete m_context;
                m_context = nullptr;
                return;
            }
            loadShaderPrograms();
            m_context->doneCurrent();

            m_context->moveToThread(this);

            shardContext->makeCurrent(mainSur);

        }
        else
        {

        }

        m_threadWorking = true;
        m_frameSync.init(m_params.frameRate);
        m_frameSync.start();
        m_frameRate.start();
        start();
    }

}

void VideoSynthesizer::uninit()
{
    if (m_threadWorking)
    {
        m_threadWorking = false;
        wait();
    }
    if (m_program)
    {
        delete m_program;
        m_program = nullptr;
    }
    if (m_surface)
    {
        delete m_surface;
        m_surface = nullptr;
    }
    if (m_context)
    {
        delete m_context;
        m_context = nullptr;
    }
}

BaseLayer *VideoSynthesizer::createLayer(const QString &type)
{

    BaseLayer* layer = nullptr;
    if (type == "screen")
    {
        layer = new ScreenLayer();
    }
    else if (type == "camera")
    {
        layer = new CameraLayer();
    }
    else if (type == "picture")
    {
        layer = new PictureLayer();
    }
    if (layer)
    {
        layer->setParent(this);
        layer->setViewportSize(m_glViewportSize);
        layer->setShaderProgram(m_progPool.createProgram("base"));
    }

    return layer;
}

void VideoSynthesizer::immediateUpdate()
{
    m_immediateUpdate = true;
}

bool VideoSynthesizer::open(const QString &sourceName)
{
    Q_UNUSED(sourceName);
    if(m_status >= Opened)
    {
        return true;
    }
    m_status = Opened;
    return true;
}

void VideoSynthesizer::close()
{
    m_status = NoOpen;
}

bool VideoSynthesizer::play()
{
    if (m_status == Opened || m_status == Paused)
    {
        m_status = Palying;
        return true;
    }
    return true;
}

bool VideoSynthesizer::resetDefaultOption()
{
    m_params.encoder       = VE_X264;
    m_params.profile       = VF_BaseLine;
    m_params.presetX264    = VP_x264_VeryFast;
    m_params.presetNvenc   = VP_Nvenc_LowLatencyDefault;
    m_params.outputCSP     = Vid_CSP_I420;
    m_params.psyTune       = eTuneStillimage;
    m_params.width         = 1920;
    m_params.height        = 1080;
    m_params.frameRate     = 25.0f;
    m_params.vfr           = false;
    m_params.onlineMode    = false;
    m_params.annexb        = true;
    m_params.threadNum     = 0;
    m_params.optimizeStill = false;
    m_params.fastDecode    = false;
    m_params.rateMode      = VR_VariableBitrate;
    m_params.bitrate       = 0;
    m_params.bitrateMax    = 0;
    m_params.bitrateMin    = 0;
    m_params.vbvBuffer     = 0;
    m_params.gopMin        = 0;
    m_params.gopMax        = 0;
    m_params.BFrames       = 0;
    m_params.BFramePyramid = 0;

    m_backgroundColor = QVector4D(0.05f, 0.05f, 0.05f, 1.0f);
    return true;
}

bool VideoSynthesizer::setSize(int32_t width, int32_t height)
{
    if ( width < 16 || width > 4096 || height < 16 || height > 4096 )
        return false;
    m_params.width = width;
    m_params.height = height;

    if ( width > height )
    {
        m_glViewportSize.setHeight(2.0);
        m_glViewportSize.setWidth(width * 2.0 / height);
    }
    else
    {
        m_glViewportSize.setHeight(height * 2.0 / width);
        m_glViewportSize.setWidth(2.0);
    }
    setViewportSize(m_glViewportSize);
    return true;
}

bool VideoSynthesizer::setFrameRate(float fps)
{
    if ( fps <= 0.0f || fps > 200.0f )
        return false;
    m_params.frameRate = fps;
    m_frameSync.init(fps);
    if (m_threadWorking)
        m_frameSync.start();
    setSourcesFramerate(fps);
    return true;
}

bool VideoSynthesizer::setProfile(EVideoProfile profile)
{
    m_params.profile = profile;
    return true;
}

bool VideoSynthesizer::setPreset(EVideoPreset_x264 preset)
{
    m_params.presetX264 = preset;
    return true;
}

bool VideoSynthesizer::setCSP(EVideoCSP csp)
{
    m_params.outputCSP = csp;
    return true;
}

bool VideoSynthesizer::setPsyTune(EPsyTuneType psy)
{
    m_params.psyTune = psy;
    return true;
}

bool VideoSynthesizer::setBitrateMode(EVideoRateMode rateMode)
{
    m_params.rateMode = rateMode;
    return true;
}

bool VideoSynthesizer::setBitrate(int32_t bitrate)
{
    m_params.bitrate = bitrate;
    return true;
}

bool VideoSynthesizer::setBitrateMin(int32_t bitrateMin)
{
    m_params.bitrateMin = bitrateMin;
    return true;
}

bool VideoSynthesizer::setBitrateMax(int32_t bitrateMax)
{
    m_params.bitrateMax = bitrateMax;
    return true;
}

bool VideoSynthesizer::setVbvBuffer(int32_t vbvBufferSize)
{
    m_params.vbvBuffer = vbvBufferSize;
    return true;
}

bool VideoSynthesizer::setGopMin(int32_t gopMin)
{
    m_params.gopMin = gopMin;
    return true;
}

bool VideoSynthesizer::setGopMax(int32_t gopMax)
{
    m_params.gopMax = gopMax;
    return true;
}

bool VideoSynthesizer::setRefFrames(int refFrames)
{
    m_params.refFrames = refFrames;
    return true;
}

bool VideoSynthesizer::setBFrames(int bFrames)
{
    m_params.BFrames = bFrames;
    return true;
}

void VideoSynthesizer::renderThread()
{
    bool                    isUpdated = false;

    if ( !m_context->makeCurrent(m_surface) )
    {
        return;
    }
    m_program->bind();
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
    QOpenGLFramebufferObjectFormat fboFmt;
    fboFmt.setMipmap(false);
    fboFmt.setInternalTextureFormat(GL_RGB);    //VMWare + UOS 中，GL_RGBA8 和 GL_RGBA 的FBO纹理共享是全黑的，可能是虚拟机的问题，这里使用 GL_RGB 就可以了。
    fboFmt.setTextureTarget(GL_TEXTURE_2D);
    QOpenGLFramebufferObject* fboRgba = new QOpenGLFramebufferObject(m_params.width, m_params.height, fboFmt);
    glBindTexture(GL_TEXTURE_2D, fboRgba->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    QOpenGLFramebufferObject* fboRgba2 = new QOpenGLFramebufferObject(m_params.width, m_params.height, fboFmt);

m_status  = Palying;
    emit initDone(true);
    int64_t preTimer = 0;

    while(m_threadWorking)
    {
        if ( updateSourceTextures() ) isUpdated = true;
        m_frameData.timestamp = m_frameSync.isNextFrame();
        if ( m_frameData.timestamp - preTimer > 1000000 ) isUpdated = true;
        if ( ( m_frameData.timestamp >= 0 && isUpdated ) || m_immediateUpdate )
        {
            preTimer = m_frameData.timestamp;
            bool fromImm = m_immediateUpdate;
            fboRgba->bind();
            glViewport(0, 0, m_params.width, m_params.height);
            glClearColor(m_backgroundColor.x(), m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
            //glClearColor(1.0f, m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (auto it = m_childs.rbegin(); it != m_childs.rend(); ++it)
            {
                (*it)->draw();
            }

//            if ( m_status  == Palying )
//            {
//                if ( m_frameData.buffer == nullptr )
//                {
//                    if (!initYuvFbo())
//                        break;
//                }
//               // glFlush();
//                putFrameToEncoder(fboRgba->texture());
//            }
//            else if ( m_status < Opened && m_frameData.buffer )
//            {
//                uninitYubFbo();
//            }

            fboRgba2->bind();
            glViewport(0, 0, m_params.width, m_params.height);
            glClearColor(m_backgroundColor.x(), m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_program->bind();
            m_program->setUniformValue("qt_Texture0", 0);

            if ( m_vbo == nullptr )
            {
                m_vbo = new QOpenGLBuffer();
                m_vbo->create();
                m_vbo->bind();
                m_vbo->allocate(&m_vertex, 5 * 4 * sizeof(GLfloat));

            }
            else
            {
                m_vbo->bind();
            }


            m_program->enableAttributeArray(0);
            m_program->enableAttributeArray(1);
            m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
            m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

            QMatrix4x4 m;
            m.ortho(-1.0, 1.0, 1.0, -1.0, -1, 1);
            //m_program->bind();
            m_program->setUniformValue("qt_ModelViewProjectionMatrix",m);


            glBindTexture(GL_TEXTURE_2D, fboRgba->texture());



            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            //glFlush();


                glFlush();

//fboRgba->release();
//fboRgba2->toImage().save(QString("/home/guee/Pictures/Temp/%1.jpg").arg(m_frameData.timestamp) );
fboRgba2->toImage().save(QString("/home/guee/Pictures/YUV/%1.png").arg(m_frameData.timestamp));

            emit frameReady(fboRgba->texture());
            if ( fromImm )
            {
                m_immediateUpdate = false;
            }
            if (isUpdated)
            {
                m_frameRate.add();
                isUpdated = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    delete fboRgba;
    fboRgba = nullptr;
    uninitYubFbo();
}

void VideoSynthesizer::run()
{
    renderThread();
}

void VideoSynthesizer::loadShaderPrograms()
{
    m_progPool.addShaderFromSourceFile("base", QOpenGLShader::Vertex, ":/Shaders/base.vert" );
    m_progPool.addShaderFromSourceFile("base", QOpenGLShader::Fragment, ":/Shaders/base.frag" );

    m_progPool.addShaderFromSourceFile("SelectScreen", QOpenGLShader::Vertex, ":/Shaders/SelectScreen.vert" );
    m_progPool.addShaderFromSourceFile("SelectScreen", QOpenGLShader::Fragment, ":/Shaders/SelectScreen.frag" );

    m_progPool.addShaderFromSourceFile("RgbToYuv", QOpenGLShader::Vertex, ":/Shaders/RgbToYuv.vert" );
    m_progPool.addShaderFromSourceFile("RgbToYuv", QOpenGLShader::Fragment, ":/Shaders/RgbToYuv.frag" );


    QVector<QPair<QString, QOpenGLShader::ShaderType>> s;
    s.push_back(QPair<QString, QOpenGLShader::ShaderType>("base", QOpenGLShader::Vertex));
    s.push_back(QPair<QString, QOpenGLShader::ShaderType>("base", QOpenGLShader::Fragment));
    m_progPool.setProgramShaders("base", s);
    s.clear();
    s.push_back(QPair<QString, QOpenGLShader::ShaderType>("SelectScreen", QOpenGLShader::Vertex));
    s.push_back(QPair<QString, QOpenGLShader::ShaderType>("SelectScreen", QOpenGLShader::Fragment));
    m_progPool.setProgramShaders("SelectScreen", s);
    s.clear();
    s.push_back(QPair<QString, QOpenGLShader::ShaderType>("RgbToYuv", QOpenGLShader::Vertex));
    s.push_back(QPair<QString, QOpenGLShader::ShaderType>("RgbToYuv", QOpenGLShader::Fragment));
    m_progPool.setProgramShaders("RgbToYuv", s);

    m_program = m_progPool.createProgram("RgbToYuv");
}

bool VideoSynthesizer::initYuvFbo()
{

    switch(m_params.outputCSP)
    {
    case Vid_CSP_I420:
    case Vid_CSP_YV12:
        m_frameData.alignWidth = ( m_params.width + 1 ) / 2 * 2;
        m_frameData.alignHeight = ( m_params.height + 1 ) / 2 * 2;
        m_frameData.textureWidth = m_frameData.alignWidth;
        m_frameData.textureHeight = m_frameData.alignHeight + m_frameData.alignHeight / 2;
        m_frameData.internalFormat = GL_LUMINANCE;
        m_frameData.dateType = GL_UNSIGNED_BYTE;
        m_frameData.dataSize = m_frameData.alignWidth * m_frameData.textureHeight;
        m_frameData.planeCount = 3;
        m_frameData.stride[0] = m_frameData.alignWidth;
        m_frameData.stride[1] = m_frameData.alignWidth / 2;
        m_frameData.stride[2] = m_frameData.alignWidth / 2;
        break;
    case Vid_CSP_NV12:
    case Vid_CSP_NV21:
        m_frameData.alignWidth = ( m_params.width + 1 ) / 2 * 2;
        m_frameData.alignHeight = ( m_params.height + 1 ) / 2 * 2;
        m_frameData.textureWidth = m_frameData.alignWidth;
        m_frameData.textureHeight = m_frameData.alignHeight + m_frameData.alignHeight / 2;
        m_frameData.internalFormat = GL_LUMINANCE;
        m_frameData.dateType = GL_UNSIGNED_BYTE;
        m_frameData.dataSize = m_frameData.alignWidth * m_frameData.textureHeight;
        m_frameData.planeCount = 2;
        m_frameData.stride[0] = m_frameData.alignWidth;
        m_frameData.stride[1] = m_frameData.alignWidth;
        m_frameData.stride[2] = 0;
        break;
    case Vid_CSP_I422:
    case Vid_CSP_YV16:
        m_frameData.alignWidth = ( m_params.width + 1 ) / 2 * 2;
        m_frameData.alignHeight = m_params.height;
        m_frameData.textureWidth = m_frameData.alignWidth;
        m_frameData.textureHeight = m_frameData.alignHeight * 2;
        m_frameData.internalFormat = GL_LUMINANCE;
        m_frameData.dateType = GL_UNSIGNED_BYTE;
        m_frameData.dataSize = m_frameData.alignWidth * m_frameData.textureHeight;
        m_frameData.planeCount = 3;
        m_frameData.stride[0] = m_frameData.alignWidth;
        m_frameData.stride[1] = m_frameData.alignWidth / 2;
        m_frameData.stride[2] = m_frameData.alignWidth / 2;
        break;
    case Vid_CSP_NV16:
        m_frameData.alignWidth = ( m_params.width + 1 ) / 2 * 2;
        m_frameData.alignHeight = m_params.height;
        m_frameData.textureWidth = m_frameData.alignWidth;
        m_frameData.textureHeight = m_frameData.alignHeight * 2;
        m_frameData.internalFormat = GL_LUMINANCE;
        m_frameData.dateType = GL_UNSIGNED_BYTE;
        m_frameData.dataSize = m_frameData.alignWidth * m_frameData.textureHeight;
        m_frameData.planeCount = 2;
        m_frameData.stride[0] = m_frameData.alignWidth;
        m_frameData.stride[1] = m_frameData.alignWidth;
        m_frameData.stride[2] = 0;
        break;
    case Vid_CSP_YUY2:
    case Vid_CSP_UYVY:
        m_frameData.alignWidth = ( m_params.width + 1 ) / 2 * 2;
        m_frameData.alignHeight = m_params.height;
        m_frameData.textureWidth = m_frameData.alignWidth;
        m_frameData.textureHeight = m_frameData.alignHeight;
        m_frameData.internalFormat = GL_LUMINANCE_ALPHA;
        m_frameData.dateType = GL_UNSIGNED_BYTE;
        m_frameData.dataSize = m_frameData.alignWidth * m_frameData.textureHeight * 2;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = m_frameData.alignWidth * 2;
        m_frameData.stride[1] = 0;
        m_frameData.stride[2] = 0;
        break;
    case Vid_CSP_I444:
        m_frameData.alignWidth = m_params.width;
        m_frameData.alignHeight = m_params.height;
        m_frameData.textureWidth = m_frameData.alignWidth;
        m_frameData.textureHeight = m_frameData.alignHeight * 3;
        m_frameData.internalFormat = GL_LUMINANCE;
        m_frameData.dateType = GL_UNSIGNED_BYTE;
        m_frameData.dataSize = m_frameData.alignWidth * m_frameData.textureHeight;
        m_frameData.planeCount = 3;
        m_frameData.stride[0] = m_frameData.alignWidth;
        m_frameData.stride[1] = m_frameData.alignWidth;
        m_frameData.stride[2] = m_frameData.alignWidth;
        break;
    default:
        return false;
    }
    uninitYubFbo();
    m_frameData.csp = m_params.outputCSP;
    QOpenGLFramebufferObjectFormat fboFmt;
    fboFmt.setMipmap(false);
    fboFmt.setInternalTextureFormat(GL_RGB/*m_frameData.internalFormat*/);    //VMWare + UOS 中，GL_RGBA8 和 GL_RGBA 的FBO纹理共享是全黑的，可能是虚拟机的问题，这里使用 GL_RGB 就可以了。
    fboFmt.setTextureTarget(GL_TEXTURE_2D);
    m_frameData.fbo = new QOpenGLFramebufferObject(m_frameData.textureWidth, m_frameData.textureHeight, fboFmt);
    m_frameData.buffer = new QOpenGLBuffer(QOpenGLBuffer::PixelPackBuffer);
    if ( !m_frameData.buffer->create() )
    {
        uninitYubFbo();
        return false;
    }
    m_frameData.buffer->bind();
    m_frameData.buffer->setUsagePattern(QOpenGLBuffer::StreamRead);
    m_frameData.buffer->allocate(m_frameData.dataSize);


    m_vbo = new QOpenGLBuffer();
    if ( !m_vbo->create() )
    {
        uninitYubFbo();
        return false;
    }
    m_vbo->bind();
    m_vertex[0].vert = QVector3D(1.0, -1.0, 0.0);
    m_vertex[0].text = QVector2D(1.0, 0.0);
    m_vertex[1].vert = QVector3D(-1.0, -1.0, 0.0);
    m_vertex[1].text = QVector2D(0.0, 0.0);
    m_vertex[2].vert = QVector3D(-1.0, 1.0, 0.0);
    m_vertex[2].text = QVector2D(0.0, 1.0);
    m_vertex[3].vert = QVector3D(1.0, 1.0, 0.0);
    m_vertex[3].text = QVector2D(1.0, 1.0);
    m_vbo->allocate(&m_vertex, 5 * 4 * sizeof(GLfloat));



    return true;
}

void VideoSynthesizer::uninitYubFbo()
{
    if ( m_frameData.buffer )
    {
        delete m_frameData.buffer;
        m_frameData.buffer = nullptr;
    }
    if (m_frameData.fbo)
    {
        delete m_frameData.fbo;
        m_frameData.fbo = nullptr;
    }
    if (m_vbo)
    {
        delete m_vbo;
        m_vbo = nullptr;
    }
}

void VideoSynthesizer::putFrameToEncoder(GLuint textureId)
{
    if ( m_status < Palying || m_frameData.buffer == nullptr )
    {
        return;
    }
    uint8_t* offset = nullptr;

    m_frameData.fbo->bind();
    m_vbo->bind();
    glBindTexture(GL_TEXTURE_2D, textureId);

    m_program->bind();
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->setUniformValue("PlaneType", m_frameData.csp);
    m_program->setUniformValue("texureSize", m_frameData.textureWidth, m_frameData.textureHeight);
    m_program->setUniformValue("alignSize", m_frameData.alignWidth, m_frameData.alignHeight);
    QMatrix4x4 m;
    m.ortho(-1.0, 1.0, 1.0, -1.0, -1, 1);
    //m_program->bind();
    m_program->setUniformValue("qt_ModelViewProjectionMatrix",m);

    glViewport(0, 0, m_frameData.fbo->width(), m_frameData.fbo->height());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glFlush();
    m_frameData.buffer->bind();
    m_frameData.fbo->toImage().save(QString("/home/guee/Pictures/YUV/%1.png").arg(m_frameData.timestamp));
//    glReadPixels(0, 0, m_frameData.fbo->width(), m_frameData.fbo->height(),
//                 m_frameData.internalFormat, m_frameData.dateType, offset);

//    void* dat = m_frameData.buffer->map(QOpenGLBuffer::ReadOnly);
//    if (dat)
//    {
//        QImage img((const uchar*)dat, m_frameData.fbo->width(), m_frameData.fbo->height(), m_frameData.fbo->width(), QImage::Format_Grayscale8);
//        QString fn = QString("/home/guee/Pictures/YUV/%1.png").arg(m_frameData.timestamp);
//        img.save(fn, "png");

//        m_frameData.buffer->unmap();
//    }
 //   m_yuvBuffer->release();

}


