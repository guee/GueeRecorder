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

private:
    SoundDevInfo(SoundRecorder& recorder);
    bool start(const QAudioFormat& format);
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

    QMutex m_mutexPcmBuf;
//    QMutex m_mutexPending;
//    QMutex m_mutexIdling;
//    QList<uint8_t*> m_pending;
//    QList<uint8_t*> m_idling;
    bool m_needResample = false;

    void initResample();
    void uninitResample();
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

    bool mixPcm( int64_t frameBegin, SoundDevInfo& dev, uint8_t* mixBuf );

};

#endif // SOUNDRECORD_H
