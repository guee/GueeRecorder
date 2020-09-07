#ifndef SOUNDRECORD_H
#define SOUNDRECORD_H

#include "AudioCaptureSource.h"
#include <QList>

class SoundRecorder;
class SoundDevInfo : private AudioCaptureSource
{
public:
    ~SoundDevInfo();
    bool isCallbackType() const;
    QStringList availableDev(bool refresh);
    QString defaultDev();
    QString currentDev() const;

    bool selectDev(const QString& dev);
    bool isEnabled() const;
    bool setEnable(bool enable);

    using AudioCaptureSource::amplitude;
    using AudioCaptureSource::volume;
    using AudioCaptureSource::setVolume;
    using AudioCaptureSource::format;
private:
    SoundDevInfo(SoundRecorder& recorder);
    friend SoundRecorder;
    bool m_isCallbackType = false;
    bool m_isEnabled = false;
    SoundRecorder& m_rec;
    QAudioDeviceInfo m_curDev;
    QList<QAudioDeviceInfo> m_devLst;
};

class SoundRecorder
{
public:
    SoundRecorder();
    ~SoundRecorder();

    SoundDevInfo& callbackDev() { return m_sndCallback; }
    SoundDevInfo& micInputDev() { return m_sndMicInput; }

    bool startRec(const QAudioFormat& format);
    bool stopRec();
private:
    friend SoundDevInfo;
    SoundDevInfo m_sndCallback;
    SoundDevInfo m_sndMicInput;
};

#endif // SOUNDRECORD_H
