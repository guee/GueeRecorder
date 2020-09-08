#ifndef SOUNDRECORD_H
#define SOUNDRECORD_H
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QList>
#include "H264Codec.h"

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
    bool m_isDevOpened = false;
    qreal m_volume = -1.0;

    quint32 m_maxAmplitude = 0;
    qreal m_curAmplitude = 0.0; // 0.0 <= m_level <= 1.0

    quint32 getMaxAmplitude(const QAudioFormat& format);
};

class SoundRecorder
{
public:
    SoundRecorder();
    ~SoundRecorder();

    SoundDevInfo& callbackDev() { return m_sndCallback; }
    SoundDevInfo& micInputDev() { return m_sndMicInput; }

    bool startRec(int32_t rate, ESampleBits bits, int32_t channel);
    bool stopRec();
    bool isOpened() const {return m_isRecOpened;}
private:
    friend SoundDevInfo;
    bool m_isRecOpened = false;
    QAudioFormat m_audioFormat;
    SoundDevInfo m_sndCallback;
    SoundDevInfo m_sndMicInput;
};

#endif // SOUNDRECORD_H
