#ifndef SOUNDRECORD_H
#define SOUNDRECORD_H
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QSemaphore>

#include "H264Codec.h"
#include "faac.h"
#include "MediaStream.h"

#include "WaveFile.h"
#include "../Common/FrameTimestamp.h"

class SoundRecorder;
class SoundDevInfo : private QIODevice
{
public:
    ~SoundDevInfo() override;
    bool isCallbackType() const;
    QStringList availableDev(bool refresh);
    QString defaultDev();
    QString currentDev() const;

    bool selectDev(const QString& dev);
    bool isEnabled() const { return m_isEnabled; }
    void setEnable(bool enable);

    qreal amplitude() const { return m_curAmplitude; }
    qreal volume() const { return m_volume; }
    bool setVolume(qreal val);
    const QAudioFormat& format() const { return m_format; }

    int32_t realSampleRate() const { return m_realSample ? int32_t(m_realSample * 1000000 / m_time.elapsed()) : 0; }
private:
    SoundDevInfo(SoundRecorder& recorder);
    bool start(const QAudioFormat& format, bool checkDev);
    void stop();
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    friend SoundRecorder;
    bool m_isCallbackType = false;
    bool m_isEnabled = false;
    SoundRecorder& m_rec;
    QAudioDeviceInfo m_curDev;
    QList<QAudioDeviceInfo> m_devLst;

    QAudioFormat m_format;
    QAudioInput* m_audioInput = nullptr;
    bool m_isDevOpened = false;
    qreal m_volume = -1.0;

    qreal m_curAmplitude = 0.0;

    uint8_t* m_pcmBuffer = nullptr;
    int32_t m_bufferSize = 0;
    int32_t m_dataOffset = 0;
    int32_t m_dataSize = 0;
    int64_t m_sampleBegin = 0;
    int64_t m_prevRealse = 0;

    QMutex m_mutexPcmBuf;
    bool m_firstGetPCM = false;
    bool m_needResample = false;
    FrameTimestamp m_time;
    int64_t m_realSample;
    uint8_t* m_resBuffer;
    uint8_t m_prevSample[16];

    void initResample();
    void uninitResample();
    static QList<QAudioDeviceInfo> &availableDevices();
};

class SoundRecorder : private QThread
{
public:
    SoundRecorder();
    ~SoundRecorder();

    bool bindStream( GueeMediaStream* stream );

    SoundDevInfo& callbackDev() { return m_sndCallback; }
    SoundDevInfo& micInputDev() { return m_sndMicInput; }

    bool startRec(int32_t rate, ESampleBits bits, int32_t channel);
    bool stopRec();
    bool isOpened() const {return m_status >= Opened;}

    bool startEncode( const SAudioParams* audioParams );
    void endEncode();
    bool isEncodeing() const {return m_status >= Encodeing;}

    bool pauseEncode(bool pause);
private:
    void run();
    friend SoundDevInfo;
    enum RecStatus
    {
        NoOpen,         //没有打开
        Opened,         //已经打开成功
        Encodeing,      //正在录制/编码
        Paused,         //暂停中
    };
    RecStatus m_status = NoOpen;
    QAudioFormat m_audioFormat;
    SoundDevInfo m_sndCallback;
    SoundDevInfo m_sndMicInput;
    GueeMediaStream* m_mediaStream = nullptr;
    faacEncHandle  m_faacHandle = nullptr;
    SAudioParams   m_audioParams;
    int32_t     m_bytesPerSample;   //每个采样的字节数（一个声道）
    int32_t     m_samplesPerFrame;  //每个AAC帧的采样数（声道数*采样数）
    int32_t     m_maxOutByteNum;    //AAC每帧编码输出的最大字节数
    QSemaphore m_waitPcmBuffer;
    int64_t     m_lastSampleNum = 0;
#define _USE_WAVE_TEST 0
#if _USE_WAVE_TEST
    CWaveFile m_waveFile;
#endif
    bool mixPcm( int64_t frameBegin, SoundDevInfo& dev, uint8_t* mixBuf );
private:
    static bool initFaac_Functions();

    typedef int (FAACAPI *p_faacEncGetVersion)(char **faac_id_string, char **faac_copyright_string);
    typedef faacEncConfigurationPtr (FAACAPI *p_faacEncGetCurrentConfiguration)(faacEncHandle hEncoder);
    typedef int (FAACAPI *p_faacEncSetConfiguration)(faacEncHandle hEncoder, faacEncConfigurationPtr config);
    typedef faacEncHandle (FAACAPI *p_faacEncOpen)(unsigned long sampleRate, unsigned int numChannels, unsigned long *inputSamples, unsigned long *maxOutputBytes);
    typedef int (FAACAPI *p_faacEncGetDecoderSpecificInfo)(faacEncHandle hEncoder, unsigned char **ppBuffer, unsigned long *pSizeOfDecoderSpecificInfo);
    typedef int (FAACAPI *p_faacEncEncode)(faacEncHandle hEncoder, int32_t * inputBuffer, unsigned int samplesInput, unsigned char *outputBuffer, unsigned int bufferSize);
    typedef int (FAACAPI *p_faacEncClose)(faacEncHandle hEncoder);

    static p_faacEncGetVersion faacEncGetVersion;
    static p_faacEncGetCurrentConfiguration faacEncGetCurrentConfiguration;
    static p_faacEncSetConfiguration faacEncSetConfiguration;
    static p_faacEncOpen faacEncOpen;
    static p_faacEncGetDecoderSpecificInfo faacEncGetDecoderSpecificInfo;
    static p_faacEncEncode faacEncEncode;
    static p_faacEncClose faacEncClose;
    static QLibrary       m_libFaac;
};

#endif // SOUNDRECORD_H
