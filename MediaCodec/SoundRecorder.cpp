#include "SoundRecorder.h"
#include <QDebug>
#include "../VideoSynthesizer.h"

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

bool SoundRecorder::bindStream(GueeMediaStream *stream)
{
    if (m_status == Encodeing) return false;

    if ( nullptr == stream )
    {
        return false;
    }
    else
    {
        m_mediaStream = stream;
    }
    return true;
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
    if (m_sndCallback.start(fmt, false)) m_status = Opened;
    if (m_sndMicInput.start(fmt, false)) m_status = Opened;
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
        m_audioParams.sampleBits		= eSampleBit24i;
        m_bytesPerSample = 3;
        faacBits	= FAAC_INPUT_24BIT;
        break;
    case eSampleBit32i:
    case eSampleBit24In32i:
        m_audioParams.sampleBits		= eSampleBit32i;
        m_bytesPerSample = 4;
        faacBits	= FAAC_INPUT_32BIT;
        break;
    case eSampleBit32f:
        m_audioParams.sampleBits		= eSampleBit32f;
        m_bytesPerSample = 4;
        faacBits	= FAAC_INPUT_FLOAT;     //浮点格式的采样似乎播不出声音？！
        break;
    }


    ulong	bitRate	= ulong(m_audioParams.bitrate * 1024);
    ulong   samPerf = 0;
    ulong   outBufs = 0;
    m_faacHandle	= faacEncOpen( ulong(m_audioParams.sampleRate), uint(m_audioParams.channels),
                                   &samPerf, &outBufs );
    if (m_faacHandle == nullptr)
    {
        return false;
    }
    m_samplesPerFrame = samPerf;
    m_maxOutByteNum = outBufs;

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
    m_lastSampleNum = 0;

    m_sndCallback.initResample();
    m_sndMicInput.initResample();
#if _USE_WAVE_TEST
    SWaveFormat wavFmt;
    wavFmt.wFormatTag = faacBits == FAAC_INPUT_FLOAT ? 3 : 1;
    wavFmt.nChannels = m_audioParams.channels;
    wavFmt.nSamplesPerSec = m_audioParams.sampleRate;
    wavFmt.nBlockAlign = m_bytesPerSample * m_audioParams.channels;
    wavFmt.nAvgBytesPerSec = wavFmt.nBlockAlign * wavFmt.nSamplesPerSec;
    wavFmt.wBitsPerSample = m_bytesPerSample * 8;
    wavFmt.cbSize = sizeof(wavFmt);
    m_waveFile.openFile("/home/guee/Videos/1.wav", &wavFmt);
#endif

    m_status = Encodeing;
    start();
    return true;
}

void SoundRecorder::endEncode()
{
    if (m_status == Encodeing)
    {
        m_status = Opened;
#if _USE_WAVE_TEST
        m_waveFile.closeFile();
#endif
        m_waitPcmBuffer.release();
        if ( isRunning() )
        {
            wait();
        }
        if ( m_faacHandle )
        {
            faacEncClose(m_faacHandle);
            m_faacHandle	= nullptr;
        }
        m_sndCallback.uninitResample();
        m_sndMicInput.uninitResample();
    }
}

bool SoundRecorder::pauseEncode(bool pause)
{
    if (m_status == Encodeing && pause)
    {
        m_status = Paused;
        return true;
    }
    else if (m_status == Paused && !pause)
    {
        m_status = Encodeing;
        return true;
    }
    return true;
}

void SoundRecorder::run()
{

    int64_t delaySample; //当声音与实际时间发生延迟时（没有采样数据），允许的最大延迟采样数。
    delaySample = m_audioParams.sampleRate;
    int32_t bytesPerFrame;    //每个AAC帧的字节数（全部声道）
    bytesPerFrame = m_samplesPerFrame * m_bytesPerSample;
    uint8_t* mixBuf = new uint8_t[bytesPerFrame];
    uint8_t* encBuf = new uint8_t[m_maxOutByteNum];
    int64_t frameBegin = 0;//VideoSynthesizer::instance().timestamp();
    //frameBegin = frameBegin * m_audioParams.sampleRate / 1000;
    int32_t sampPerFrame = m_samplesPerFrame / m_audioParams.channels;
    int32_t encOutSize = 0;
    memset(mixBuf, 0, ulong(bytesPerFrame));

    while(m_status >= Encodeing)
    {
        if (m_waitPcmBuffer.tryAcquire(1, 100))
        {
            if (m_status < Encodeing) break;
//            qDebug() << "Time:" << VideoSynthesizer::instance().timestamp() / 1000
//                     << "PCB sample begin:" << m_sndCallback.m_sampleBegin << " ,cout:" << m_sndCallback.m_dataSize / m_bytesPerSample
//                     << "    , MIC sample:" << m_sndMicInput.m_sampleBegin << " ,cout:" << m_sndMicInput.m_dataSize / m_bytesPerSample;

            while(m_sndCallback.m_dataSize || m_sndMicInput.m_dataSize)
            {
                bool pcbOk = mixPcm(frameBegin, m_sndCallback, mixBuf);
                bool micOk = mixPcm(frameBegin, m_sndMicInput, mixBuf);
                //qDebug() << "pcbOk=" << pcbOk << m_sndCallback.m_dataSize / m_bytesPerSample << "   , micOk=" << micOk << m_sndMicInput.m_dataSize / m_bytesPerSample;
                if (pcbOk && micOk)
                {
#if _USE_WAVE_TEST
                    m_waveFile.appendSamples(sampPerFrame, mixBuf);
#endif
                    encOutSize = faacEncEncode(m_faacHandle, reinterpret_cast<int32_t*>(mixBuf),
                                               uint(m_samplesPerFrame), encBuf, uint(m_maxOutByteNum));
                    if (encOutSize)
                    {
                        m_mediaStream->putAudioFrame(encBuf, encOutSize, frameBegin * 1000 / m_audioParams.sampleRate);
                    }
                    memset(mixBuf, 0, ulong(bytesPerFrame));
                    //fprintf(stderr, "Time %d , frameBegin:%d ,encOutSize:%d\n",
                    //        int32_t(VideoSynthesizer::instance().timestamp() / 1000), int32_t(frameBegin), int32_t(encOutSize));
                    frameBegin += sampPerFrame;
                    m_lastSampleNum = qMax(m_lastSampleNum, frameBegin);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            if (m_status < Encodeing) break;
            int64_t sample = VideoSynthesizer::instance().timestamp() * m_audioParams.sampleRate / 1000000;
            while(frameBegin + delaySample <= sample)
            {
                memset(mixBuf, 0, ulong(bytesPerFrame));
#if _USE_WAVE_TEST
                m_waveFile.appendSamples(sampPerFrame, mixBuf);
#endif
                encOutSize = faacEncEncode(m_faacHandle, reinterpret_cast<int32_t*>(mixBuf),
                                           uint(m_samplesPerFrame), encBuf, uint(m_maxOutByteNum));
                if (encOutSize)
                {
                    m_mediaStream->putAudioFrame(encBuf, encOutSize, frameBegin * 1000 / m_audioParams.sampleRate);
                }
                qDebug() << "[null]Time:" << VideoSynthesizer::instance().timestamp() / 1000 << ", frameBegin:" << frameBegin << ", encOutSize:" << encOutSize;
                frameBegin += sampPerFrame;
            }

        }

    }

    encOutSize = faacEncEncode(m_faacHandle, nullptr,
                               0, encBuf, uint(m_maxOutByteNum));
    if (encOutSize)
    {
        m_mediaStream->putAudioFrame(encBuf, encOutSize, frameBegin * 1000 / m_audioParams.sampleRate);
    }
    frameBegin += sampPerFrame;
    delete []mixBuf;
    delete []encBuf;
}

bool SoundRecorder::mixPcm(int64_t frameBegin, SoundDevInfo &dev, uint8_t *mixBuf)
{
    bool ret = false;
    int32_t bytesPerSample = m_bytesPerSample * m_audioParams.channels;
    dev.m_mutexPcmBuf.lock();
    int64_t frameEnd = frameBegin + int64_t(m_samplesPerFrame / m_audioParams.channels);
    int64_t sampleEnd = dev.m_sampleBegin + dev.m_dataSize / bytesPerSample;

    //采样数据滞后超出限制，重置采样缓冲区
    if (sampleEnd <= frameBegin)
    {
        if (!dev.m_isEnabled)
        {
            dev.m_dataSize = 0;
            ret = true;
        }
        else if (sampleEnd + m_audioParams.sampleRate < frameBegin)
        {
            qDebug() << (dev.m_isCallbackType?"PCB":"MIC") << "采样数据（sampleEnd="<< sampleEnd
                     << ", dev.m_sampleBegin:" << dev.m_sampleBegin << ", dev.m_dataSize:"
                     << dev.m_dataSize << "）滞后超出限制(frameBegin="<<frameBegin<<")，重置采样缓冲区";
            dev.m_sampleBegin = m_lastSampleNum;
            dev.m_dataSize = 0;
            ret = true;
        }
    }
    else if (dev.m_sampleBegin >= frameEnd)
    {
       // qDebug() << (dev.m_isCallbackType?"PCB":"MIC") << "超出采样：" << dev.m_sampleBegin << "~" << sampleEnd << ",  frameEnd:" << frameEnd;
        ret = true;
    }
    else
    {
        while(dev.m_dataSize && !ret)
        {
            sampleEnd = dev.m_sampleBegin + qMin(dev.m_bufferSize - dev.m_dataOffset, dev.m_dataSize) / bytesPerSample;
            int64_t begin = qMax(frameBegin, dev.m_sampleBegin);
            int64_t end = qMin(frameEnd, sampleEnd);
            if (begin == end)
            {
                ret = (end == frameEnd);
                qDebug() << "00000000000000000000";
                break;
            }
            uint8_t* mixPtr = mixBuf + (begin - frameBegin) * bytesPerSample;
            uint8_t* inpPtr = dev.m_pcmBuffer
                    + (dev.m_dataOffset + ( begin - dev.m_sampleBegin ) * bytesPerSample ) % dev.m_bufferSize;
            if (m_audioParams.sampleBits == eSampleBit16i)
            {
                int16_t* pcmMix = reinterpret_cast<int16_t*>(mixPtr);
                int16_t* pcmInp = reinterpret_cast<int16_t*>(inpPtr);
                int32_t num = int32_t(end - begin) * m_audioParams.channels;
                for (int32_t i = 0; i < num; ++i)
                {
                    pcmMix[i] = int16_t(qMax(-32767,qMin(32767,int32_t(pcmMix[i]) + int32_t(pcmInp[i]))));
                }
            }
            else if (m_audioParams.sampleBits == eSampleBit32i)
            {
                int32_t* pcmMix = reinterpret_cast<int32_t*>(mixPtr);
                int32_t* pcmInp = reinterpret_cast<int32_t*>(inpPtr);
                int32_t num = int32_t(end - begin) * m_audioParams.channels;
                for (int32_t i = 0; i < num; ++i)
                {
                    //AAC的32位音频，实际上是24位的。
                    pcmMix[i] = (pcmMix[i] + (pcmInp[i]>>8));
                }
            }
            else if (m_audioParams.sampleBits == eSampleBit32f)
            {
                float* pcmMix = reinterpret_cast<float*>(mixPtr);
                float* pcmInp = reinterpret_cast<float*>(inpPtr);
                int32_t num = int32_t(end - begin) * m_audioParams.channels;
                for (int32_t i = 0; i < num; ++i)
                {
                    //AAC的浮点音频，值范围不是0~1,要放大一些倍数，具体需要放大多少没有找到资料，暂定32767。
                    pcmMix[i] = (pcmMix[i] + pcmInp[i] * 32767.0f);
                }
            }

            dev.m_dataOffset = ( dev.m_dataOffset + (end - dev.m_sampleBegin) * bytesPerSample ) % dev.m_bufferSize;
            dev.m_dataSize -= (end - dev.m_sampleBegin) * bytesPerSample;

            dev.m_sampleBegin = end;
            ret = (end == frameEnd);
        }
    }
    dev.m_mutexPcmBuf.unlock();
    return ret;
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
                    if (m_curDev.isNull())
                    {
                        m_curDev = deviceInfo;
                    }
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
                if (m_curDev.isNull())
                {
                    m_curDev = defaultDeviceInfo;
                }
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
    if (dev.isEmpty()) return false;
    availableDev(true);
    for (auto& d: m_devLst)
    {
        if (d.deviceName() == dev && m_curDev != d)
        {
            m_curDev = d;
            if (m_isDevOpened)
            {
                return start(m_format, false);
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
        if (m_isDevOpened)
        {
            if (m_audioInput)
            {
                if (enable)
                {
                    m_dataOffset = 0;
                    m_dataSize = 0;
                    m_sampleBegin = m_rec.m_lastSampleNum;
                    m_prevRealse = 0;
                    m_time.start();
                    m_audioInput->resume();
                    qDebug() << "resume err:" << m_audioInput->error() << ", state:" << m_audioInput->state();
                }
                else
                {
                    m_time.pause();
                    m_audioInput->suspend();
                    qDebug() << "resume err:" << m_audioInput->error() << ", state:" << m_audioInput->state();
                    m_curAmplitude = 0;
                }
            }
            else if (enable)
            {
                start(m_format, true);
            }
        }
        m_isEnabled = enable;
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

bool SoundDevInfo::start(const QAudioFormat &format, bool checkDev)
{
    if (m_devLst.isEmpty())
    {
        availableDev(true);
        checkDev = false;
    }
    if (m_curDev.isNull())
        return false;
    if (m_audioInput)
    {
        stop();
    }
    if (checkDev)
    {
        availableDev(true);
        for (auto& d: m_devLst)
        {
            if (d.deviceName() == m_curDev.deviceName())
            {
                m_curDev = d;
                break;
            }
        }
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
   // fmt.setByteOrder(QAudioFormat::BigEndian);

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
    m_realSample = 0;
    m_time.start();
    open(QIODevice::WriteOnly);
    m_audioInput->start(this);
    if ( !m_isEnabled )
    {
        m_audioInput->suspend();
    }
    fmt = m_audioInput->format();
    qDebug() << (m_isCallbackType ? "PCB":"MIC");
    qDebug() << "fmt.bytesPerFrame" << fmt.bytesPerFrame();
    qDebug() << "fmt.sampleRate:" << fmt.sampleRate();
    qDebug() << "fmt.channelCount:" << fmt.channelCount();
    qDebug() << "fmt.sampleSize:" << fmt.sampleSize();
    qDebug() << "fmt.sampleType:" << fmt.sampleType();
    qDebug() << "fmt.byteOrder:" << fmt.byteOrder();
    qDebug() << "fmt.codec:" << fmt.codec();

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
    m_realSample = 0;
    m_time.stop();

}

qint64 SoundDevInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)

    return 0;
}

qint64 SoundDevInfo::writeData(const char *data, qint64 len)
{
    int32_t inpSample = int32_t(len / m_format.bytesPerFrame());
    m_realSample += inpSample;

    if (m_rec.m_status == SoundRecorder::Encodeing)
    {
        int32_t num = int32_t(len);
        const char* dat = data;

        int64_t needSample = m_rec.m_audioParams.sampleRate * VideoSynthesizer::instance().timestamp() / 1000000;
        float scale = float(double(realSampleRate()) / double(m_rec.m_audioParams.sampleRate));
        if (scale < 0.9f)
        {
            needSample -= (m_sampleBegin + m_dataSize / m_format.bytesPerFrame());
            needSample = qMin(int32_t(needSample), m_rec.m_audioParams.sampleRate);
            scale = float(inpSample) / float(needSample);
            float pos = scale - 0.50001f;
            if (m_rec.m_audioParams.sampleBits == eSampleBit16i)
            {
                int16_t* pcmOut = reinterpret_cast<int16_t*>(m_resBuffer);
                const int16_t* pcmInp = reinterpret_cast<const int16_t*>(dat);
                const int16_t* pcmPre = reinterpret_cast<const int16_t*>(m_pcmBuffer + ((m_dataSize + m_dataOffset - m_format.bytesPerFrame()) % m_bufferSize));
                if (m_format.channelCount()==1)
                {
                    for (int i = 0; i < needSample; ++i)
                    {
                        int nex = int(pos + 0.5f);
                        float dn = pos + 0.5f - nex;

                        if (nex == 0)
                        {
                            *pcmOut = int16_t(*pcmPre * (1.0f - dn) + pcmInp[nex] * dn);
                        }
                        else
                        {
                            *pcmOut = int16_t(pcmInp[nex-1] * (1.0f - dn) + pcmInp[nex] * dn);
                        }
                        ++pcmOut;
                        pos += scale;
                    }
                }
                else if (m_format.channelCount() == 2)
                {
                    for (int i = 0; i < needSample; ++i)
                    {
                        int nex = int(pos + 0.5f);
                        float dn = pos + 0.5f - nex;

                        if (nex == 0)
                        {
                            pcmOut[0] = int16_t(pcmPre[0] * (1.0f - dn) + pcmInp[nex * 2] * dn);
                            pcmOut[1] = int16_t(pcmPre[1] * (1.0f - dn) + pcmInp[nex * 2 + 1] * dn);
                        }
                        else
                        {
                            pcmOut[0] = int16_t(pcmInp[nex * 2 - 2] * (1.0f - dn) + pcmInp[nex * 2] * dn);
                            pcmOut[1] = int16_t(pcmInp[nex * 2 - 1] * (1.0f - dn) + pcmInp[nex * 2 + 1] * dn);
                        }
                        pcmOut += 2;
                        pos += scale;
                    }
                }
            }
            else if (m_rec.m_audioParams.sampleBits == eSampleBit32i)
            {
                int32_t* pcmOut = reinterpret_cast<int32_t*>(m_resBuffer);
                const int32_t* pcmInp = reinterpret_cast<const int32_t*>(dat);
                const int32_t* pcmPre = reinterpret_cast<const int32_t*>(m_pcmBuffer + ((m_dataSize + m_dataOffset - m_format.bytesPerFrame()) % m_bufferSize));
                if (m_format.channelCount()==1)
                {
                    for (int i = 0; i < needSample; ++i)
                    {
                        int nex = int(pos + 0.5f);
                        float dn = pos + 0.5f - nex;

                        if (nex == 0)
                        {
                            *pcmOut = int32_t(*pcmPre * (1.0f - dn) + pcmInp[nex] * dn);
                        }
                        else
                        {
                            *pcmOut = int32_t(pcmInp[nex-1] * (1.0f - dn) + pcmInp[nex] * dn);
                        }
                        ++pcmOut;
                        pos += scale;
                    }
                }
                else if (m_format.channelCount() == 2)
                {
                    for (int i = 0; i < needSample; ++i)
                    {
                        int nex = int(pos + 0.5f);
                        float dn = pos + 0.5f - nex;

                        if (nex == 0)
                        {
                            pcmOut[0] = int32_t(pcmPre[0] * (1.0f - dn) + pcmInp[nex * 2] * dn);
                            pcmOut[1] = int32_t(pcmPre[1] * (1.0f - dn) + pcmInp[nex * 2 + 1] * dn);
                        }
                        else
                        {
                            pcmOut[0] = int32_t(pcmInp[nex * 2 - 2] * (1.0f - dn) + pcmInp[nex * 2] * dn);
                            pcmOut[1] = int32_t(pcmInp[nex * 2 - 1] * (1.0f - dn) + pcmInp[nex * 2 + 1] * dn);
                        }
                        pcmOut += 2;
                        pos += scale;
                    }
                }
            }
            else if (m_rec.m_audioParams.sampleBits == eSampleBit32f)
            {
                float* pcmOut = reinterpret_cast<float*>(m_resBuffer);
                const float* pcmInp = reinterpret_cast<const float*>(dat);
                const float* pcmPre = reinterpret_cast<const float*>(m_pcmBuffer + ((m_dataSize + m_dataOffset - m_format.bytesPerFrame()) % m_bufferSize));
                if (m_format.channelCount()==1)
                {
                    for (int i = 0; i < needSample; ++i)
                    {
                        int nex = int(pos + 0.5f);
                        float dn = pos + 0.5f - nex;

                        if (nex == 0)
                        {
                            *pcmOut = float(*pcmPre * (1.0f - dn) + pcmInp[nex] * dn);
                        }
                        else
                        {
                            *pcmOut = float(pcmInp[nex-1] * (1.0f - dn) + pcmInp[nex] * dn);
                        }
                        ++pcmOut;
                        pos += scale;
                    }
                }
                else if (m_format.channelCount() == 2)
                {
                    for (int i = 0; i < needSample; ++i)
                    {
                        int nex = int(pos + 0.5f);
                        float dn = pos + 0.5f - nex;

                        if (nex == 0)
                        {
                            pcmOut[0] = float(pcmPre[0] * (1.0f - dn) + pcmInp[nex * 2] * dn);
                            pcmOut[1] = float(pcmPre[1] * (1.0f - dn) + pcmInp[nex * 2 + 1] * dn);
                        }
                        else
                        {
                            pcmOut[0] = float(pcmInp[nex * 2 - 2] * (1.0f - dn) + pcmInp[nex * 2] * dn);
                            pcmOut[1] = float(pcmInp[nex * 2 - 1] * (1.0f - dn) + pcmInp[nex * 2 + 1] * dn);
                        }
                        pcmOut += 2;
                        pos += scale;
                    }
                }
            }

            num = int32_t(needSample) * m_format.bytesPerFrame();
            dat = reinterpret_cast<char*>(m_resBuffer);
        }


        //qDebug() << (m_isCallbackType?"PCB":"MIC") << "录音time:" << VideoSynthesizer::instance().timestamp() / 1000 << "Sample Begin:" << m_sampleBegin << ", Inpu Sample:" << len / (m_rec.m_bytesPerSample * m_rec.m_audioParams.channels) << "(" << len << ")";
        m_mutexPcmBuf.lock();
        //如果输入的字节数大于缓冲区剩余字节数，需要抛弃前面的数据。
        if (num > m_bufferSize - m_dataSize)
        {
            //计算需要抛弃的字节数
            int32_t skip = num - (m_bufferSize - m_dataSize);
            qDebug() << (m_isCallbackType?"PCB":"MIC") << "Smple:" << m_sampleBegin << ", 输入的音频数据：" << num <<"大于m_bufferSize(" << m_bufferSize << ")-m_dataSize(" << m_dataSize << ")，需要跳过："<<skip;
            if ( skip > m_dataSize)
            {
                //需要抛弃的字节数大于了现有数据，就还需要抛弃一部分输入数据
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
            //qDebug() << "抛弃音频采样数据" << skip;
            //抛弃数据后重新计算数据开始位置对应的采样起始位置。
            m_sampleBegin += skip / m_format.bytesPerFrame();
        }
        while(num)
        {
            //向缓冲区写入数据，剩余的输入数据长度已经小余缓冲区大小。
            int32_t endOffset = (m_dataOffset + m_dataSize) % m_bufferSize;
            int32_t cpySize = qMin(m_bufferSize - endOffset, num);
            memcpy(m_pcmBuffer + endOffset, dat, ulong(cpySize));
            m_dataSize += cpySize;
            num -= cpySize;
            dat += cpySize;
        }
        int64_t simp = m_sampleBegin + m_dataSize / m_format.bytesPerFrame();
        m_rec.m_lastSampleNum = qMax(m_rec.m_lastSampleNum, simp);
        if (simp - m_prevRealse >= m_rec.m_samplesPerFrame * 2)
        {
           // qDebug() << (m_isCallbackType?"PCB":"MIC") << "relese wait, beg sample:"<< m_sampleBegin << simp << "   data size = " << m_dataSize;
            m_prevRealse = simp;
            m_rec.m_waitPcmBuffer.release();
        }
        m_mutexPcmBuf.unlock();
        //qDebug() << (m_isCallbackType?"PCB":"MIC") << "累计sample:" << m_dataSize /(m_rec.m_bytesPerSample * m_rec.m_audioParams.channels)  << "(" << m_dataSize << ")";
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

    m_curAmplitude /= QAudio::convertVolume(m_volume,
                           QAudio::LogarithmicVolumeScale,QAudio::LinearVolumeScale
                           );
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
    m_resBuffer = new uint8_t[m_bufferSize];
    m_bufferSize *= 3;
    m_pcmBuffer = new uint8_t[m_bufferSize];
    m_dataOffset = 0;
    m_dataSize = 0;
    m_sampleBegin = m_rec.m_lastSampleNum;
    m_prevRealse = 0;
    memset(m_prevSample, 0, sizeof(m_prevSample));
}

void SoundDevInfo::uninitResample()
{
    m_bufferSize = 0;
    if (m_pcmBuffer) delete []m_pcmBuffer;
    m_pcmBuffer = nullptr;
    if (m_resBuffer) delete []m_resBuffer;
    m_resBuffer = nullptr;
    m_dataOffset = 0;
    m_dataSize = 0;
    m_sampleBegin = 0;
}



