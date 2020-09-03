#ifndef VIDEOSYNTHESIZER_H
#define VIDEOSYNTHESIZER_H
#include "InputSource/BaseLayer.h"
#include "MediaCodec/VideoEncoder.h"
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
    int32_t width() const {return m_params.width;}
    int32_t height() const {return m_params.height;}
    bool setFrameRate(float fps);
    float frameRate() const override {return m_params.frameRate;}
    float renderFps() const {return m_frameRate.fps();}
    bool setProfile(EVideoProfile profile);
    EVideoProfile profile() const {return m_params.profile;}
    bool setPreset(EVideoPreset_x264 preset);
    EVideoPreset_x264 perset() const {return m_params.presetX264;}
    bool setCSP(EVideoCSP csp);
    EVideoCSP csp() const {return m_params.outputCSP;}
    bool setPsyTune(EPsyTuneType psy);
    EPsyTuneType psyTune() const {return m_params.psyTune;}
    bool setBitrateMode(EVideoRateMode rateMode);
    EVideoRateMode bitrateMode() const {return m_params.rateMode;}
    bool setBitrate(int32_t bitrate);
    int32_t bitrate() const {return m_params.bitrate;}
    bool setBitrateMin(int32_t bitrateMin);
    int32_t bitrateMin() const {return m_params.bitrateMin;}
    bool setBitrateMax(int32_t bitrateMax);
    int32_t bitrateMax() const {return m_params.bitrateMax;}
    bool setVbvBuffer(int32_t vbvBufferSize);
    int32_t vbvBuffer() const {return m_params.vbvBuffer;}
    bool setGopMin(int32_t gopMin);
    int32_t gopMin() const {return m_params.gopMin;}
    bool setGopMax(int32_t gopMax);
    int32_t gopMax() const {return m_params.gopMax;}
    bool setRefFrames(int32_t refFrames);
    int32_t refFrames() const {return m_params.refFrames;}
    bool setBFrames(int32_t bFrames);
    int32_t bFrames() const {return m_params.BFrames;}
private:
    VideoSynthesizer();
    SVideoParams m_params;
    FrameSynchronization    m_frameSync;
    FrameRateCalc           m_frameRate;
    struct FrameInfo
    {
        EVideoCSP csp;
        int64_t timestamp;
        int alignWidth;
        int alignHeight;
        int textureWidth;
        int textureHeight;
        GLenum internalFormat;
        GLenum dateType;
        int dataSize;
        int planeCount;
        int stride[3];
        QOpenGLBuffer*  buffer = nullptr;
        QOpenGLFramebufferObject* fbo = nullptr;
    };

    FrameInfo    m_frameData;

    CVideoEncoder*  m_encoder = nullptr;
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
    void putFrameToEncoder(GLuint textureId);
signals:
    void frameReady(uint textureId);
    void initDone( bool success);
};

#endif // VIDEOSYNTHESIZER_H
