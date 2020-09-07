#ifndef AUDIOCAPTURESOURCE_H
#define AUDIOCAPTURESOURCE_H

#include <QAudioInput>
#include <QAudioDeviceInfo>

class AudioCaptureSource : public QIODevice
{
public:
    AudioCaptureSource();
    virtual ~AudioCaptureSource() override;
    void start(const QAudioDeviceInfo& deviceInfo, const QAudioFormat& format);
    void stop();

    qreal volume() const { return m_volume; }
    bool setVolume(qreal val);

    qreal amplitude() const { return m_curAmplitude; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    const QAudioFormat& format();
private:
    QAudioFormat m_format;
    QAudioInput* m_audioInput = nullptr;
    qreal   m_volume = -1.0;

    quint32 m_maxAmplitude = 0;
    qreal m_curAmplitude = 0.0; // 0.0 <= m_level <= 1.0

    void getMaxAmplitude();
};

#endif // AUDIOCAPTURESOURCE_H
