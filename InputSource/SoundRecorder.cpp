#include "SoundRecorder.h"
#include <QDebug>

SoundRecorder::SoundRecorder()
    :m_sndCallback(*this)
    ,m_sndMicInput(*this)
{
    m_sndCallback.m_isCallbackType = true;
    m_sndMicInput.m_isCallbackType = false;
}

SoundRecorder::~SoundRecorder()
{

}

bool SoundRecorder::startRec(const QAudioFormat &format)
{
    return false;
}

bool SoundRecorder::stopRec()
{
    return false;
}

SoundDevInfo::SoundDevInfo(SoundRecorder &recorder)
    :m_rec(recorder)
{

}
SoundDevInfo::~SoundDevInfo()
{

}

bool SoundDevInfo::isCallbackType() const
{
    return m_isCallbackType;
}

QStringList SoundDevInfo::availableDev(bool refresh)
{
    if (refresh || m_devLst.empty())
    {
        m_devLst.clear();
        if (m_isCallbackType)
        {
            const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
            QString defalut;
            if (!defaultDeviceInfo.isNull())
            {
                defalut = defaultDeviceInfo.deviceName();
            }
            for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
            {
                QString name = deviceInfo.deviceName();
                if (name.endsWith(".monitor"))
                {
                    name = name.left(name.length() - 8);
                }
                if (defalut == name)
                {
                    m_devLst.prepend(deviceInfo);
                    if (m_curDev.isNull()) m_curDev = deviceInfo;
                }
                else if (deviceInfo.deviceName().startsWith("alsa_output."))
                {
                    m_devLst.append(deviceInfo);
                }
                //qDebug() <<deviceInfo.deviceName();
            }
        }
        else
        {
            const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
            if (!defaultDeviceInfo.isNull())
            {
                m_devLst.append(defaultDeviceInfo);
                if (m_curDev.isNull()) m_curDev = defaultDeviceInfo;
            }
            for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
            {
                if (defaultDeviceInfo != deviceInfo && deviceInfo.deviceName().startsWith("alsa_input."))
                {
                    m_devLst.append(deviceInfo);
                }
                //qDebug() <<deviceInfo.deviceName();
            }
        }
    }
    QStringList result;
    for (auto &d:m_devLst)
    {
        result.append(d.deviceName());
    }
    return result;
}

QString SoundDevInfo::defaultDev()
{
    QStringList lst = availableDev(false);
    if(lst.empty()) return "";
    return lst[0];
}

QString SoundDevInfo::currentDev() const
{
    return m_curDev.isNull() ? "" : m_curDev.deviceName();
}

bool SoundDevInfo::selectDev(const QString &dev)
{
    if (!m_curDev.isNull())
    {
        if (m_curDev.deviceName() == dev)
            return true;
    }
    for (auto& d: m_devLst)
    {
        if (d.deviceName() == dev)
        {
            m_curDev = d;
            return true;
        }
    }
    return false;
}

bool SoundDevInfo::isEnabled() const
{
    return m_isEnabled;
}

bool SoundDevInfo::setEnable(bool enable)
{
    m_isEnabled = true;
}

