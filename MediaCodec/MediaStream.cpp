#include "MediaStream.h"
#include "MediaWriter.h"
#include <cmath>
#include <QDebug>

GueeMediaStream::GueeMediaStream()
{
	memset(&m_videoParams, 0, sizeof(m_videoParams));
	memset(&m_audioParams, 0, sizeof(m_audioParams));
	m_curFrame = (H264Frame*)malloc(sizeof(H264Frame) + sizeof(H264Frame::NAL) * 256);
	m_videoDelayFrame = 0;
	restartMember();
}


GueeMediaStream::~GueeMediaStream()
{
	if (m_curFrame ) free(m_curFrame);
	m_curFrame = nullptr;
}

void GueeMediaStream::restartMember()
{
	m_onReadFile = nullptr;
	m_hasVideo = false;
	m_hasAudio = false;
	m_isOpened = false;
	m_headIsGeted = false;

	m_sps.clear();
	m_pps.clear();
	m_sei.clear();

	m_videoDelayFrame = 0;
	m_videoDelayMS = 0;
	m_videoInitDelta = 0;
	m_dtsCompress = false;
	m_videoFrameNum = 0;
	m_videoDuration = 0;
	m_videoTotalSize = 0;

	m_audioFrameNum = 0;
	m_audioDelay = 0;
	m_audioDuration = 0;
	m_audioTotalSize = 0;
	memset(m_audioSpecificConfig, 0, sizeof(m_audioSpecificConfig));
	m_audioSpecificConfigSize = 0;

	m_videoStreamBegin = false;
	m_audioStreamBegin = false;
	m_autoCheckAnnexb = false;
	m_nalBufLoc = false;
	m_nalBuffer.clear();
	m_lastVideo.clear();
	m_lastAudio.clear();
	if (m_curFrame) memset(m_curFrame, 0, sizeof(H264Frame) + sizeof(H264Frame::NAL) * 256);
}

bool GueeMediaStream::setVideoParams(const SVideoParams & params)
{
	m_videoParams = params;
	return true;
}

bool GueeMediaStream::setAudioParams(const SAudioParams & params)
{
	m_audioParams = params;
	return true;
}

bool GueeMediaStream::appendWriter(GueeMediaWriter * writer)
{
	if (writer == nullptr) return false;
	for (auto i = m_writers.begin(); i != m_writers.end(); ++i)
	{
		if (*i == writer) return true;
	}
	m_writers.push_back(writer);
	return true;
}

bool GueeMediaStream::removeWriter(GueeMediaWriter * writer)
{
	if (writer == nullptr) return false;
	for (auto i = m_writers.begin(); i != m_writers.end(); ++i)
	{
		if (*i == writer)
		{
			m_writers.erase(i);
			return true;
		}
	}
	return false;
}

bool GueeMediaStream::startParse(IOnReadFile* cbRead)
{
	m_mutexWrite.lock();
	if (m_isOpened)
	{
		m_mutexWrite.unlock();
		return false;
	}
	restartMember();
	m_onReadFile = cbRead;
    int32_t	failCount = 0;
    for ( auto w : m_writers)
    {
        if (!w->m_isEnabled) continue;
        if (!w->onOpenWrite())
        {
            ++failCount;
        }
    }
    if (failCount == int32_t(m_writers.size()))
	{
		m_mutexWrite.unlock();
		endParse();
		return false;
	}
	m_mutexWrite.unlock();
	m_isOpened = true;
	if (m_isOpened && m_onReadFile)
	{
		if (!getNextFrame())
		{
			endParse();
		}
	}
	return m_isOpened;
}

void GueeMediaStream::endParse()
{
	if (m_isOpened)
	{
        if (!m_lastVideo.isEmpty())
		{
            putVideoSlice( reinterpret_cast<const uint8_t*>(m_lastVideo.data()), m_lastVideo.length());
		}
		m_lastVideo.clear();
		m_lastAudio.clear();

		putVideoSlice(nullptr, 0);
		writeToCache(nullptr, nullptr);

		m_mutexWrite.lock();
        for (int32_t i = 0; i < int32_t(m_writers.size()); ++i)
			m_writers[i]->onCloseWrite();
		m_isOpened = false;
		for (auto i = m_frameCaches.begin(); i != m_frameCaches.end(); ++i)
		{
			if (i->second.aud) free(i->second.aud);
			if (i->second.vid) free(i->second.vid);
		}
		m_frameCaches.erase(m_frameCaches.begin(), m_frameCaches.end());
		m_frameCaches = map<int64_t, FrameCache>();
        m_nalBuffer.clear();
		m_mutexWrite.unlock();
	}
}

bool GueeMediaStream::getNextFrame()
{
	return m_onReadFile ? onGetNextFrame() : false;
}

bool GueeMediaStream::putFileStream(const uint8_t * data, int32_t length)
{
	return m_onReadFile ? false : onPutFileStream(data, length);
}

bool GueeMediaStream::onGetNextFrame()
{
	return false;
}

bool GueeMediaStream::onPutFileStream(const uint8_t * data, int32_t length)
{
    Q_UNUSED(data);
    Q_UNUSED(length);
    return false;
}

bool GueeMediaStream::putVideoStream(const uint8_t * data, int32_t length)
{
	if (!m_isOpened || data == nullptr || length <= 0) return false;
	if (!m_videoParams.enabled) return true;
	if (!m_videoStreamBegin)
	{
#define	D(i) ((i)<m_lastVideo.length() ? m_lastVideo[i] : data[i-m_lastVideo.length()])
		int32_t		size = int32_t(m_lastVideo.length() + length - 2);
		for (int32_t i = 0; i <= size; ++i)
		{
			if (D(i) == 0 && D(i+1) == 0 && D(i+2) == 0 && D(i+3) == 1)
			{
				if (i < m_lastVideo.length())
				{
                    memmove(m_lastVideo.data(), m_lastVideo.data() + i, m_lastVideo.length() - i);
					m_lastVideo.resize(m_lastVideo.length() - i);
				}
				else
				{
					i -= (int32_t)m_lastVideo.length();
					m_lastVideo.clear();
					data += i;
					length -= i;
				}
				m_videoStreamBegin = true;
				m_videoParams.annexb = true;
				m_nalBufLoc = false;
				break;
			}
		}
		if (!m_videoStreamBegin)
		{
			if (length >= 3)
			{
				m_lastVideo.clear();
				m_lastVideo.append((char*)data + length - 3, 3);
			}
			else
			{
				m_lastVideo.append((char*)data, length);
			}
			return true;
		}
	}
	if (length <= 0) return true;
    if (!m_lastVideo.isEmpty())
	{
		if (m_lastVideo.size() > 6 && length >= 3)
		{
			char sp[6] = { m_lastVideo[m_lastVideo.size() - 3], m_lastVideo[m_lastVideo.size() - 2], m_lastVideo[m_lastVideo.size() - 1],
                char(data[0]), char(data[1]), char(data[2]) };
			for (int32_t i = 0; i < 3; ++i)
			{
				if (sp[i + 0] == 0 && sp[i + 1] == 0 && (sp[i + 2] == 1 || (sp[i + 2] == 0 && sp[i + 3] == 1)))
				{
					m_curFrame->dts = 0;
					m_curFrame->pts = 0;
                    putVideoSlice( reinterpret_cast<const uint8_t*>(m_lastVideo.data()), m_lastVideo.length() - (3 - i));
					m_lastVideo.clear();
					m_lastVideo.append(sp + i, 3 - i);
					break;
				}
			}
		}
		const uint8_t*	frameEnd = data + max(0, int32_t(6 - m_lastVideo.size()));
		const uint8_t*	dataEnd = data + length - 4;
		while (frameEnd <= dataEnd)
		{
			if (frameEnd[0] == 0 && frameEnd[1] == 0 && (frameEnd[2] == 1 || (frameEnd[2] == 0 && frameEnd[3] == 1)))
			{
				int32_t size = int32_t(frameEnd - data);
				if (size)m_lastVideo.append((const char*)data, size);
				m_curFrame->dts = 0;
				m_curFrame->pts = 0;
                putVideoSlice( reinterpret_cast<const uint8_t*>(m_lastVideo.data()), m_lastVideo.length());
                m_lastVideo.clear();
				data += size;
				length -= size;
				break;
			}
			++frameEnd;
		}
		if (frameEnd > dataEnd)
		{
            m_lastVideo.append(reinterpret_cast<const char*>(data), length);
			return true;
		}
	}
	const uint8_t*	frameEnd = data + 4;
	const uint8_t*	dataEnd = data + length - 4;
	while (frameEnd <= dataEnd)
	{
		if (frameEnd[0] == 0 && frameEnd[1] == 0 && (frameEnd[2] == 1 || (frameEnd[2] == 0 && frameEnd[3] == 1)))
		{
			int32_t size = int32_t(frameEnd - data);
			m_curFrame->dts = 0;
			m_curFrame->pts = 0;
			putVideoSlice(data, size);
			data += size;
			length -= size;
			frameEnd = data + 4;
		}
		++frameEnd;
	}
	if (frameEnd > dataEnd && length)
	{
        m_lastVideo.append(reinterpret_cast<const char*>(data), length);
	}
	return true;
}

bool GueeMediaStream::putAudioStream(const uint8_t * data, int32_t length)
{
	if (!m_isOpened ) return false;
	if (!m_audioParams.enabled) return true;
	if (!m_audioStreamBegin)
	{
		#define	DA(i) ((i)<m_lastAudio.length() ? m_lastAudio[i] : data[i-m_lastAudio.length()])
		int32_t		size = int32_t( m_lastAudio.length() + length - 2 );
		for (int32_t i = 0; i <= size; ++i)
		{
			if (DA(i) == 0xFF && (DA(i + 1) & 0xF0) == 0xF0)
			{
				if (i < m_lastAudio.length())
				{
                    memmove(m_lastAudio.data(), m_lastAudio.data() + i, size_t(m_lastAudio.length() - i));
					m_lastAudio.resize(m_lastAudio.length() - i);
				}
				else
				{
                    i -= m_lastAudio.length();
					m_lastAudio.clear();
					data += i;
					length -= i;
				}
				m_audioStreamBegin = true;
				break;
			}
		}
		if (!m_audioStreamBegin)
		{
            m_lastAudio.append(1, char(data[length - 1]));
			return true;
		}
	}
    if (!m_lastAudio.isEmpty())
	{
		if (m_lastAudio.length() < 7)
		{
			int32_t	get = min(int32_t(7 - m_lastAudio.length()), length);
            m_lastAudio.append(reinterpret_cast<const char*>(data), get);
			data += get;
			length -= get;
		}
		if (m_lastAudio.length() < 7) return true;
        unsigned char* buff = reinterpret_cast<unsigned char*>(m_lastAudio.data());
		int32_t aac_frame_length = (buff[3] & 3) << 11 | buff[4] << 3 | buff[5] >> 5;
		int32_t	get = min(int32_t(aac_frame_length - m_lastAudio.length()), length);
        m_lastAudio.append(reinterpret_cast<const char*>(data), get);
		data += get;
		length -= get;
		if (m_lastAudio.length() < aac_frame_length) return true;
        putAudioFrame(reinterpret_cast<const uint8_t*>(m_lastAudio.data()), aac_frame_length, 0);
		m_lastAudio.clear();
	}
	while (length)
	{
		if (length >= 7)
		{
			int32_t aac_frame_length = (data[3] & 3) << 11 | data[4] << 3 | data[5] >> 5;
			if (length >= aac_frame_length)
			{
				putAudioFrame(data, aac_frame_length, 0);
				data += aac_frame_length;
				length -= aac_frame_length;
			}
			else
			{
				m_lastAudio.append((const char*)data, length);
				break;
			}
		}
		else
		{
			m_lastAudio.append((const char*)data, length);
			break;
		}
	}
	return true;
}

bool GueeMediaStream::putVideoFrame(H264Frame * frame)
{
	if (!m_isOpened ||
		frame == nullptr || frame->payload <= 0 ||
		frame->nalCount <= 0 || frame->nals->nalData == nullptr) return false;
	if (!m_videoParams.enabled) return true;
	if (frame != m_curFrame)
	{
		m_curFrame->pts = frame->pts;
		m_curFrame->dts = frame->dts;
		if (m_videoFrameNum == 0 && m_autoCheckAnnexb)
		{
			m_videoParams.annexb = (frame->nals->nalData[0] == 0 && frame->nals->nalData[1] == 0
				&& frame->nals->nalData[2] == 0 && frame->nals->nalData[3] == 1);
		}
		m_nalBufLoc = true;
		for (int32_t i = 0; i < frame->nalCount; ++i)
		{
			putVideoSlice(frame->nals[i].nalData, frame->nals[i].nalSize);
		}
		return putVideoSlice(nullptr, 0);
	}
	if ( 0 == frame->pts && 0 == frame->dts)
	{
		frame->dts = frame->pts = m_videoFrameNum;
	}
	if (m_videoFrameNum == 0)
	{
		bool	idr = false;
		for (int32_t i = 0; i < frame->nalCount; ++i)
		{
			const H264Frame::NAL& nal = frame->nals[i];
			if (nal.nalType == NalSlice_idr)
			{
				idr = true;
				break;
			}
		}
		if (!idr) return true;
		m_videoDelayFrame = m_videoParams.BFrames ? (m_videoParams.BFramePyramid ? 2 : 1) : 0;
		m_videoDelayMS = convertTimebaseMs(frame->dts * -1);
		m_hasVideo = true;
		m_videoStreamBegin = true;
	}

	if (m_dtsCompress)
	{
		if (1 == m_videoFrameNum)
			m_videoInitDelta = convertTimebaseMs(frame->dts) + m_videoDelayMS;
		frame->dtsTimeMS = m_videoFrameNum > m_videoDelayFrame
			? convertTimebaseMs(frame->dts)
			: m_videoFrameNum * m_videoInitDelta / (m_videoDelayFrame + 1);
		frame->ptsTimeMS = convertTimebaseMs(frame->pts);
	}
	else
	{
		frame->dtsTimeMS = convertTimebaseMs(frame->dts) + m_videoDelayMS;
		frame->ptsTimeMS = convertTimebaseMs(frame->pts) + m_videoDelayMS;
	}
	if (!writeToCache(frame, nullptr)) return false;
	m_videoTotalSize += frame->payload;
	++m_videoFrameNum;
    m_videoDuration = frame->dtsTimeMS;
	return true;
}

bool GueeMediaStream::putAudioFrame(AUDFrame * frame)
{
	if (!m_isOpened ||
		frame == nullptr || frame->size <= 0 || frame->data == nullptr ) return false;
	if (!m_audioParams.enabled) return true;
	if (0 == m_audioFrameNum)
	{
		if (frame->data[0] == 0xFF && (frame->data[1] & 0xF0) == 0xF0)
		{
			parseADTS(frame->data);
		}
		m_audioDelay = frame->timestamp * -1;
		m_hasAudio = true;
		m_audioStreamBegin = true;
		makeAAC_SequenceHeader();
	}
	if (frame->timestamp == 0)
	{
        frame->timestamp = m_audioFrameNum * m_aduioFrameSamples * 1000 / m_audioParams.sampleRate;
	}
	else
	{
		frame->timestamp += m_audioDelay;
	}
	if (!writeToCache(nullptr, frame)) return false;
	m_audioTotalSize += frame->size;
	++m_audioFrameNum;
	m_audioDuration = frame->timestamp;
	return true;
}

bool GueeMediaStream::writeToCache(H264Frame* vid, AUDFrame* aud)
{
	bool	append = false;
	bool	success = true;
    int64_t	times = vid ? vid->dtsTimeMS : (aud ? aud->timestamp : 0x7FFFFFFFFFFFFFFF);
	m_mutexWrite.lock();
	if (!m_headIsGeted)
	{
		append = true;		//还没有写入文件头，必须先缓存数据。
        if ( (!m_videoParams.enabled || ( !m_sps.isEmpty() && !m_pps.isEmpty() ))
			&& (!m_audioParams.enabled || m_audioSpecificConfigSize) )
		{
			if (!onWriteHeader())
			{
				m_mutexWrite.unlock();
				return false;
			}
			else
			{
				append = false;
				m_headIsGeted = true;
			}
		}
	}

	if (m_headIsGeted && m_videoParams.enabled && m_audioParams.enabled)
	{
		auto	last = m_frameCaches.upper_bound(times);

		if ((vid || aud) && last == m_frameCaches.end())	//如果找不到时间戳比当前时间戳大的，说明当前帧将成为最后一帧
		{
			append = true;	//最后一帧需要缓存下来。
			while (last != m_frameCaches.begin())
			{
				--last;
				if ( (vid && last->second.aud) || (aud && last->second.vid) )	//从后往前找到不同类型的一帧
				{
					if (last->first == times)	//如果找到的那一帧和当前时间戳相等，那么当前帧就不需要缓存
						append = false;
					++last;
					break;
				}
			}
		}
		for (auto i = m_frameCaches.begin(); i != last && success; ++i)
		{
			if (i->second.vid)
			{
				if (success) success = onWriteVideo(i->second.vid);
				free(i->second.vid);
				i->second.vid = nullptr;
			}
			if (i->second.aud)
			{
				if (success) success = onWriteAudio(i->second.aud);
				free(i->second.aud);
				i->second.aud = nullptr;
			}
		}
		m_frameCaches.erase(m_frameCaches.begin(), last);
	
	}
	if (success)
	{
		if (vid)
		{
			if (append)
			{
				H264Frame*	newFrame = (H264Frame*)malloc(sizeof(H264Frame) + sizeof(H264Frame::NAL) * vid->nalCount + vid->payload);
				if (newFrame == nullptr) return false;
				memcpy(newFrame, vid, sizeof(H264Frame) + sizeof(H264Frame::NAL) * vid->nalCount);
				uint8_t*	nalData = ((uint8_t*)newFrame) + (sizeof(H264Frame) + sizeof(H264Frame::NAL) * vid->nalCount);
				memcpy(nalData, vid->nals->nalData, vid->payload);
				for (int32_t i = 0; i < vid->nalCount; ++i)
				{
					newFrame->nals[i].nalData = nalData;
					nalData += newFrame->nals[i].nalSize;
				}
				m_frameCaches[times].vid = newFrame;
			}
			else
			{
				success = onWriteVideo(vid);
			}
		}
		else if (aud)
		{
			if (append)
			{
				AUDFrame*	newFrame = (AUDFrame*)malloc(sizeof(AUDFrame) + aud->size);
				if (newFrame == nullptr)
				{
					m_mutexWrite.unlock();
					return false;
				}
				newFrame->timestamp = aud->timestamp;
				newFrame->size = aud->size;
				newFrame->data = (uint8_t*)(newFrame + 1);
				memcpy(newFrame + 1, aud->data, aud->size);
				m_frameCaches[times].aud = newFrame;
			}
			else
			{
				success = onWriteAudio(aud);
			}
		}
	}
	m_mutexWrite.unlock();
	return success;
}

bool GueeMediaStream::putVideoFrame(const uint8_t * frame, int32_t length, int64_t pts, int64_t dts)
{
	if (!m_isOpened || frame == nullptr || length <= 4) return false;
	if (!m_videoParams.enabled) return true;
	memset(m_curFrame, 0, sizeof(H264Frame));
	m_curFrame->dts = dts;
	m_curFrame->pts = pts;
	if (!m_videoStreamBegin)
	{
		if (m_autoCheckAnnexb) m_videoParams.annexb = (frame[0] == 0 && frame[1] == 0 && frame[2] == 0 && frame[3] == 1);
		m_videoStreamBegin = true;
		m_nalBufLoc = true;
	}
	if (m_videoParams.annexb)
	{
		const uint8_t * frameEnd = frame + 4;
		const uint8_t * dataEnd = frame + length - 4;
		while (frameEnd <= dataEnd)
		{
			if (frameEnd[0] == 0 && frameEnd[1] == 0 && (frameEnd[2] == 1 || (frameEnd[2] == 0 && frameEnd[3] == 1)))
			{
				putVideoSlice(frame, int32_t(frameEnd - frame));
				frame = frameEnd;
				frameEnd += 4;
			}
			++frameEnd;
		}
		putVideoSlice(frame, int32_t(dataEnd + 4 - frame));
	}
	else
	{
		while (length > 0)
		{
			int32_t	len = endianFix32(*((uint32_t*)frame)) + 4;
			if (len > length) return false;
			putVideoSlice(frame, len);
			length -= len;
			frame += len;
		}
	}
	return putVideoSlice(nullptr, 0);
}

bool GueeMediaStream::putAudioFrame(const uint8_t * frame, int32_t length, int64_t timestamp)
{
	if (!m_isOpened || frame == nullptr || length <= 0) return false;
	if (!m_audioParams.enabled) return true;
	AUDFrame	aud = { timestamp, length, frame };
	return putAudioFrame(&aud);
}

bool GueeMediaStream::putVideoSlice(const uint8_t * data, int32_t length)
{
	int32_t	prefixOrg = 0;
	int32_t	prefix = 0;
	NalUnitType	nalType = NalUnknown;
	bool	sliceBegin = false;
	bool	groupBegin = false;
	bool	success = true;
	if (length && data)
	{
		prefixOrg = (m_videoParams.annexb && data[2] == 1) ? 3 : 4;
		data += prefixOrg;
		length -= prefixOrg;
		nalType = NalUnitType(data[0] & 0x1F);
		sliceBegin = (data[1] & 0x80) ? true : false;
		switch (nalType)
		{
		case NalSlice:
		case NalSlice_dpa:
		case NalSlice_dpb:
		case NalSlice_dpc:
		case NalSlice_idr:
			if (nalType == NalSlice_idr) m_curFrame->isKeyFrame = true;
			if (sliceBegin)
			{
				if (m_curFrame->nalCount && m_curFrame->nals[0].nalType == NalAud)
				{
					prefix = 3;
				}
				else
				{
					prefix = 4;
					groupBegin = true;
				}
			}
			else
			{
				prefix = 3;
			}
			break;
		case NalSei:
			checkSpsPpsSei(NalSei, data, length);
			prefix = 3;
			break;
		case NalSps:
			m_curFrame->isKeyFrame = true;
			checkSpsPpsSei(NalSps, data, length);
			prefix = 4;
			m_hasVideo = true;
			break;
		case NalPps:
			m_curFrame->isKeyFrame = true;
			checkSpsPpsSei(NalPps, data, length);
			prefix = 4;
			break;
		case NalAud:
			prefix = 4;
			groupBegin = true;
			break;
		default:
			prefix = 3;
		}
	}
	else
	{
		groupBegin = true;
	}

	if (groupBegin)
	{
		if (m_curFrame->nalCount)
		{
			if (!m_nalBufLoc)
			{
                uint8_t * nals = reinterpret_cast<uint8_t*>(m_nalBuffer.data());
				for (int32_t i = 0; i < m_curFrame->nalCount; ++i)
				{
					m_curFrame->nals[i].nalData = nals;
					nals += m_curFrame->nals[i].nalSize;
				}
			}
			success = putVideoFrame(m_curFrame);
			m_curFrame->nalCount = 0;
			m_curFrame->nals->nalType = NalUnknown;
			m_curFrame->isKeyFrame = false;
			m_curFrame->payload = 0;
		}
		m_nalBuffer.clear();
	}
	if (prefixOrg)
	{
		if (m_nalBufLoc)
		{
			prefix = prefixOrg;
			m_curFrame->nals[m_curFrame->nalCount].nalData = data - prefix;
		}
		else
		{
			if (m_videoParams.annexb)
			{
				m_nalBuffer.append(prefix == 3 ? "\0\0\1" : "\0\0\0\1", prefix);
			}
			else
			{
				uint32_t	bt = endianFix32(length);
				m_nalBuffer.append((char*)&bt, 4);
				prefix = 4;
			}
			m_nalBuffer.append((const char*)data, length);
		}
		m_curFrame->nals[m_curFrame->nalCount].nalType = nalType;
		m_curFrame->nals[m_curFrame->nalCount].nalSize = length + prefix;
		m_curFrame->payload += m_curFrame->nals[m_curFrame->nalCount].nalSize;
		++m_curFrame->nalCount;
	}
	return success;
}

bool GueeMediaStream::onWriteHeader()
{
    for ( auto w : m_writers)
    {
        if (!w->m_isEnabled) continue;
        if (!w->onWriteHeader())
        {
        }
    }
	return true;
}
bool GueeMediaStream::onWriteVideo(const H264Frame * frame)
{
    for ( auto w : m_writers)
    {
        if (!w->m_isEnabled) continue;
        if (!w->onWriteVideo(frame))
        {
        }
    }
	return true;
}
bool GueeMediaStream::onWriteAudio(const AUDFrame * frame)
{
    for ( auto w : m_writers)
    {
        if (!w->m_isEnabled) continue;
        if (!w->onWriteAudio(frame))
        {
        }
    }
	return true;
}

void GueeMediaStream::checkSpsPpsSei(NalUnitType type, const uint8_t * data, int32_t length)
{
	switch (type)
	{
	case NalSei:
        if (m_sei.size() != length || memcmp(m_sei.data(), data, length) != 0)
		{
			m_sei.resize(length);
            memcpy(m_sei.data(), data, length);
		}
		break;
	case NalSps:
        if (m_sps.size() != length || memcmp(m_sps.data(), data, length) != 0)
		{
			m_sps.resize(length);
            memcpy(m_sps.data(), data, length);
			h264_decode_sps(data, length);

		}
		break;
	case NalPps:
        if (m_pps.size() != length || memcmp(m_pps.data(), data, length) != 0)
		{
			m_pps.resize(length);
            memcpy(m_pps.data(), data, length);
		}
		break;
    default:
        break;
	}
}

uint32_t GueeMediaStream::Ue(uint8_t *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
	//计算0bit的个数  
	uint32_t nZeroNum = 0;
	while (nStartBit < nLen * 8)
	{
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余  
		{
			break;
		}
		nZeroNum++;
		nStartBit++;
	}
	nStartBit++;


	//计算结果  
	uint32_t dwRet = 0;
	for (uint32_t i = 0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return (1 << nZeroNum) - 1 + dwRet;
}


int32_t GueeMediaStream::Se(uint8_t *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    uint32_t UeVal = Ue(pBuff, nLen, nStartBit);
	double k = UeVal;
    int nValue = int(ceil(k / 2));//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2 == 0)
		nValue = -nValue;
	return nValue;
}


uint32_t GueeMediaStream::u(uint32_t BitCount, uint8_t * buf, uint32_t &nStartBit)
{
	uint32_t dwRet = 0;
	for (uint32_t i = 0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return dwRet;
}

/**
* H264的NAL起始码防竞争机制
*
* @param buf SPS数据内容
*
* @无返回值
*/
void GueeMediaStream::de_emulation_prevention(uint8_t* buf, uint32_t* buf_size)
{
	uint32_t i = 0, j = 0;
	uint8_t* tmp_ptr = NULL;
	uint32_t tmp_buf_size = 0;
	uint32_t val = 0;

	tmp_ptr = buf;
	tmp_buf_size = *buf_size;
	for (i = 0; i<(tmp_buf_size - 2); i++)
	{
		//check for 0x000003  
		val = (tmp_ptr[i] ^ 0x00) + (tmp_ptr[i + 1] ^ 0x00) + (tmp_ptr[i + 2] ^ 0x03);
		if (val == 0)
		{
			//kick out 0x03  
			for (j = i + 2; j<tmp_buf_size - 1; j++)
				tmp_ptr[j] = tmp_ptr[j + 1];

			//and so we should devrease bufsize  
			(*buf_size)--;
		}
	}
}

/**
* 解码SPS,获取视频图像宽、高和帧率信息
*
* @param buf SPS数据内容
* @param nLen SPS数据的长度
* @param width 图像宽度
* @param height 图像高度

* @成功则返回true , 失败则返回false
*/
void GueeMediaStream::h264_decode_sps(const uint8_t * spsBuf, uint32_t nLen)
{
	uint32_t StartBit = 0;
    const bool debufInfo = false;
    uint8_t*	buf = new uint8_t[nLen];
	memcpy(buf, spsBuf, nLen);
	de_emulation_prevention(buf, &nLen);

    uint32_t forbidden_zero_bit = u(1, buf, StartBit);
    uint32_t nal_ref_idc = u(2, buf, StartBit);
    uint32_t nal_unit_type = u(5, buf, StartBit);
    if (debufInfo)
    {
        qDebug() << "forbidden_zero_bit:" << forbidden_zero_bit;
        qDebug() << "nal_ref_idc:" << nal_ref_idc;
    }
	if (nal_unit_type == 7)
	{
        uint32_t profile_idc = u(8, buf, StartBit);
        uint32_t constraint_set0_flag = u(1, buf, StartBit);//(buf[1] & 0x80)>>7;
        uint32_t constraint_set1_flag = u(1, buf, StartBit);//(buf[1] & 0x40)>>6;
        uint32_t constraint_set2_flag = u(1, buf, StartBit);//(buf[1] & 0x20)>>5;
        uint32_t constraint_set3_flag = u(1, buf, StartBit);//(buf[1] & 0x10)>>4;
        uint32_t reserved_zero_4bits = u(4, buf, StartBit);
        uint32_t level_idc = u(8, buf, StartBit);

		int seq_parameter_set_id = Ue(buf, nLen, StartBit);
        if (debufInfo)
        {
            qDebug() << "constraint_set0_flag:" << constraint_set0_flag;
            qDebug() << "constraint_set1_flag:" << constraint_set1_flag;
            qDebug() << "constraint_set2_flag:" << constraint_set2_flag;
            qDebug() << "constraint_set3_flag:" << constraint_set3_flag;
            qDebug() << "reserved_zero_4bits:" << reserved_zero_4bits;
            qDebug() << "level_idc:" << level_idc;
            qDebug() << "seq_parameter_set_id:" << seq_parameter_set_id;
        }
		if (profile_idc == 100 || profile_idc == 110 ||
			profile_idc == 122 || profile_idc == 144)
		{
            uint32_t chroma_format_idc = Ue(buf, nLen, StartBit);
            uint32_t residual_colour_transform_flag = 0;
			if (chroma_format_idc == 3)
                residual_colour_transform_flag = u(1, buf, StartBit);
            uint32_t bit_depth_luma_minus8 = Ue(buf, nLen, StartBit);
            uint32_t bit_depth_chroma_minus8 = Ue(buf, nLen, StartBit);
            uint32_t qpprime_y_zero_transform_bypass_flag = u(1, buf, StartBit);
            uint32_t seq_scaling_matrix_present_flag = u(1, buf, StartBit);
            uint32_t seq_scaling_list_present_flag[8];
            if (debufInfo)
            {
                qDebug() << "residual_colour_transform_flag:" << residual_colour_transform_flag;
                qDebug() << "bit_depth_luma_minus8:" << bit_depth_luma_minus8;
                qDebug() << "bit_depth_chroma_minus8:" << bit_depth_chroma_minus8;
                qDebug() << "qpprime_y_zero_transform_bypass_flag:" << qpprime_y_zero_transform_bypass_flag;
                qDebug() << "seq_scaling_matrix_present_flag:" << seq_scaling_matrix_present_flag;
                qDebug() << "seq_scaling_list_present_flag:" << seq_scaling_list_present_flag;
            }
			if (seq_scaling_matrix_present_flag)
			{
				for (int i = 0; i < 8; i++) {
					seq_scaling_list_present_flag[i] = u(1, buf, StartBit);
				}
			}
		}
        uint32_t log2_max_frame_num_minus4 = Ue(buf, nLen, StartBit);
        uint32_t pic_order_cnt_type = Ue(buf, nLen, StartBit);
        if (debufInfo)
        {
            qDebug() << "log2_max_frame_num_minus4:" << log2_max_frame_num_minus4;
            qDebug() << "pic_order_cnt_type:" << pic_order_cnt_type;
        }
		if (pic_order_cnt_type == 0)
        {
            uint32_t log2_max_pic_order_cnt_lsb_minus4 = Ue(buf, nLen, StartBit);
            if (debufInfo)
            {
                qDebug() << "log2_max_pic_order_cnt_lsb_minus4:" << log2_max_pic_order_cnt_lsb_minus4;
            }
        }
		else if (pic_order_cnt_type == 1)
		{
            uint32_t delta_pic_order_always_zero_flag = u(1, buf, StartBit);
            int offset_for_non_ref_pic = Se(buf, nLen, StartBit);
            int offset_for_top_to_bottom_field = Se(buf, nLen, StartBit);
            uint32_t num_ref_frames_in_pic_order_cnt_cycle = Ue(buf, nLen, StartBit);

			int *offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
            for (uint32_t i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
				offset_for_ref_frame[i] = Se(buf, nLen, StartBit);

            if (debufInfo)
            {
                qDebug() << "delta_pic_order_always_zero_flag:" << delta_pic_order_always_zero_flag;
                qDebug() << "offset_for_non_ref_pic:" << offset_for_non_ref_pic;
                qDebug() << "offset_for_top_to_bottom_field:" << offset_for_top_to_bottom_field;
                qDebug() << "num_ref_frames_in_pic_order_cnt_cycle:" << num_ref_frames_in_pic_order_cnt_cycle;
            }
			delete[] offset_for_ref_frame;
		}
        uint32_t num_ref_frames = Ue(buf, nLen, StartBit);
        uint32_t gaps_in_frame_num_value_allowed_flag = u(1, buf, StartBit);
        uint32_t spsWidth = Ue(buf, nLen, StartBit);
        uint32_t spsHeight = Ue(buf, nLen, StartBit);
		spsWidth = (spsWidth + 1) * 16;
		spsHeight = (spsHeight + 1) * 16;
        if (debufInfo)
        {
            qDebug() << "num_ref_frames:" << num_ref_frames;
            qDebug() << "gaps_in_frame_num_value_allowed_flag:" << gaps_in_frame_num_value_allowed_flag;
            qDebug() << "spsWidth:" << spsWidth;
            qDebug() << "spsHeight:" << spsHeight;
        }
        uint32_t frame_mbs_only_flag = u(1, buf, StartBit);
		if (!frame_mbs_only_flag)
        {
            uint32_t mb_adaptive_frame_field_flag = u(1, buf, StartBit);
            if (debufInfo)
            {
                qDebug() << "mb_adaptive_frame_field_flag:" << mb_adaptive_frame_field_flag;
            }
        }
        uint32_t direct_8x8_inference_flag = u(1, buf, StartBit);
        uint32_t frame_cropping_flag = u(1, buf, StartBit);
		if (frame_cropping_flag)
		{
            uint32_t frame_crop_left_offset = Ue(buf, nLen, StartBit);
            uint32_t frame_crop_right_offset = Ue(buf, nLen, StartBit);
            uint32_t frame_crop_top_offset = Ue(buf, nLen, StartBit);
            uint32_t frame_crop_bottom_offset = Ue(buf, nLen, StartBit);

			spsWidth -= frame_crop_left_offset * 2;
			spsWidth -= frame_crop_right_offset * 2;
			spsHeight -= frame_crop_top_offset * 2;
			spsHeight -= frame_crop_bottom_offset * 2;
		}
        if (debufInfo)
        {
            qDebug() << "direct_8x8_inference_flag:" << direct_8x8_inference_flag;
            qDebug() << "frame_cropping_flag:" << frame_cropping_flag;
        }
		if (0 >= m_videoParams.width || 0 >= m_videoParams.height)
		{
            m_videoParams.width = int32_t(spsWidth);
            m_videoParams.height = int32_t(spsHeight);
		}
        uint32_t vui_parameter_present_flag = u(1, buf, StartBit);
		if (vui_parameter_present_flag)
		{
            uint32_t aspect_ratio_info_present_flag = u(1, buf, StartBit);
			if (aspect_ratio_info_present_flag)
			{
                uint32_t aspect_ratio_idc = u(8, buf, StartBit);
				if (aspect_ratio_idc == 255)
				{
                    uint32_t sar_width = u(16, buf, StartBit);
                    uint32_t sar_height = u(16, buf, StartBit);
                    if (debufInfo)
                    {
                        qDebug() << "sar_width:" << sar_width;
                        qDebug() << "sar_height:" << sar_height;
                    }
                }
			}
            uint32_t overscan_info_present_flag = u(1, buf, StartBit);
			if (overscan_info_present_flag)
            {
                uint32_t overscan_appropriate_flagu = u(1, buf, StartBit);
                if (debufInfo)
                {
                    qDebug() << "overscan_appropriate_flagu:" << overscan_appropriate_flagu;
                }
            }
            uint32_t video_signal_type_present_flag = u(1, buf, StartBit);
			if (video_signal_type_present_flag)
			{
                uint32_t video_format = u(3, buf, StartBit);
                uint32_t video_full_range_flag = u(1, buf, StartBit);
                uint32_t colour_description_present_flag = u(1, buf, StartBit);
                if (debufInfo)
                {
                    qDebug() << "video_format:" << video_format;
                    qDebug() << "video_full_range_flag:" << video_full_range_flag;
                    qDebug() << "colour_description_present_flag:" << colour_description_present_flag;
                }
				if (colour_description_present_flag)
				{
                    uint32_t colour_primaries = u(8, buf, StartBit);
                    uint32_t transfer_characteristics = u(8, buf, StartBit);
                    uint32_t matrix_coefficients = u(8, buf, StartBit);
                    if (debufInfo)
                    {
                        qDebug() << "colour_primaries:" << colour_primaries;
                        qDebug() << "transfer_characteristics:" << transfer_characteristics;
                        qDebug() << "matrix_coefficients:" << matrix_coefficients;
                    }
                }
			}
            uint32_t chroma_loc_info_present_flag = u(1, buf, StartBit);
			if (chroma_loc_info_present_flag)
			{
                uint32_t chroma_sample_loc_type_top_field = Ue(buf, nLen, StartBit);
                uint32_t chroma_sample_loc_type_bottom_field = Ue(buf, nLen, StartBit);
                if (debufInfo)
                {
                    qDebug() << "chroma_sample_loc_type_top_field:" << chroma_sample_loc_type_top_field;
                    qDebug() << "chroma_sample_loc_type_bottom_field:" << chroma_sample_loc_type_bottom_field;
                }
            }
            uint32_t timing_info_present_flag = u(1, buf, StartBit);

			if (timing_info_present_flag)
			{
                uint32_t num_units_in_tick = u(32, buf, StartBit);
                uint32_t time_scale = u(32, buf, StartBit);
				//m_spsFrameRate = float_t(time_scale) / float_t(num_units_in_tick) / 2.0f;
                uint32_t fixed_frame_rate_flag = u(1, buf, StartBit);
                if (0 >= m_videoParams.frameRate)
				{
                    m_videoParams.frameRate = float( time_scale) / float(num_units_in_tick * 2);
				}
                if (debufInfo)
                {
                    qDebug() << "num_units_in_tick:" << num_units_in_tick;
                    qDebug() << "time_scale:" << time_scale;
                    qDebug() << "fixed_frame_rate_flag:" << fixed_frame_rate_flag;
                }

			}
            else if (0 >= m_videoParams.frameRate)
			{
                m_videoParams.frameRate = 30.0f;
			}
		}
        else if (0 >= m_videoParams.frameRate)
        {
            m_videoParams.frameRate = 30.0f;
        }
    }
    delete []buf;
}

void GueeMediaStream::parseADTS(const uint8_t* data)
{
    SADTS	adts;
    memset(&adts, 0, sizeof(adts));
	adts.syncword = data[0] << 4 | data[1] >> 4;
	adts.ID = (data[1] >> 3) & 1;
	adts.layer = (data[1] >> 1) & 3;
	adts.protection_absent = data[1] & 1;

	adts.profile = data[2] >> 6;
	adts.sampling_frequency_index = (data[2] >> 2) & 0xF;
	adts.private_bit = (data[2] >> 1) & 1;
	adts.channel_configuration = (data[2] & 1) << 2 | data[3] >> 6;
	adts.original_copy = (data[3] >> 5) & 1;
	adts.home = (data[3] >> 4) & 1;

	adts.copyright_identiflcation_bit = (data[3] >> 3) & 1;
	adts.copyright_identiflcation_start = (data[3] >> 2) & 1;

	adts.aac_frame_length = (data[3] & 3) << 11 | data[4] << 3 | data[5] >> 5;
	adts.adts_buffer_fullness = (data[5] & 0x1F) << 6 | data[6] >> 2;
	adts.num_of_raw_data_blocks_in_frame = data[6] & 3;

	if (adts.sampling_frequency_index > 12) adts.sampling_frequency_index = 7;
	if (adts.channel_configuration == 0) adts.channel_configuration = 1;
	m_audioParams.useADTS = true;
	//m_audioParams.eCodec = adts.ID ? AC_MP2AAC : AC_AAC;
	m_audioParams.eCodec = AC_AAC;
	m_audioParams.encLevel = adts.profile + 1;
    m_audioParams.sampleRate = AAC_Sampling_Frequency_Table[adts.sampling_frequency_index];
	m_audioParams.channels = adts.channel_configuration;
    m_audioParams.sampleBits = eSampleBit16i;
	m_aduioFrameSamples = 1024;
	makeAAC_SequenceHeader();
}

void GueeMediaStream::parseAAC_SequenceHeader(const uint8_t * data)
{
	uint16_t spec = (data[0] << 8) | data[1];
	uint16_t samp = (spec >> 7) & 0x0f;
	m_audioParams.channels = (spec >> 3) & 0x0f;
	m_audioParams.encLevel = spec >> 11;
    m_audioParams.sampleRate = AAC_Sampling_Frequency_Table[samp];
	m_aduioFrameSamples = 1024;
	makeAAC_SequenceHeader();
}

void GueeMediaStream::makeAAC_SequenceHeader()
{
	int32_t	sampIndex = 0;
	int32_t	minSub = INT_MAX;
	for (int32_t i = 0; i < 16; ++i)
	{
        if (abs(AAC_Sampling_Frequency_Table[i] - int32_t(m_audioParams.sampleRate)) < minSub)
		{
            minSub = abs(AAC_Sampling_Frequency_Table[i] - int32_t(m_audioParams.sampleRate));
			sampIndex = i;
		}
	}
	//4. AAC音频包 https://blog.csdn.net/huibailingyu/article/details/50554151
	//	结构为：0x08, 3字节包长度，4字节时间戳，00 00 00，AF 01 N字节AAC数据 | 前包长度
	//	其中编码后AAC纯数据长度为N，3字节包长度 = N + 2
	//	前包长度 = 11 + 3字节包长度 = 11 + N + 2 = 13 + N。
	uint16_t	specificConfig = m_audioParams.encLevel << 11 | sampIndex << 7 | m_audioParams.channels << 3;
	m_audioSpecificConfig[0] = (specificConfig >> 8);
	m_audioSpecificConfig[1] = (specificConfig & 0xFF);
	m_audioSpecificConfigSize = 2;
}
