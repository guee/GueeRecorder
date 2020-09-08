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
    stopRec();
}

bool SoundRecorder::startRec(int32_t rate, ESampleBits bits, int32_t channel)
{
    QAudioFormat fmt;
    fmt.setSampleRate(rate);
    fmt.setChannelCount(channel);
    switch (bits)
    {
    case eSampleBit8i:
        fmt.setSampleSize(8);
        fmt.setSampleType(QAudioFormat::UnSignedInt);
        break;
    case eSampleBit16i:
        fmt.setSampleSize(16);
        fmt.setSampleType(QAudioFormat::SignedInt);
        break;
    case eSampleBit24i:
        fmt.setSampleSize(24);
        fmt.setSampleType(QAudioFormat::SignedInt);
        break;
    case eSampleBit32i:
        fmt.setSampleSize(32);
        fmt.setSampleType(QAudioFormat::SignedInt);
        break;
    case eSampleBit24In32i:
        fmt.setSampleSize(32);
        fmt.setSampleType(QAudioFormat::SignedInt);
        break;
    case eSampleBit32f:
        fmt.setSampleSize(32);
        fmt.setSampleType(QAudioFormat::Float);
        break;
    }
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setCodec("audio/pcm");

    if (m_isRecOpened && fmt == m_audioFormat)
        return  true;
    stopRec();
    m_isRecOpened = m_sndCallback.start(fmt) || m_isRecOpened;
    m_isRecOpened = m_sndMicInput.start(fmt) || m_isRecOpened;
    if (m_isRecOpened)
    {
        m_audioFormat = fmt;
    }
    else
    {
        m_sndCallback.stop();
        m_sndMicInput.stop();
    }

    return m_isRecOpened;
}

bool SoundRecorder::stopRec()
{
    if (m_isRecOpened)
    {
        m_isRecOpened = false;
        m_sndCallback.stop();
        m_sndMicInput.stop();
        return true;
    }
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
            if (m_isDevOpened)
            {
                return start(m_format);
            }
            return true;
        }
    }
    return false;
}

bool SoundDevInfo::isEnabled() const
{
    return m_isEnabled;
}

void SoundDevInfo::setEnable(bool enable)
{
    if (m_isEnabled != enable)
    {
        m_isEnabled = enable;
        if (m_isDevOpened)
        {
            if (m_audioInput)
            {
                if (enable)
                {
                    m_audioInput->resume();
                }
                else
                {
                    m_audioInput->suspend();
                    m_curAmplitude = 0.0;
                }
            }
            else if (enable)
            {
                start(m_format);
            }
        }
    }
}

bool SoundDevInfo::setVolume(qreal val)
{
    m_volume = val;
    if (m_audioInput)
    {
        qreal linearVolume = QAudio::convertVolume(val,
                               QAudio::LogarithmicVolumeScale,
                               QAudio::LinearVolumeScale);
        m_audioInput->setVolume(linearVolume);

    }
}

bool SoundDevInfo::start(const QAudioFormat &format)
{
    availableDev(false);
    if (m_curDev.isNull())
        return false;
    if (m_audioInput)
    {
        stop();
    }
    QAudioFormat fmt;
    if (m_curDev.isFormatSupported(format))
    {
        fmt = m_curDev.nearestFormat(format);
    }
    else
    {
        fmt = format;
    }
    qDebug() << "fmt.sampleRate:" << fmt.sampleRate();
    qDebug() << "fmt.channelCount:" << fmt.channelCount();
    qDebug() << "fmt.sampleSize:" << fmt.sampleSize();
    qDebug() << "fmt.sampleType:" << fmt.sampleType();
    qDebug() << "fmt.byteOrder:" << fmt.byteOrder();
    qDebug() << "fmt.codec:" << fmt.codec();
    m_maxAmplitude = getMaxAmplitude(fmt);
    m_audioInput = new QAudioInput(m_curDev, fmt);
    if (m_audioInput->error() == QAudio::OpenError)
    {
        stop();
        return false;
    }
    m_format = format;
    if (m_volume < 0.0)
    {
        m_volume = QAudio::convertVolume(m_audioInput->volume(),
                                QAudio::LinearVolumeScale,
                                QAudio::LogarithmicVolumeScale);
    }
    else
    {
        setVolume(m_volume);
    }
    open(QIODevice::WriteOnly);
    m_audioInput->start(this);
    if ( !m_isEnabled )
    {
        m_audioInput->suspend();
    }
    m_isDevOpened = true;
    return true;
}

void SoundDevInfo::stop()
{
    if (m_audioInput)
    {
        m_audioInput->stop();
        close();
        delete m_audioInput;
        m_audioInput = nullptr;
    }
    m_isDevOpened = false;
}

qint64 SoundDevInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)

    return 0;
}

qint64 SoundDevInfo::writeData(const char *data, qint64 len)
{
    if (m_maxAmplitude)
    {
        Q_ASSERT(m_format.sampleSize() % 8 == 0);
        const int channelBytes = m_format.sampleSize() / 8;
        const int sampleBytes = m_format.channelCount() * channelBytes;
        Q_ASSERT(len % sampleBytes == 0);
        const int numSamples = len / sampleBytes;

        quint32 maxValue = 0;
        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

        for (int i = 0; i < numSamples; ++i) {
            for (int j = 0; j < m_format.channelCount(); ++j) {
                quint32 value = 0;

                if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                    value = *reinterpret_cast<const quint8*>(ptr);
                } else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt) {
                    value = qAbs(*reinterpret_cast<const qint8*>(ptr));
                } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint16>(ptr);
                    else
                        value = qFromBigEndian<quint16>(ptr);
                } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt) {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qAbs(qFromLittleEndian<qint16>(ptr));
                    else
                        value = qAbs(qFromBigEndian<qint16>(ptr));
                } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint32>(ptr);
                    else
                        value = qFromBigEndian<quint32>(ptr);
                } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::SignedInt) {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qAbs(qFromLittleEndian<qint32>(ptr));
                    else
                        value = qAbs(qFromBigEndian<qint32>(ptr));
                } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::Float) {
                    value = qAbs(*reinterpret_cast<const float*>(ptr) * 0x7fffffff); // assumes 0-1.0
                }

                maxValue = qMax(value, maxValue);
                ptr += channelBytes;
            }
        }

        maxValue = qMin(maxValue, m_maxAmplitude);
        m_curAmplitude = qreal(maxValue) / m_maxAmplitude;
    }
    return len;
}

quint32 SoundDevInfo::getMaxAmplitude(const QAudioFormat& format)
{
    quint32 maxAmplitude = 0;
    switch (format.sampleSize()) {
    case 8:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            maxAmplitude = 0x7fffffff; // Kind of
        default:
            break;
        }
        break;

    default:
        break;
    }
    return maxAmplitude;
}
