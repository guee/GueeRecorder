#include "SoundRecorder.h"
#include <QDebug>

SoundRecorder::SoundRecorder()
    :m_sndCallback(*this)
    ,m_sndMicInput(*this)
{
    m_sndCallback.m_isCallbackType = true;
    m_sndMicInput.m_isCallbackType = false;
    memset(&m_audioParams, 0, sizeof(m_audioParams));
}

SoundRecorder::~SoundRecorder()
{
    stopRec();
}

bool SoundRecorder::startRec(int32_t rate, ESampleBits bits, int32_t channel)
{
    if (m_status > Opened)
        return false;

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

    if (m_status == Opened && fmt == m_audioFormat)
        return  true;
    stopRec();
    if (m_sndCallback.start(fmt)) m_status = Opened;
    if (m_sndMicInput.start(fmt)) m_status = Opened;
    if (m_status == Opened)
    {
        m_audioFormat = fmt;
    }
    else
    {
        m_sndCallback.stop();
        m_sndMicInput.stop();
    }
    return m_status == Opened;
}

bool SoundRecorder::stopRec()
{
    if (m_status >= Opened)
    {
        endEncode();
        m_status = NoOpen;
        m_sndCallback.stop();
        m_sndMicInput.stop();
        return true;
    }
    return false;
}

bool SoundRecorder::startEncode(const SAudioParams *audioParams)
{
    if (audioParams == nullptr)
    {
        return false;
    }
    m_audioParams = *audioParams;
    m_audioParams.encLevel	= ( m_audioParams.encLevel < MAIN || m_audioParams.encLevel > LTP ) ? LOW : m_audioParams.encLevel;
    uint32_t					faacBits	= 0;
    switch( m_audioParams.sampleBits )
    {
    case eSampleBit8i:
    case eSampleBit16i:
        m_audioParams.sampleBits		= eSampleBit16i;
        m_bytesPerSample = 2;
        faacBits	= FAAC_INPUT_16BIT;
        break;
    case eSampleBit24i:
    case eSampleBit32i:
    case eSampleBit24In32i:
        m_audioParams.sampleBits		= eSampleBit32i;
        m_bytesPerSample = 4;
        faacBits	= FAAC_INPUT_32BIT;
        break;
    case eSampleBit32f:
        m_audioParams.sampleBits		= eSampleBit32f;
        m_bytesPerSample = 4;
        faacBits	= FAAC_INPUT_FLOAT;
        break;
    }
    ulong		bitRate	= ulong(m_audioParams.bitrate * 1024 / m_audioParams.channels);

    m_faacHandle	= faacEncOpen( ulong(m_audioParams.sampleRate),
        uint(m_audioParams.channels), &m_samplesPerFrame, &m_maxOutByteNum );
    m_bytesPerFrame = m_samplesPerFrame * m_bytesPerSample * m_audioParams.channels;
    faacEncConfigurationPtr	pFaacCfg	= faacEncGetCurrentConfiguration( m_faacHandle );
    pFaacCfg->mpegVersion	= MPEG4;
    pFaacCfg->aacObjectType	= m_audioParams.encLevel;
    pFaacCfg->bitRate		= bitRate;
    pFaacCfg->allowMidside	= 0;
    //pFaacCfg->quantqual		= 50;
    pFaacCfg->outputFormat	= m_audioParams.useADTS ? 1 : 0;
    pFaacCfg->inputFormat	= faacBits;
    if ( !faacEncSetConfiguration( m_faacHandle, pFaacCfg ) )
    {
        faacEncClose( m_faacHandle );
        m_faacHandle	= nullptr;
        return false;
    }
    m_status = recording;
    return true;
}

void SoundRecorder::endEncode()
{

}

void SoundRecorder::run()
{
    uint8_t* pcb = nullptr;
    uint8_t* mic = nullptr;
    int64_t pcbSampleNum = 0;
    int64_t micSampleNum = 0;
    int64_t delaySample = m_audioParams.sampleRate * 200 / 1000;
    uint8_t* mixBuf = new uint8_t[m_bytesPerFrame];
    int64_t frameSampleBegin = 0;

    memset(mixBuf, 0, m_bytesPerFrame);

    while(m_status >= recording)
    {
        if (m_waitPcmBuffer.tryAcquire(1, 200))
        {
            m_sndCallback.m_mutexPcmBuf.lock();
            int64_t sampleEnd = m_sndCallback.m_sampleBegin + m_sndCallback.m_dataSize / (m_bytesPerSample * m_audioParams.channels);
            if (m_sndCallback.m_dataSize)
            {
                int64_t begin = qMax(frameSampleBegin, m_sndCallback.m_sampleBegin);
                int64_t end = qMin(frameSampleBegin + m_samplesPerFrame,
                                   );
                if (m_audioParams.sampleBits == eSampleBit16i)
                {
                    int16_t* pcmMix = reinterpret_cast<int16_t*>(mixBuf) + (begin - frameSampleBegin) * m_audioParams.channels;
                    int16_t* pcmIn = reinterpret_cast<int16_t*>(m_sndCallback.m_pcmBuffer + m_sndCallback.m_dataOffset);
                    begin *= m_audioParams.channels;
                    end *= m_audioParams.channels;
                    while(begin < end)
                    {
                        *pcm = m_sndCallback.m_pcmBuffer[m_sndCallback.ea]
                    }
                }
                else if (m_audioParams.sampleBits == eSampleBit32i)
                {
                }
                else if (m_audioParams.sampleBits == eSampleBit32f)
                {
                }
            }

            m_sndCallback.m_mutexPcmBuf.unlock();

            if (m_sndCallback.m_isEnabled && m_sndMicInput.m_isEnabled)
            {
                if (nullptr == pcb) pcb = m_sndCallback.popPendBuffer();
                if (nullptr == mic) mic = m_sndMicInput.popPendBuffer();
                if (pcb && mic)
                {
                    //mix and put to aac encoder
                    pcbSampleNum += m_samplesPerFrame;
                    micSampleNum += m_samplesPerFrame;
                    pcb = nullptr;
                    mic = nullptr;
                }
                else if (qAbs(pcbSampleNum - micSampleNum) >= delaySample)
                {
                    //put to aac encoder
                    pcbSampleNum += m_samplesPerFrame;
                    micSampleNum += m_samplesPerFrame;
                    pcb = nullptr;
                    mic = nullptr;
                }
            }
            else if (m_sndCallback.m_isEnabled)
            {
                if (nullptr == pcb) pcb = m_sndCallback.popPendBuffer();
                if (pcb)
                {

                }
            }
        }

    }
}

bool SoundRecorder::mixPcm(int64_t frameBegin, SoundDevInfo &dev, uint8_t *mixBuf)
{
    bool ret = false;
    dev.m_mutexPcmBuf.lock();
    int64_t frameEnd = frameBegin + m_samplesPerFrame;
    int64_t sampleEnd = dev.m_sampleBegin + dev.m_dataSize / (m_bytesPerSample * m_audioParams.channels);
    if ( sampleEnd + m_maxDelaySample < frameBegin)
    {
        dev.m_sampleBegin = frameBegin;
        dev.m_dataSize = 0;
        ret = true;
    }
    else if (dev.m_sampleBegin >= frameEnd)
    {
        ret = true;
    }
    else
    {
        int64_t begin = qMax(frameBegin, m_sndCallback.m_sampleBegin);
        int64_t end = qMin(frameEnd, sampleEnd);
                                  );
        if (m_audioParams.sampleBits == eSampleBit16i)
        {
            int16_t* pcmMix = reinterpret_cast<int16_t*>(mixBuf) + (begin - frameSampleBegin) * m_audioParams.channels;
            int16_t* pcmIn = reinterpret_cast<int16_t*>(m_sndCallback.m_pcmBuffer + m_sndCallback.m_dataOffset);
            begin *= m_audioParams.channels;
            end *= m_audioParams.channels;
            while(begin < end)
            {
                *pcm = m_sndCallback.m_pcmBuffer[m_sndCallback.ea]
            }
        }
        else if (m_audioParams.sampleBits == eSampleBit32i)
        {
        }
        else if (m_audioParams.sampleBits == eSampleBit32f)
        {
        }
    }

    dev.m_mutexPcmBuf.unlock();
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
                    qDebug() << "resume err:" << m_audioInput->error() << ", state:" << m_audioInput->state();
                }
                else
                {
                    m_audioInput->suspend();
                    qDebug() << "resume err:" << m_audioInput->error() << ", state:" << m_audioInput->state();
                    m_curAmplitude = 0;

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
    m_volume = qMax(0.0, qMin(1.0, val));
    if (m_audioInput)
    {
        qreal linearVolume = QAudio::convertVolume(val,
                               QAudio::LogarithmicVolumeScale,
                               QAudio::LinearVolumeScale);
        m_audioInput->setVolume(linearVolume);
    }
    return true;
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
    m_curAmplitude = 0;
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
    m_curAmplitude = 0;
}

qint64 SoundDevInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)

    return 0;
}

qint64 SoundDevInfo::writeData(const char *data, qint64 len)
{
    if (m_rec.m_status == SoundRecorder::recording)
    {
        ulong num = ulong(len);
        const char* dat = data;

        if (m_needResample)
        {

        }
        else
        {
            m_mutexPcmBuf.lock();
            if (num > m_bufferSize - m_dataSize)
            {
                ulong skip = num - (m_bufferSize - m_dataSize);
                if ( skip > m_dataSize)
                {
                    num -= (skip - m_dataSize);
                    dat += (skip - m_dataSize);
                    m_dataSize = 0;
                    m_dataOffset = 0;
                }
                else
                {
                    m_dataSize -= skip;
                    m_dataOffset = (m_dataOffset + skip) % m_bufferSize;
                }
                m_sampleBegin += skip / (m_rec.m_bytesPerSample * m_rec.m_audioParams.channels);
            }
            while(num)
            {
                ulong endOffset = (m_dataOffset + m_dataSize) % m_bufferSize;
                ulong cpySize = qMin(m_bufferSize - endOffset, num);
                memcpy(m_pcmBuffer + endOffset, dat, cpySize);
                m_dataSize += cpySize;
                num -= cpySize;
                dat += cpySize;
            }
            m_mutexPcmBuf.unlock();
            m_rec.m_waitPcmBuffer.release();
        }
    }

    const int numSamples = int(len / (m_format.sampleSize() / 8));
    int skip = m_format.channelCount() * 2 + 1;
    qreal amplitude = 0.0;
    switch (m_format.sampleSize())
    {
    case 8:
        if (m_format.sampleType() == QAudioFormat::SignedInt)
        {
            qint8 maxValue = 0;
            const qint8* ptr = reinterpret_cast<const qint8*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(ptr[i], maxValue);
            amplitude = qreal(maxValue) / 127.0;
        }
        else
        {
            quint8 maxValue = 0;
            const quint8* ptr = reinterpret_cast<const quint8*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(ptr[i], maxValue);
            amplitude = qreal(maxValue) / 255.0;
        }
        break;
    case 16:
        if (m_format.sampleType() == QAudioFormat::SignedInt)
        {
            qint16 maxValue = 0;
            const qint16* ptr = reinterpret_cast<const qint16*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(qAbs(ptr[i]), maxValue);
            amplitude = qreal(maxValue) / 32767.0;
        }
        else
        {
            quint16 maxValue = 0;
            const quint16* ptr = reinterpret_cast<const quint16*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(ptr[i], maxValue);
            amplitude = qreal(maxValue) / 65536.0;
        }
        break;
    case 32:
        if (m_format.sampleType() == QAudioFormat::SignedInt)
        {
            qint32 maxValue = 0;
            const qint32* ptr = reinterpret_cast<const qint32*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(qAbs(ptr[i]), maxValue);
            amplitude = qreal(maxValue) / 0x7fffffff;
        }
        else if (m_format.sampleType() == QAudioFormat::UnSignedInt)
        {
            quint32 maxValue = 0;
            const quint32* ptr = reinterpret_cast<const quint32*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(ptr[i], maxValue);
            amplitude = qreal(maxValue) / quint32(0xffffffff);
        }
        else
        {
            float maxValue = 0;
            const float* ptr = reinterpret_cast<const float*>(data);
            for (int i = 0; i < numSamples; i += skip)
                maxValue = qMax(qAbs(ptr[i]), maxValue);
            amplitude = qreal(maxValue);
        }
    }
    m_curAmplitude = qMin(1.0, amplitude);
    return len;
}

void SoundDevInfo::initResample()
{
    if (m_format.sampleRate() != m_rec.m_audioParams.sampleRate)
    {
        m_needResample = true;
    }
    else
    {
        switch (m_rec.m_audioParams.sampleBits)
        {
        case eSampleBit8i:
            m_needResample = (m_format.sampleSize() != 8 || m_format.sampleType() != QAudioFormat::UnSignedInt);
            break;
        case eSampleBit16i:
            m_needResample = (m_format.sampleSize() != 16 || m_format.sampleType() != QAudioFormat::SignedInt);
            break;
        case eSampleBit24i:
            m_needResample = (m_format.sampleSize() != 24 || m_format.sampleType() != QAudioFormat::SignedInt);
            break;
        case eSampleBit32i:
            m_needResample = (m_format.sampleSize() != 32 || m_format.sampleType() != QAudioFormat::SignedInt);
            break;
        case eSampleBit24In32i:
            m_needResample = (m_format.sampleSize() != 32 || m_format.sampleType() != QAudioFormat::SignedInt);
            break;
        case eSampleBit32f:
            m_needResample = (m_format.sampleSize() != 32 || m_format.sampleType() != QAudioFormat::Float);
            break;
        }
    }
    m_bufferSize = m_rec.m_audioParams.sampleRate * m_rec.m_bytesPerSample * m_rec.m_audioParams.channels;
    m_pcmBuffer = new uint8_t[m_bufferSize];
    m_dataOffset = 0;
    m_dataSize = 0;
    m_sampleBegin = 0;
}

void SoundDevInfo::uninitResample()
{
    m_bufferSize = 0;
    if (m_pcmBuffer) delete []m_pcmBuffer;
    m_pcmBuffer = nullptr;
    m_dataOffset = 0;
    m_dataSize = 0;
    m_sampleBegin = 0;
}



