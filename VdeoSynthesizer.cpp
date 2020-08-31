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
        m_frameSync.init(m_params.fFrameRate);
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
    m_params.eEncoder       = VE_X264;
    m_params.eProfile       = VF_BaseLine;
    m_params.ePresetX264    = VP_x264_VeryFast;
    m_params.ePresetNvenc   = VP_Nvenc_LowLatencyDefault;
    m_params.eOutputCSP     = Vid_CSP_I420;
    m_params.ePsyTune       = eTuneStillimage;
    m_params.iWidth         = 1920;
    m_params.iHeight        = 1080;
    m_params.fFrameRate     = 25.0f;
    m_params.bVfr           = false;
    m_params.isOnlineMode   = false;
    m_params.bAnnexb        = true;
    m_params.bMaxSpeed      = true;
    m_params.bOptimizeStill = false;
    m_params.bFastDecode    = false;
    m_params.iStretchMode   = 0;
    m_params.eBitrateMode   = VR_VariableBitrate;
    m_params.iBitrate       = 0;
    m_params.iBitrateMax    = 0;
    m_params.iBitrateMin    = 0;
    m_params.iVbvBuffer     = 0;
    m_params.iGopMin        = 0;
    m_params.iGopMax        = 0;
    m_params.iBFrames       = 0;
    m_params.iBFramePyramid = 0;

    m_backgroundColor = QVector4D(0.05f, 0.05f, 0.05f, 1.0f);
    return true;
}

bool VideoSynthesizer::setSize(int32_t width, int32_t height)
{
    if ( width < 16 || width > 4096 || height < 16 || height > 4096 )
        return false;
    m_params.iWidth = width;
    m_params.iHeight = height;

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
    m_params.fFrameRate = fps;
    m_frameSync.init(fps);
    if (m_threadWorking)
        m_frameSync.start();
    setSourcesFramerate(fps);
    return true;
}

bool VideoSynthesizer::setProfile(EVideoProfile profile)
{
    m_params.eProfile = profile;
    return true;
}

bool VideoSynthesizer::setPreset(EVideoPreset_x264 preset)
{
    m_params.ePresetX264 = preset;
    return true;
}

bool VideoSynthesizer::setCSP(EVideoCSP csp)
{
    m_params.eOutputCSP = csp;
    return true;
}

bool VideoSynthesizer::setPsyTune(EPsyTuneType psy)
{
    m_params.ePsyTune = psy;
    return true;
}

bool VideoSynthesizer::setBitrateMode(EBitrateMode rateMode)
{
    m_params.eBitrateMode = rateMode;
    return true;
}

bool VideoSynthesizer::setBitrate(int32_t bitrate)
{
    m_params.iBitrate = bitrate;
    return true;
}

bool VideoSynthesizer::setBitrateMin(int32_t bitrateMin)
{
    m_params.iBitrateMin = bitrateMin;
    return true;
}

bool VideoSynthesizer::setBitrateMax(int32_t bitrateMax)
{
    m_params.iBitrateMax = bitrateMax;
    return true;
}

bool VideoSynthesizer::setVbvBuffer(int32_t vbvBufferSize)
{
    m_params.iVbvBuffer = vbvBufferSize;
    return true;
}

bool VideoSynthesizer::setGopMin(int32_t gopMin)
{
    m_params.iGopMin = gopMin;
    return true;
}

bool VideoSynthesizer::setGopMax(int32_t gopMax)
{
    m_params.iGopMax = gopMax;
    return true;
}

bool VideoSynthesizer::setRefFrames(int refFrames)
{
    m_params.iRefFrames = refFrames;
    return true;
}

bool VideoSynthesizer::setBFrames(int bFrames)
{
    m_params.iBFrames = bFrames;
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
    QOpenGLFramebufferObject* fboRgba = new QOpenGLFramebufferObject(m_params.iWidth, m_params.iHeight, fboFmt);
    glBindTexture(GL_TEXTURE_2D, fboRgba->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

//m_status  = Palying;
    emit initDone(true);
    int64_t preTimer = 0;

    while(m_threadWorking)
    {
        if ( m_status  == Palying )
        {
            if ( m_yuvBuffer == nullptr )
            {
                initYuvFbo();
            }
        }
        else if ( m_status < Opened && m_yuvBuffer )
        {
            uninitYubFbo();
        }
        if ( updateSourceTextures() ) isUpdated = true;
        m_frameData.timestamp = m_frameSync.isNextFrame();
        if ( m_frameData.timestamp - preTimer > 1000000 ) isUpdated = true;
        if ( ( m_frameData.timestamp >= 0 && isUpdated ) || m_immediateUpdate )
        {
            preTimer = m_frameData.timestamp;
            bool fromImm = m_immediateUpdate;
            fboRgba->bind();
            glViewport(0, 0, m_params.iWidth, m_params.iHeight);
            glClearColor(m_backgroundColor.x(), m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
            //glClearColor(1.0f, m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (auto it = m_childs.rbegin(); it != m_childs.rend(); ++it)
            {
                (*it)->draw();
            }
            //glBindTexture(GL_TEXTURE_2D, fboRgba->texture());


//            if ( m_status  >= Palying && m_yuvBuffer  )
//            {
//                putFrameToEncoder();
//            }
//            else
//            {
                glFlush();
//            }
//fboRgba->release();
//fboRgba->toImage().save(QString("/home/guee/Pictures/Temp/%1.jpg").arg(m_frameData.timestamp) );
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
    uninitYubFbo();
    m_frameData.csp = m_params.eOutputCSP;
    int32_t width = 0;
    int32_t height = 0;

    switch(m_frameData.csp)
    {
    case Vid_CSP_I420:
    case Vid_CSP_YV12:
        width = ( m_params.iWidth + 1 ) / 2 * 2;
        height = ( m_params.iHeight + 1 ) / 2 * 2;
        m_frameData.planeCount = 3;
        m_frameData.stride[0] = width;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_Y;
        m_planes[0].format = GL_RED;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;

        width /= 2;
        height /= 2;
        m_frameData.stride[2] = m_frameData.stride[1] = width;
        m_planes[1].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[1].dataSize = m_frameData.stride[1] * height;
        m_planes[1].planeType = m_frameData.csp == Vid_CSP_I420 ? Plane_U : Plane_V;
        m_planes[1].format = GL_RED;
        m_planes[1].dataType = GL_UNSIGNED_BYTE;

        m_planes[2].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[2].dataSize = m_frameData.stride[2] * height;
        m_planes[2].planeType = m_frameData.csp == Vid_CSP_I420 ? Plane_V : Plane_U;
        m_planes[2].format = GL_RED;
        m_planes[2].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_NV12:
    case Vid_CSP_NV21:
        width = ( m_params.iWidth + 1 ) / 2 * 2;
        height = ( m_params.iHeight + 1 ) / 2 * 2;
        m_frameData.planeCount = 2;
        m_frameData.stride[0] = width;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_Y;
        m_planes[0].format = GL_RED;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;

        m_frameData.stride[1] = width ;
        m_planes[1].fbo = new QOpenGLFramebufferObject(width / 2, height / 2, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG );
        m_planes[1].dataSize = m_frameData.stride[1] * height;
        m_planes[1].planeType = m_frameData.csp == Vid_CSP_I420 ? Plane_UV : Plane_VU;
        m_planes[1].format = GL_RG;
        m_planes[1].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_I422:
    case Vid_CSP_YV16:
        width = ( m_params.iWidth + 1 ) / 2 * 2;
        height = m_params.iHeight;
        m_frameData.planeCount = 3;
        m_frameData.stride[0] = width;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_Y;
        m_planes[0].format = GL_RED;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;

        width /= 2;
        m_frameData.stride[2] = m_frameData.stride[1] = width;
        m_planes[1].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[1].dataSize = m_frameData.stride[1] * height;
        m_planes[1].planeType = m_frameData.csp == Vid_CSP_I422 ? Plane_U : Plane_V;
        m_planes[1].format = GL_RED;
        m_planes[1].dataType = GL_UNSIGNED_BYTE;

        m_planes[2].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[2].dataSize = m_frameData.stride[2] * height;
        m_planes[2].planeType = m_frameData.csp == Vid_CSP_I422 ? Plane_V : Plane_U;
        m_planes[2].format = GL_RED;
        m_planes[2].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_NV16:
        width = ( m_params.iWidth + 1 ) / 2 * 2;
        height = m_params.iHeight;
        m_frameData.planeCount = 2;
        m_frameData.stride[0] = width;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_Y;
        m_planes[0].format = GL_RED;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;

        m_frameData.stride[1] = width;
        width /= 2;
        m_planes[1].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG );
        m_planes[1].dataSize = m_frameData.stride[1] * height;
        m_planes[1].planeType = Plane_UV;
        m_planes[1].format = GL_RG;
        m_planes[1].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_YUY2:
    case Vid_CSP_UYVY:
        width = ( m_params.iWidth + 1 ) / 2 * 2;
        height = m_params.iHeight;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = width * 2;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = m_frameData.csp == Vid_CSP_YUY2 ? Plane_YUYV : Plane_UYVY;
        m_planes[0].format = GL_RG;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_V210:
        break;
    case Vid_CSP_I444:
    case Vid_CSP_YV24:
        width = m_params.iWidth;
        height = m_params.iHeight;
        m_frameData.planeCount = 3;
        m_frameData.stride[0] = m_frameData.stride[1] = m_frameData.stride[2] = width;
        m_planes[0].dataSize = m_planes[1].dataSize = m_planes[2].dataSize = m_frameData.stride[0] * height;

        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[0].planeType = Plane_Y;
        m_planes[0].format = GL_RED;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;
        m_planes[1].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[1].planeType = m_frameData.csp == Vid_CSP_I444 ? Plane_U : Plane_V;
        m_planes[1].format = GL_RED;
        m_planes[1].dataType = GL_UNSIGNED_BYTE;
        m_planes[2].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RED );
        m_planes[2].planeType = m_frameData.csp == Vid_CSP_I444 ? Plane_V : Plane_U;
        m_planes[2].format = GL_RED;
        m_planes[2].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_AYUV:
        width = m_params.iWidth;
        height = m_params.iHeight;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = width * 4;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_BGRA );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_AYUV;
        m_planes[0].format = GL_BGRA;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_BGR:
        width = m_params.iWidth;
        height = m_params.iHeight;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = width * 3;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_BGR );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_BGR;
        m_planes[0].format = GL_BGR;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_RGB:
        width = m_params.iWidth;
        height = m_params.iHeight;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = m_frameData.stride[1] = m_frameData.stride[2] = width * 3;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGB );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_RGB;
        m_planes[0].format = GL_RGB;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_BGRA:
        width = m_params.iWidth;
        height = m_params.iHeight;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = width * 4;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_BGRA );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_BGRA;
        m_planes[0].format = GL_BGRA;
        m_planes[0].dataType = GL_UNSIGNED_BYTE;
        break;
    case Vid_CSP_BGRA10:
        width = m_params.iWidth;
        height = m_params.iHeight;
        m_frameData.planeCount = 1;
        m_frameData.stride[0] = width * 4;
        m_planes[0].fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_BGRA );
        m_planes[0].dataSize = m_frameData.stride[0] * height;
        m_planes[0].planeType = Plane_BGRA;
        m_planes[0].format = GL_BGRA;
        m_planes[0].dataType = GL_UNSIGNED_INT_2_10_10_10_REV;
        break;
    }
    if ( (m_planes[0].fbo && !m_planes[0].fbo->isValid()) ||
         (m_planes[1].fbo && !m_planes[1].fbo->isValid()) ||
         (m_planes[2].fbo && !m_planes[2].fbo->isValid()) )
    {
        uninitYubFbo();
        return false;
    }
    m_yuvBuffer = new QOpenGLBuffer(QOpenGLBuffer::PixelPackBuffer);
    if ( !m_yuvBuffer->create() )
    {
        uninitYubFbo();
        return false;
    }
    int32_t bufSize = m_planes[0].dataSize + m_planes[1].dataSize + m_planes[2].dataSize;
    m_yuvBuffer->bind();
    m_yuvBuffer->setUsagePattern(QOpenGLBuffer::StreamRead);
    m_yuvBuffer->allocate(bufSize);

    m_vertex[0].text.setX( m_planes[0].fbo->width() * 1.0f / m_params.iWidth );
    m_vertex[3].text.setX( m_planes[0].fbo->width() * 1.0f / m_params.iWidth );
    m_vertex[2].text.setY( m_planes[0].fbo->height() * 1.0f / m_params.iHeight );
    m_vertex[3].text.setY( m_planes[0].fbo->height() * 1.0f / m_params.iHeight );

    m_vbo = new QOpenGLBuffer();
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(&m_vertex, 5 * 4 * sizeof(GLfloat));
  //  m_vbo->bind();
  //  m_vbo->write(0, &m_vertex, 5 * 4 * sizeof(GLfloat));
}

void VideoSynthesizer::uninitYubFbo()
{
    for ( int32_t i = 0; i < 3; ++i  )
    {
        if (m_planes[i].fbo)
        {
            delete m_planes[i].fbo;
        }
    }
    memset(&m_planes, 0, sizeof(m_planes));
    if ( m_yuvBuffer )
    {
        delete m_yuvBuffer;
        m_yuvBuffer = nullptr;
    }
    if (m_vbo)
    {
        delete m_vbo;
        m_vbo = nullptr;
    }
    memset(&m_frameData, 0, sizeof(m_frameData));
}

void VideoSynthesizer::putFrameToEncoder()
{
    if ( m_status < Palying || m_yuvBuffer == nullptr )
    {
        return;
    }
    m_program->bind();
    m_program->setUniformValue("qt_Texture0",0);
    m_vbo->bind();

    uint8_t* offset = nullptr;
    for ( int32_t i = 0; i < 3; ++i )
    {
        if ( m_planes[i].fbo )
        {
            m_planes[i].fbo->bind();
            glViewport(0, 0, m_planes[i].fbo->width(), m_planes[i].fbo->height());
            m_program->setUniformValue("PlaneType", m_planes[i].planeType);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            m_yuvBuffer->bind();
            glReadPixels(0, 0, m_planes[i].fbo->width(), m_planes[i].fbo->height(), m_planes[i].format, m_planes[i].dataType, offset);
            offset += m_planes[i].dataSize;
        }
    }

    void* dat = m_yuvBuffer->map(QOpenGLBuffer::ReadOnly);
    if (dat)
    {
      //  QImage img((const uchar*)dat, m_planes[0].fbo->width(), m_yuvBuffer->size() / m_planes[0].fbo->width(), QImage::Format_Grayscale8);
      //  QString fn = QString("/home/guee/图片/[%1x%2] %3.png").arg(img.width()).arg(img.height()).arg(m_frameData.timestamp / 1000000.0);
      //  img.save(fn, "png");

        m_yuvBuffer->unmap();
    }
 //   m_yuvBuffer->release();

}
