#include "VideoSynthesizer.h"
#include "InputSource/ScreenLayer.h"
#include "InputSource/CameraLayer.h"
#include "InputSource/PictureLayer.h"

VideoSynthesizer::VideoSynthesizer()
{
    memset(&m_vidParams, 0, sizeof(m_vidParams));
    memset(&m_audParams, 0, sizeof(m_audParams));
    memset(&m_frameData, 0, sizeof(m_frameData));
    resetDefaultOption();

    m_vidEncoder.bindStream(&m_medStream);
    m_audRecorder.bindStream(&m_medStream);
}

VideoSynthesizer::~VideoSynthesizer()
{
    close(nullptr, nullptr);
    uninit();
}

void VideoSynthesizer::init(QOpenGLContext* shardContext)
{
    if (m_threadWorking == false)
    {
        if ( shardContext )
        {
            auto mainSur = shardContext->surface();
            //m_surface = new QOffscreenSurface(nullptr, dynamic_cast<QObject*>(this));
            m_surface = new QOffscreenSurface(nullptr);


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

            m_context->moveToThread(dynamic_cast<QThread*>(this));

            shardContext->makeCurrent(mainSur);

        }
        else
        {

        }

        qDebug() <<"GL_VENDOR:" << QString::fromUtf8((const char*)glGetString(GL_VENDOR));
        qDebug() <<"GL_RENDERER:" << QString::fromUtf8((const char*)glGetString(GL_RENDERER));
        qDebug() <<"GL_VERSION:" << QString::fromUtf8((const char*)glGetString(GL_VERSION));
        //qDebug() <<"GL_EXTENSIONS:" << QString::fromUtf8((const char*)glGetString(GL_EXTENSIONS));

        m_videoSizeChanged = false;
        m_threadWorking = true;
        m_frameSync.init(m_vidParams.frameRate);
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
    m_audRecorder.stopRec();
    while( childLayerCount() )
    {
        delete childLayer(0);
    }
    ScreenSource::static_uninit();
//    if (m_program)
//    {
//        delete m_program;
//        m_program = nullptr;
//    }
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
    if (!m_threadWorking) return nullptr;
    BaseLayer* layer = nullptr;
    if (type == "screen")
    {
        layer = new ScreenLayer();
    }
    else if (type == "camera")
    {
        layer = dynamic_cast<BaseLayer*>(new CameraLayer());
    }
    else if (type == "picture")
    {
        layer = new PictureLayer();
    }
    if (layer)
    {
        layer->setParent(this);
        layer->setViewportSize(m_glViewportSize, m_pixViewportSize);
        layer->setShaderProgram(m_progPool.createProgram("base"));
    }

    return layer;
}

bool VideoSynthesizer::open(const QString &sourceName)
{
    if (!m_threadWorking) return false;
    Q_UNUSED(sourceName);
    if(m_status >= Opened)
    {
        return true;
    }
    if (sourceName.isEmpty())
    {
        return false;
    }
    fprintf(stderr, "VideoSynthesizer open:%s\n", sourceName.toUtf8().data() );
    QStringList files = sourceName.split('|', QString::SkipEmptyParts);
    int64_t tim = QDateTime::currentMSecsSinceEpoch() - QDateTime(QDate(1904,1,1)).toMSecsSinceEpoch();

    for (auto fn : files)
    {
        if (fn.startsWith("rtmp://", Qt::CaseInsensitive))
        {

        }
        else if (fn.endsWith(".flv", Qt::CaseInsensitive))
        {
            GueeMediaWriterFlv* flv = new GueeMediaWriterFlv(m_medStream);
            m_writers.append(flv);
            flv->setTimeFlg(tim, tim);
            flv->setFilePath(fn.toStdString());
        }
        else if (fn.endsWith(".mp4", Qt::CaseInsensitive))
        {
            GueeMediaWriterMp4* mp4 = new GueeMediaWriterMp4(m_medStream);
            m_writers.append(mp4);
            mp4->setTimeFlg(tim, tim);
            mp4->setFilePath(fn.toStdString());
        }
        else if (fn.endsWith(".ts", Qt::CaseInsensitive))
        {
            GueeMediaWriterTs* ts = new GueeMediaWriterTs(m_medStream);
            m_writers.append(ts);
            ts->setTimeFlg(tim, tim);
            ts->setFilePath(fn.toStdString());
        }
    }
    m_vidParams.enabled = true;
    m_medStream.setVideoParams(m_vidParams);
    m_audParams.useADTS = false;
    m_audParams.encLevel = 2;
    m_medStream.setAudioParams(m_audParams);
    fprintf(stderr, "m_medStream.startParse()\n");
    if (!m_medStream.startParse())
    {
        return false;
    }
    if (m_audParams.enabled)
    {
        fprintf(stderr, "m_audRecorder.startEncode\n");
        if (!m_audRecorder.startEncode(&m_audParams))
        {
            return false;
        }
    }
            fprintf(stderr, "m_vidEncoder.startEncode\n");
    if (!m_vidEncoder.startEncode(&m_vidParams))
    {
        m_medStream.endParse();
        return false;
    }
    m_status = Opened;
            fprintf(stderr, "VideoSynthesizer.exit\n");
            return true;
}

void VideoSynthesizer::close(close_step_progress fun, void* param)
{
    FrameTimestamp  ts;
    ts.start();
    m_status = NoOpen;
    m_audRecorder.endEncode();
    if (fun) fun(param);
    qDebug() << "m_audRecorder.endEncode:" << ts.elapsed();
    m_vidEncoder.endEncode(fun, param);
    qDebug() << "m_vidEncoder.endEncode:" << ts.elapsed();
    m_medStream.endParse();
    if (fun) fun(param);
    qDebug() << "m_medStream.endParse:" << ts.elapsed();
    m_timestamp.stop();
    qDebug() << "m_timestamp.stop:" << ts.elapsed();
    for (auto w:m_writers)
    {
        delete w;
    }
    m_writers.clear();
    qDebug() << "close done:" << ts.elapsed();
}

bool VideoSynthesizer::play()
{
    if(m_status == Palying) return true;
    if(m_status == Paused)
    {
        m_audRecorder.pauseEncode(false);
        m_timestamp.start();
        m_status = Palying;
        return true;
    }
    else if (m_status == Opened)
    {
        m_audRecorder.pauseEncode(false);
        m_timestamp.start();
        m_status = Palying;
        return true;
    }
    return false;
}

bool VideoSynthesizer::pause()
{
    if(m_status == Paused) return true;
    if (m_status == Palying || m_status == Opened)
    {
        m_audRecorder.pauseEncode(true);
        m_status = Paused;
        m_timestamp.pause();
        return true;
    }
    return false;
}

bool VideoSynthesizer::resetDefaultOption()
{
    m_vidParams.encoder       = VE_X264;
    m_vidParams.profile       = VF_Auto;
    m_vidParams.presetX264    = VP_x264_SuperFast;
    m_vidParams.presetNvenc   = VP_Nvenc_LowLatencyDefault;
    m_vidParams.outputCSP     = Vid_CSP_I420;
    m_vidParams.psyTune       = eTuneNone;//eTuneAnimation;
    m_vidParams.width         = 0;
    m_vidParams.height        = 0;
    m_vidParams.frameRate     = 30.0f;
    m_vidParams.vfr           = false;
    m_vidParams.onlineMode    = false;
    m_vidParams.annexb        = true;
    m_vidParams.threadNum     = 0;
    m_vidParams.optimizeStill = false;
    m_vidParams.fastDecode    = false;

    m_vidParams.useMbInfo     = true;
    //m_vidParams.useMbInfo     = false;
    m_vidParams.rateMode      = VR_ConstantQP;
    m_vidParams.constantQP    = 23;
    m_vidParams.bitrate       = 2000;
    m_vidParams.bitrateMax    = 0;
    m_vidParams.bitrateMin    = 0;
    m_vidParams.vbvBuffer     = 0;
    m_vidParams.gopMin        = 30;
    m_vidParams.gopMax        = 300;
    m_vidParams.refFrames     = 4;
    m_vidParams.BFrames       = 0;
    m_vidParams.BFramePyramid = 0;

//    m_vidParams.BFrames       = 0;
//    m_vidParams.BFramePyramid = 0;

    m_backgroundColor = QVector4D(0.05f, 0.05f, 0.05f, 1.0f);
    setSize( 1920, 1080);
    setFrameRate(25.0f);

    m_audParams.enabled = true;
    m_audParams.eCodec = AC_AAC;
    m_audParams.isOnlineMode = true;
    m_audParams.useADTS = false;
    m_audParams.encLevel = 2;
    m_audParams.bitrate = 64;

    m_audParams.sampleBits = eSampleBit16i;
    m_audParams.sampleRate = 22050;
    m_audParams.channels = 2;
    m_audParams.channelMask = 0;

    return true;
}

bool VideoSynthesizer::setSize(int32_t width, int32_t height)
{
    if ( width < 16 || width > 5120 || height < 16 || height > 5120 )
        return false;
    if ( m_vidParams.width == width && m_vidParams.height == height)
    {
        return true;
    }
    qDebug() << "set size to:" << width << "x" << height;
    //if ()
    m_vidParams.width = width;
    m_vidParams.height = height;

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

    m_videoSizeChanged = true;
    return true;
}

bool VideoSynthesizer::setFrameRate(float fps)
{
    if ( fps <= 0.0f || fps > 200.0f )
        return false;
    m_vidParams.frameRate = fps;
    m_frameSync.init(fps);
    if (m_threadWorking)
        m_frameSync.start();
    setSourcesFramerate(fps);
    return true;
}

bool VideoSynthesizer::setProfile(EVideoProfile profile)
{
    m_vidParams.profile = profile;
    return true;
}

bool VideoSynthesizer::setPreset(EVideoPreset_x264 preset)
{
    m_vidParams.presetX264 = preset;
    return true;
}

bool VideoSynthesizer::setCSP(EVideoCSP csp)
{
    m_vidParams.outputCSP = csp;
    return true;
}

bool VideoSynthesizer::setPsyTune(EPsyTuneType psy)
{
    m_vidParams.psyTune = psy;
    return true;
}

bool VideoSynthesizer::setBitrateMode(EVideoRateMode rateMode)
{
    m_vidParams.rateMode = rateMode;
    return true;
}

bool VideoSynthesizer::setConstantQP(int32_t qp)
{
    m_vidParams.constantQP = qp;
    return true;
}

bool VideoSynthesizer::setBitrate(int32_t bitrate)
{
    m_vidParams.bitrate = bitrate;
    return true;
}

bool VideoSynthesizer::setBitrateMin(int32_t bitrateMin)
{
    m_vidParams.bitrateMin = bitrateMin;
    return true;
}

bool VideoSynthesizer::setBitrateMax(int32_t bitrateMax)
{
    m_vidParams.bitrateMax = bitrateMax;
    return true;
}

bool VideoSynthesizer::setVbvBuffer(int32_t vbvBufferSize)
{
    m_vidParams.vbvBuffer = vbvBufferSize;
    return true;
}

bool VideoSynthesizer::setGopMin(int32_t gopMin)
{
    m_vidParams.gopMin = gopMin;
    return true;
}

bool VideoSynthesizer::setGopMax(int32_t gopMax)
{
    m_vidParams.gopMax = gopMax;
    return true;
}

bool VideoSynthesizer::enableAudio(bool enable)
{
   // if (!m_threadWorking) return false;
    bool ret = false;
    m_audParams.enabled = enable;
    if (enable)
        ret = m_audRecorder.startRec(m_audParams.sampleRate, m_audParams.sampleBits, m_audParams.channels);
    else
        ret = m_audRecorder.stopRec();
    return ret;
}

bool VideoSynthesizer::setSampleBits(ESampleBits bits)
{
    bool ret = true;
    if (bits != eSampleBit16i && bits != eSampleBit32i && bits != eSampleBit32f)
    {
        return false;
    }
    if (bits != m_audParams.sampleBits)
    {
        m_audParams.sampleBits = bits;
        if (m_audParams.enabled && m_audRecorder.isOpened())
        {
            ret = enableAudio(true);
        }
    }
    return ret;
}

bool VideoSynthesizer::setSampleRate(int32_t rate)
{
    bool ret = true;
    if (rate != 11025 && rate != 22050 && rate != 44100)
    {
        return false;
    }
    if (rate != m_audParams.sampleRate)
    {
        m_audParams.sampleRate = rate;
        if (m_audParams.enabled && m_audRecorder.isOpened())
        {
            ret = enableAudio(true);
        }
    }
    return ret;
}

bool VideoSynthesizer::setChannels(int32_t channels)
{
    bool ret = true;
    if (channels != 1 && channels != 2)
    {
        return false;
    }
    if (channels != m_audParams.channels)
    {
        m_audParams.channels = channels;
        if (m_audParams.enabled && m_audRecorder.isOpened())
        {
            ret = enableAudio(true);
        }
    }
    return ret;
}

bool VideoSynthesizer::setAudioBitrate(int32_t bitrate)
{
    bool ret = true;
    bitrate = qMin(maxAudioBitrate(), bitrate);
    bitrate = qMax(minAudioBitrate(), bitrate);
    if (bitrate != m_audParams.bitrate)
    {
        m_audParams.bitrate = bitrate;
    }
    return ret;
}

int32_t VideoSynthesizer::maxAudioBitrate() const
{
    return static_cast<int32_t>((6144.0 * double(m_audParams.sampleRate) / 1024.5) / 1024);
}

int32_t VideoSynthesizer::minAudioBitrate() const
{
    return 8 * m_audParams.channels;
}

void VideoSynthesizer::enablePreview(bool enabled)
{
    m_enablePreview = enabled;
}

bool VideoSynthesizer::setRefFrames(int refFrames)
{
    if (refFrames < 0 || refFrames > 8)
        return false;
    m_vidParams.refFrames = refFrames;
    return true;
}

bool VideoSynthesizer::setBFrames(int bFrames)
{
    if (bFrames < 0 || bFrames > 8)
        return false;
    m_vidParams.BFrames = bFrames;
    return true;
}

void VideoSynthesizer::run()
{
    bool isTextureUpdated = false;

    if ( !m_context->makeCurrent(m_surface) )
    {
        return;
    }

    QOpenGLFramebufferObjectFormat fboFmt;
    fboFmt.setMipmap(false);
    fboFmt.setInternalTextureFormat(GL_RGB);    //VMWare + UOS 中，GL_RGBA8 和 GL_RGBA 的FBO纹理共享是全黑的，可能是虚拟机的问题，这里使用 GL_RGB 就可以了。
    fboFmt.setTextureTarget(GL_TEXTURE_2D);
    m_pixViewportSize.setWidth(m_vidParams.width);
    m_pixViewportSize.setHeight(m_vidParams.height);

    auto create_fbo = [&fboFmt](int width, int height)->QOpenGLFramebufferObject* {
        QOpenGLFramebufferObject* fboRgba = new QOpenGLFramebufferObject(width, height, fboFmt);
        glBindTexture(GL_TEXTURE_2D, fboRgba->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return fboRgba;
    };

    QOpenGLFramebufferObject* fboRgba1 = create_fbo(m_vidParams.width, m_vidParams.height);
    QOpenGLFramebufferObject* fboRgba2 = create_fbo(m_vidParams.width, m_vidParams.height);
    initYuvFbo();

    emit initDone(true);
    int64_t preTimer = -1000000;
    int64_t curTimer = 0;

    while(m_threadWorking)
    {
        if ( m_videoSizeChanged &&
             (m_pixViewportSize.width() != m_vidParams.width || m_pixViewportSize.height() != m_vidParams.height ))
        {
            delete fboRgba1;
            fboRgba1 = create_fbo(m_vidParams.width, m_vidParams.height);
            delete fboRgba2;
            fboRgba2 = create_fbo(m_vidParams.width, m_vidParams.height);
            m_pixViewportSize.setWidth(m_vidParams.width);
            m_pixViewportSize.setHeight(m_vidParams.height);
            setViewportSize(m_glViewportSize, m_pixViewportSize, true);

            uninitYubFbo();
            initYuvFbo();

            m_videoSizeChanged = false;
            emit frameReady(0);
        }
        isTextureUpdated |= updateSourceTextures();

        curTimer = m_frameSync.isNextFrame();
        if (curTimer >= 0)
        {
            if ( curTimer - preTimer > 1000000 ) isTextureUpdated = true;
        }
        if ((curTimer >= 0 && isTextureUpdated) || m_immediateUpdate)
        {
            m_immediateUpdate = false;
            m_frameData.timestamp = qMax(int64_t(0), m_timestamp.elapsed());
            readySourceNextImage(curTimer);
            if (isTextureUpdated)
            {
                preTimer = curTimer;
                if ( m_status  == Palying )
                {
                    putFrameToEncoder();
                }
            }

            QOpenGLFramebufferObject* swfbo = fboRgba1;
            swfbo->bind();
            glViewport(0, 0, m_vidParams.width, m_vidParams.height);
            glClearColor(m_backgroundColor.x(), m_backgroundColor.y(), m_backgroundColor.z(), m_backgroundColor.w());
            //glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            auto &cds = lockChilds();
            for ( auto it = cds.rbegin(); it!= cds.rend(); ++it)
            {
                (*it)->draw();
            }
            unlockChilds();
            glDisable(GL_BLEND);


            if (isTextureUpdated)
            {
                 drawFrameToYUV(fboRgba1, fboRgba2);
                 fboRgba1 = fboRgba2;
                 fboRgba2 = swfbo;
                 m_frameRate.add();
            }
            glFlush();
            if (m_enablePreview)
            {
                emit frameReady(swfbo->texture());
            }
            isTextureUpdated = false;
        }
        msleep(1);
    }

    delete fboRgba1;
    fboRgba1 = nullptr;
    delete fboRgba2;
    fboRgba2 = nullptr;
    uninitYubFbo();
}

void VideoSynthesizer::loadShaderPrograms()
{
    m_progPool.setProgramShaders("base", ":/Shaders/base.vert", ":/Shaders/base.frag" );
    m_progPool.setProgramShaders("SelectScreen", ":/Shaders/SelectScreen.vert", ":/Shaders/SelectScreen.frag" );
    m_progPool.setProgramShaders("RgbToYuv", ":/Shaders/RgbToYuv.vert", ":/Shaders/RgbToYuv.frag" );
    m_progPool.setProgramShaders("x264-mb", ":/Shaders/x264-mb.vert", ":/Shaders/x264-mb.frag" );

    m_program = m_progPool.createProgram("RgbToYuv");

}

bool VideoSynthesizer::drawFrameToYUV(QOpenGLFramebufferObject *fboCur, QOpenGLFramebufferObject *fboPre)
{
    m_frameData.fbo->bind();
    m_program->bind();
    m_vbo->bind();
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
    glBindTexture(GL_TEXTURE_2D, fboCur->texture());
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i  = 0; i < m_frameData.planeCount; ++i)
    {
        QRect& rt = m_frameData.rect[i];
        glViewport(rt.x(), rt.y(), rt.width(), rt.height());
        m_program->setUniformValue("PlaneType", m_frameData.planeType[i]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    m_vbo->release();
    m_program->release();
//    QString fn = QString("/home/guee/Pictures/YUV/%1.png").arg(m_frameData.timestamp);
//    m_frameData.fbo->toImage().save(fn);
    m_frameData.fbo->release();

    if (m_vidParams.useMbInfo)
    {
        m_frameData.fbo_mb->bind();
        m_frameData.prog_mb->bind();
        m_frameData.vbo_mb->bind();
        glViewport(0, 0, m_frameData.mbWidth, m_frameData.mbHeight);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboCur->texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fboPre->texture());

        m_frameData.prog_mb->enableAttributeArray(0);
        m_frameData.prog_mb->enableAttributeArray(1);
        m_frameData.prog_mb->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
        m_frameData.prog_mb->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
        static bool flip = false;
        m_frameData.prog_mb->setUniformValue("flip", flip);
        flip = !flip;
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glReadBuffer(GL_FRONT);

        m_frameData.prog_mb->release();
        m_frameData.vbo_mb->release();
        m_frameData.fbo_mb->release();
        glActiveTexture(GL_TEXTURE0);

    }
    return true;
}

bool VideoSynthesizer::initYuvFbo()
{
    uninitYubFbo();
    QOpenGLFramebufferObjectFormat fboFmt;
    fboFmt.setMipmap(false);
    fboFmt.setTextureTarget(GL_TEXTURE_2D);
    fboFmt.setInternalTextureFormat(GL_ALPHA);
    switch(m_vidParams.outputCSP)
    {
    case Vid_CSP_I420:
    case Vid_CSP_YV12:
        m_frameData.planeCount = 3;
        m_frameData.rect[0].setRect(0, 0, (m_vidParams.width + 1) / 2 * 2, (m_vidParams.height + 1) / 2 * 2);
        m_frameData.rect[1].setRect(0, m_frameData.rect[0].height(), m_frameData.rect[0].width() / 2, m_frameData.rect[0].height() / 2);
        m_frameData.rect[2].setRect(m_frameData.rect[0].width() / 2, m_frameData.rect[0].height(), m_frameData.rect[1].width(), m_frameData.rect[1].height());
        m_frameData.planeType[0] = 0;
        m_frameData.planeType[1] = 1;
        m_frameData.planeType[2] = 2;
        m_frameData.fboSize.setWidth(m_frameData.rect[0].width());
        m_frameData.fboSize.setHeight(m_frameData.rect[0].height() + m_frameData.rect[1].height());
        m_frameData.fbo = new QOpenGLFramebufferObject(m_frameData.fboSize, fboFmt);
        break;
//    case Vid_CSP_NV12:
//    case Vid_CSP_NV21:
//        m_frameData.planeCount = 2;
//        makePlanes(m_frameData.planes[0], (m_vidParams.width + 1) / 2 * 2, (m_vidParams.height + 1) / 2 * 2, 1, "RgbToY");
//        makePlanes(m_frameData.planes[1], m_frameData.planes[0].width / 2, m_frameData.planes[0].height / 2, 2, "RgbToNV");
//        break;
//    case Vid_CSP_I422:
//        m_frameData.planeCount = 3;
//        makePlanes(m_frameData.planes[0], (m_vidParams.width + 1) / 2 * 2, m_vidParams.height, 1, "RgbToY");
//        makePlanes(m_frameData.planes[1], m_frameData.planes[0].width / 2, m_vidParams.height, 1, "RgbToU");
//        makePlanes(m_frameData.planes[2], m_frameData.planes[0].width / 2, m_vidParams.height, 1, "RgbToV");
//        break;
//    case Vid_CSP_YV16:
//        m_frameData.planeCount = 2;
//        makePlanes(m_frameData.planes[0], (m_vidParams.width + 1) / 2 * 2, m_vidParams.height, 1, "RgbToY");
//        makePlanes(m_frameData.planes[1], m_frameData.planes[0].width / 2, m_vidParams.height, 2, "RgbToYV");
//        break;
//    case Vid_CSP_NV16:
//        m_frameData.planeCount = 2;
//        makePlanes(m_frameData.planes[0], (m_vidParams.width + 1) / 2 * 2, m_vidParams.height, 1, "RgbToY");
//        makePlanes(m_frameData.planes[1], m_frameData.planes[0].width / 2, m_vidParams.height, 2, "RgbToNV");
//        break;
//    case Vid_CSP_YUY2:
//    case Vid_CSP_UYVY:
//        m_frameData.planeCount = 1;
//        makePlanes(m_frameData.planes[0], (m_vidParams.width + 1) / 2 * 2, m_vidParams.height, 2, "RgbToY");
//        break;
//    case Vid_CSP_I444:
//        m_frameData.planeCount = 3;
//        makePlanes(m_frameData.planes[0], m_vidParams.width, m_vidParams.height, 1, "RgbToY");
//        makePlanes(m_frameData.planes[1], m_vidParams.width, m_vidParams.height, 1, "RgbToU");
//        makePlanes(m_frameData.planes[2], m_vidParams.width, m_vidParams.height, 1, "RgbToV");
//        break;
    default:
        return false;
    }
    m_frameData.csp = m_vidParams.outputCSP;

    m_vbo = new QOpenGLBuffer();
    if ( !m_vbo->create() )
    {
        uninitYubFbo();
        return false;
    }
    QVector4D rgbToY(+0.2568f, +0.5041f, +0.0979f, 16.0f/255.0f);
    QVector4D rgbToU(-0.1482f, -0.2910f, +0.4392f, 128.0f/255.0f);
    QVector4D rgbToV(+0.4392f, -0.3678f, -0.0714f, 128.0f/255.0f);
    QMatrix4x4 m;
    m.ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    m_program->bind();
    m_program->setUniformValue("qt_ModelViewProjectionMatrix",m);
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->setUniformValue("RGB_Y", rgbToY);
    m_program->setUniformValue("RGB_U", rgbToU);
    m_program->setUniformValue("RGB_V", rgbToV);
    //m_program->setUniformValue("imageSize", m_vidParams.width, m_vidParams.height);
    m_program->release();


    float dw = float(m_frameData.rect[0].width()) / float(m_vidParams.width);
    float dh = float(m_frameData.rect[0].height()) / float(m_vidParams.height);
    m_vertex[0].text = QVector2D(dw, 1.0f);
    m_vertex[1].text = QVector2D(0.0f, 1.0f);
    m_vertex[2].text = QVector2D(0.0f, 1.0f - dh);
    m_vertex[3].text = QVector2D(dw, 1.0f - dh);
    m_userdefOnView.setRect(-1, -1, 2.0, 2.0);
    m_vbo->bind();
    m_vbo->allocate(&m_vertex, 5 * 4 * sizeof(GLfloat));
    m_vbo->release();



    if (m_vidParams.useMbInfo)
    {
        fboFmt.setInternalTextureFormat(GL_ALPHA);
        //fboFmt.setInternalTextureFormat(GL_LUMINANCE);
        m_frameData.mbWidth = (m_vidParams.width + 15) / 16;
        m_frameData.mbHeight   = (m_vidParams.height + 15) / 16;
        m_frameData.mbPitch = (m_frameData.mbWidth + 3) / 4 * 4;
        m_frameData.fbo_mb = new QOpenGLFramebufferObject(m_frameData.mbWidth, m_frameData.mbHeight, fboFmt);
        m_frameData.buf_mb = new uint8_t[m_frameData.mbPitch * m_frameData.mbHeight];
        m_frameData.vbo_mb = new QOpenGLBuffer();
        if ( !m_frameData.vbo_mb->create() )
        {
            uninitYubFbo();
            return false;
        }
        dw = float(m_frameData.mbWidth * 16) / float(m_vidParams.width);
        dh = float(m_frameData.mbHeight * 16) / float(m_vidParams.height);

        m_vertex[0].text = QVector2D(dw, 1.0f);
        m_vertex[1].text = QVector2D(0.0f, 1.0f);
        m_vertex[2].text = QVector2D(0.0f, 1.0f - dh);
        m_vertex[3].text = QVector2D(dw, 1.0f - dh);

        m_frameData.vbo_mb->bind();
        m_frameData.vbo_mb->allocate(&m_vertex, 5 * 4 * sizeof(GLfloat));
        m_frameData.vbo_mb->release();

        m_frameData.prog_mb = m_progPool.createProgram("x264-mb");
        m_frameData.prog_mb->bind();
        m_frameData.prog_mb->setUniformValue("qt_ModelViewProjectionMatrix",m);
        m_frameData.prog_mb->setUniformValue("qt_Texture0", 0);
        m_frameData.prog_mb->setUniformValue("qt_Texture1", 1);
        m_frameData.prog_mb->setUniformValue("imageSize", m_vidParams.width, m_vidParams.height);
        m_frameData.prog_mb->setUniformValue("mbSize", m_frameData.mbWidth, m_frameData.mbHeight);
        m_frameData.prog_mb->release();
    }
    return true;
}

void VideoSynthesizer::uninitYubFbo()
{
    if ( m_frameData.vbo_mb )
    {
        delete m_frameData.vbo_mb;
        m_frameData.vbo_mb = nullptr;
    }
    if ( m_frameData.fbo_mb )
    {
        delete m_frameData.fbo_mb;
        m_frameData.fbo_mb = nullptr;
    }
    if ( m_frameData.buf_mb )
    {
        delete []m_frameData.buf_mb;
        m_frameData.buf_mb = nullptr;
    }
    if (m_frameData.prog_mb)
    {
        delete m_frameData.prog_mb;
        m_frameData.prog_mb = nullptr;
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

void VideoSynthesizer::putFrameToEncoder()
{

    if (m_frameData.fbo == nullptr) return;
    FrameTimestamp ttt;

    static int64_t t = 0;
    static int64_t c = 0;
    x264_picture_t* picin = m_vidEncoder.beginAddFrame(m_frameData.timestamp);
    if (picin)
    {
        ++c;
        ttt.start();

        if (m_vidParams.useMbInfo)
        {

            m_frameData.fbo_mb->bind();
            glReadPixels(0, 0, m_frameData.mbWidth, m_frameData.mbHeight,
                         GL_ALPHA, GL_UNSIGNED_BYTE, m_frameData.buf_mb);
            m_frameData.fbo_mb->release();

            if ( m_frameData.mbPitch == m_frameData.mbWidth)
            {
                memcpy(picin->prop.mb_info, m_frameData.buf_mb, m_frameData.mbPitch * m_frameData.mbHeight);
            }
            else
            {
                for (int y = 0; y < m_frameData.mbHeight; ++y)
                {
                    memcpy(picin->prop.mb_info + y * m_frameData.mbWidth, m_frameData.buf_mb + m_frameData.mbPitch * y, m_frameData.mbWidth);
                }
            }
     //       qDebug() << "Cale MB:" << ttt.elapsed();
    //        QImage img((const uchar*)m_frameData.buf_mb, m_frameData.mbWidth, m_frameData.mbHeight, m_frameData.mbPitch, QImage::Format_Grayscale8);
    //        QString fn = QString("/home/guee/Pictures/YUV/%1.png").arg(m_frameData.timestamp);
    //        img.save(fn, "png");
        }
        m_frameData.fbo->bind();
        for (int i  = 0; i < m_frameData.planeCount; ++i)
        {
    //        glReadBuffer(GL_FRONT);
            QRect& rt = m_frameData.rect[i];

            glReadPixels(rt.x(), rt.y(), rt.width(), rt.height(),
                         GL_ALPHA, GL_UNSIGNED_BYTE, picin->img.plane[i]);

        }
        m_frameData.fbo->release();

        m_vidEncoder.doneAddFrame(picin);
        t += ttt.elapsed();
     //   qDebug() << "putFrameToEncoder ms:" << t / c;
    }


}

void VideoSynthesizer::onLayerOpened(BaseLayer *layer)
{
    m_immediateUpdate = true;
    emit layerAdded(layer);
}

void VideoSynthesizer::onLayerRemoved(BaseLayer *layer)
{
    BaseLayer::onLayerRemoved(layer);
    m_immediateUpdate = true;
    emit layerRemoved(layer);
}

void VideoSynthesizer::onSizeChanged(BaseLayer *layer)
{
    BaseLayer::onSizeChanged(layer);
    m_immediateUpdate = true;
    emit layerMoved(layer);
}


