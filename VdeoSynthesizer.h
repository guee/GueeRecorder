#ifndef VIDEOSYNTHESIZER_H
#define VIDEOSYNTHESIZER_H
#include "InputSource/BaseLayer.h"
#include "MediaCodec/EncoderParams.h"
#include "ShaderProgramPool.h"

class VideoSynthesizer : public QThread, public BaseLayer
{
    Q_OBJECT
public:
    ~VideoSynthesizer() override;
    static VideoSynthesizer& instance() { static VideoSynthesizer v; return v; }
    void init(QOpenGLContext* shardContext);
    void uninit();
    ShaderProgramPool& programPool() {return m_progPool;}
    const QString& layerType() const override { static QString tn = "synthesizer"; return tn; }
    BaseLayer *createLayer(const QString &type);
    void immediateUpdate();

    bool open(const QString &sourceName = QString());
    void close();

    bool play();
    bool pause();

    bool resetDefaultOption();
    bool setSize(int32_t width, int32_t height);
    int32_t width() const {return m_params.iWidth;}
    int32_t height() const {return m_params.iHeight;}
    bool setFrameRate(float fps);
    float frameRate() const override {return m_params.fFrameRate;}
    float renderFps() const {return m_frameRate.fps();}
    bool setProfile(EVideoProfile profile);
    EVideoProfile profile() const {return m_params.eProfile;}
    bool setPreset(EVideoPreset_x264 preset);
    EVideoPreset_x264 perset() const {return m_params.ePresetX264;}
    bool setCSP(EVideoCSP csp);
    EVideoCSP csp() const {return m_params.eOutputCSP;}
    bool setPsyTune(EPsyTuneType psy);
    EPsyTuneType psyTune() const {return m_params.ePsyTune;}
    bool setBitrateMode(EBitrateMode rateMode);
    EBitrateMode bitrateMode() const {return m_params.eBitrateMode;}
    bool setBitrate(int32_t bitrate);
    int32_t bitrate() const {return m_params.iBitrate;}
    bool setBitrateMin(int32_t bitrateMin);
    int32_t bitrateMin() const {return m_params.iBitrateMin;}
    bool setBitrateMax(int32_t bitrateMax);
    int32_t bitrateMax() const {return m_params.iBitrateMax;}
    bool setVbvBuffer(int32_t vbvBufferSize);
    int32_t vbvBuffer() const {return m_params.iVbvBuffer;}
    bool setGopMin(int32_t gopMin);
    int32_t gopMin() const {return m_params.iGopMin;}
    bool setGopMax(int32_t gopMax);
    int32_t gopMax() const {return m_params.iGopMax;}
    bool setRefFrames(int32_t refFrames);
    int32_t refFrames() const {return m_params.iRefFrames;}
    bool setBFrames(int32_t bFrames);
    int32_t bFrames() const {return m_params.iBFrames;}
private:
    VideoSynthesizer();
    SVideoParams m_params;
    FrameSynchronization    m_frameSync;
    FrameRateCalc           m_frameRate;
    SInputImage             m_frameData;
    enum PlaneType
    {
        Plane_Y,
        Plane_U,
        Plane_V,
        Plane_UV,
        Plane_VU,
        Plane_YUYV,
        Plane_UYVY,
        Plane_AYUV,
        Plane_BGR,
        Plane_RGB,
        Plane_BGRA,
    };
    struct PlaneData
    {
        PlaneType planeType;
        int32_t dataSize;
        GLenum format;
        GLenum dataType;
        QOpenGLFramebufferObject* fbo;
    };

    PlaneData  m_planes[3];
    QOpenGLBuffer*  m_yuvBuffer = nullptr;

    bool initYuvFbo();
    void uninitYubFbo();

    bool m_immediateUpdate = false;
    bool m_threadWorking = false;
    QOpenGLContext* m_context = nullptr;
    QOffscreenSurface* m_surface = nullptr;
    QVector4D m_backgroundColor;
    void renderThread();
    void run() override;
    BaseSource* onCreateSource(const QString &sourceName) override { Q_UNUSED(sourceName) return nullptr; }
    void onReleaseSource(BaseSource* source) override { Q_UNUSED(source) }
    ShaderProgramPool m_progPool;
    void loadShaderPrograms();
    void putFrameToEncoder();
signals:
    void frameReady(uint textureId);
    void initDone( bool success);
};

#endif // VIDEOSYNTHESIZER_H
