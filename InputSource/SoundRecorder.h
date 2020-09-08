#ifndef SOUNDRECORD_H
#define SOUNDRECORD_H
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QList>

class SoundRecorder;
class SoundDevInfo : private QIODevice
{
public:
    ~SoundDevInfo();
    bool isCallbackType() const;
    QStringList availableDev(bool refresh);
    QString defaultDev();
    QString currentDev() const;

    bool selectDev(const QString& dev);
    bool isEnabled() const;
    void setEnable(bool enable);

    qreal amplitude() const { return m_curAmplitude; }
    qreal volume() const { return m_volume; }
    bool setVolume(qreal val);
    const QAudioFormat& format() const { return m_format; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
private:
    SoundDevInfo(SoundRecorder& recorder);
    bool start(const QAudioFormat& format);
    void stop();

    friend SoundRecorder;
    bool m_isCallbackType = false;
    bool m_isEnabled = false;
    SoundRecorder& m_rec;
    QAudioDeviceInfo m_curDev;
    QList<QAudioDeviceInfo> m_devLst;

    QAudioFormat m_format;
    QAudioInput* m_audioInput = nullptr;
    bool m_isOpened = false;
    qreal   m_volume = -1.0;

    quint32 m_maxAmplitude = 0;
    qreal m_curAmplitude = 0.0; // 0.0 <= m_level <= 1.0

    void getMaxAmplitude();
};

class SoundRecorder
{
public:
    SoundRecorder();
    ~SoundRecorder();

    SoundDevInfo& callbackDev() { return m_sndCallback; }
    SoundDevInfo& micInputDev() { return m_sndMicInput; }

    enum SampleRate
    {
        SR_11025 = 11025,
        SR_22050 = 22050,
        SR_44100 = 44100
    };
    enum SampleBits
    {
        SB_16i = 16,
        SB_32f = 32
    };
    enum SampleChannel
    {
        SC_Mono = 1,
        SC_Stereo = 2
    };

    bool startRec(SampleRate rate, SampleBits bits, SampleChannel channel);
    bool stopRec();
private:
    friend SoundDevInfo;
    bool m_isOpened = false;
    QAudioFormat m_audioFormat;
    SoundDevInfo m_sndCallback;
    SoundDevInfo m_sndMicInput;
};

#endif // SOUNDRECORD_H
