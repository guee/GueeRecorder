#include "MediaWriterTs.h"



CMediaWriterTs::CMediaWriterTs(CMediaStream& stream)
	: CMediaWriter(stream)
{
}


CMediaWriterTs::~CMediaWriterTs()
{

}

void CMediaWriterTs::writeBits(uint8_t bits, uint32_t data )
{
	int32_t i = m_startBit / 8;
	uint8_t left = m_startBit - i * 8;
	m_startBit += bits;
	while (bits)
	{
		uint8_t w = min( uint8_t(8 - left), bits);
		data &= 0xFFFFFFFF >> (32 - bits);
		m_pack[i] &= ~((0xFF >> (8 - w)) << (8 - left - w));
		m_pack[i] |= (data >> (bits - w)) << (8 - left - w);
		bits -= w;
		++i;
		left = 0;
	}
}

bool CMediaWriterTs::onWriteHeader()
{
	const SVideoParams&	videoParams = m_stream.videoParams();
	const SAudioParams&	audioParams = m_stream.audioParams();
	bool	hasVideo = m_stream.hasVideo() && videoParams.enabled;
	bool	hasAudio = m_stream.hasAudio() && audioParams.enabled;
	unsigned char data[188] = {
	0x47, 0x40, 0x11, 0x10, 0x00, 0x42, 0xF0, 0x25, 0x00, 0x01, 0xC1, 0x00, 0x00, 0xFF, 0x01, 0xFF,
	0x00, 0x01, 0xFC, 0x80, 0x14, 0x48, 0x12, 0x01, 0x06, 0x46, 0x46, 0x6D, 0x70, 0x65, 0x67, 0x09,
	0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x30, 0x31, 0x77, 0x7C, 0x43, 0xCA, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	appendData(data, sizeof(data));
	flushData();

	m_patProgs.clear();
	m_patProgs[1] = 0x1000;
	if (hasVideo)
	{
		TS_Stream	st;
		st.programNumber = 1;
		st.stream_type = 0x1B;
		st.elementary_PID = 0x100;
		m_pmtStreams.push_back(st);
	}
	if (hasAudio)
	{
		TS_Stream	st;
		st.programNumber = 1;
		st.stream_type = 0x0F;
		st.elementary_PID = 0x101;
		st.ES_info.resize(6);
		st.ES_info[0] = 0x0A;
		st.ES_info[1] = 0x04;
		st.ES_info[2] = 0x75;
		st.ES_info[3] = 0x6E;
		st.ES_info[4] = 0x64;
		st.ES_info[5] = 0;
		m_pmtStreams.push_back(st);
	}
	makeTsPat();
	makeTsPmt(1);
	m_audioTime = -1;
	return true;
}

bool CMediaWriterTs::onWriteVideo(const CMediaStream::H264Frame * frame)
{
	const SVideoParams&	videoParams = m_stream.videoParams();

	m_videoCache.clear();
	if (videoParams.annexb)
	{
		for (int32_t i = 0; i < frame->nalCount; ++i)
		{
			const CMediaStream::H264Frame::NAL& nal = frame->nals[i];
			//if ( nal.nalType == NalAud || nal.nalType == NalSps || nal.nalType == NalSei || nal.nalType == NalPps) continue;
			m_videoCache.append((const char*)frame->nals[i].nalData, frame->nals[i].nalSize);
		}
	}
	else
	{
		for (int32_t i = 0; i < frame->nalCount; ++i)
		{
			const CMediaStream::H264Frame::NAL& nal = frame->nals[i];
			//if (nal.nalType == NalAud || nal.nalType == NalSps || nal.nalType == NalSei || nal.nalType == NalPps) continue;
			m_videoCache.append("\0\0\1", 3);
			m_videoCache.append((const char*)frame->nals[i].nalData + 4, frame->nals[i].nalSize - 4);
		}
	}
	int32_t frameSize = m_videoCache.length();
	const char*	framePtr = m_videoCache.data();

	setTsPacketHeader(0x100, true, 0xFF, frame->dtsTimeMS);
	writeBits(24, 1);	//packet_start_code_prefix
	writeBits(8, 0xe0);	//stream_id	视频取值（0xe0-0xef），通常为0xe0

	writeBits(16, frameSize < 65536 ? frameSize : 0);	//PES_packet_length
	writeBits(2, 2);	//fix_bit
	writeBits(2, 0);	//PES_scrambling_control
	writeBits(1, 0);	//PES_priority
	writeBits(1, 0);	//data_alignment_indicator
	writeBits(1, 0);	//copyright
	writeBits(1, 0);	//original_or_copy

	writeBits(2, 3);	//PTS_DTS_flags
	writeBits(1, 0);	//ESCR_flag
	writeBits(1, 0);	//ES_rate_flag
	writeBits(1, 0);	//DSM_trick_mode_flag
	writeBits(1, 0);	//additional_copy_info_flag
	writeBits(1, 0);	//PES_CRC_flag
	writeBits(1, 0);	//PES_extension_flag

	writeBits(8, 10);	//PES_header_data_length
	uint64_t base = frame->ptsTimeMS * 90;
	writeBits(4, 3);	//PTS_DTS_flags
	writeBits(3, base >> 30);	//pts
	m_startBit += 1;
	writeBits(15, base >> 15);	//pts
	m_startBit += 1;
	writeBits(15, base);	//pts
	m_startBit += 1;
	base = frame->dtsTimeMS * 90;
	writeBits(4, 3);	//PTS_DTS_flags
	writeBits(3, base >> 30);	//dts
	m_startBit += 1;
	writeBits(15, base >> 15);	//dts
	m_startBit += 1;
	writeBits(15, base);	//dts
	m_startBit += 1;

	while (frameSize)
	{
		int32_t writeSize = min((int32_t)sizeof(m_pack) - m_startBit / 8, frameSize);
		memcpy(m_pack + m_startBit / 8, framePtr, writeSize);
		appendData(m_pack, sizeof(m_pack));
		flushData();
		frameSize -= writeSize;
		framePtr += writeSize;
		if (frameSize)
		{
			setTsPacketHeader(0x100, false, 0xFF);
		}
	}
	return true;
}

bool CMediaWriterTs::onWriteAudio(const CMediaStream::AUDFrame * frame)
{
	if (m_audioTime < 0)
	{
		m_audioTime = frame->timestamp;
	}
	m_audioCache.append((const char*)frame->data, frame->size);
	if (frame->timestamp - m_audioTime < 300) return true;

	setTsPacketHeader(0x101, true, 0xFF);
	int32_t frameSize = m_audioCache.length();
	const char*	framePtr = m_audioCache.data();

	writeBits(24, 1);	//packet_start_code_prefix
	writeBits(8, 0xc0);	//stream_id	音频取值（0xc0-0xdf），通常为0xc0

	writeBits(16, frameSize < 65536 ? frameSize : 0);	//PES_packet_length
	writeBits(2, 2);	//fix_bit
	writeBits(2, 0);	//PES_scrambling_control
	writeBits(1, 0);	//PES_priority
	writeBits(1, 0);	//data_alignment_indicator
	writeBits(1, 0);	//copyright
	writeBits(1, 0);	//original_or_copy

	writeBits(2, 2);	//PTS_DTS_flags
	writeBits(1, 0);	//ESCR_flag
	writeBits(1, 0);	//ES_rate_flag
	writeBits(1, 0);	//DSM_trick_mode_flag
	writeBits(1, 0);	//additional_copy_info_flag
	writeBits(1, 0);	//PES_CRC_flag
	writeBits(1, 0);	//PES_extension_flag

	writeBits(8, 5);	//PES_header_data_length

	uint64_t base = m_audioTime * 90;
	writeBits(4, 2);	//PTS_DTS_flags
	writeBits(3, base >> 30);	//pts
	m_startBit += 1;
	writeBits(15, base >> 15);	//pts
	m_startBit += 1;
	writeBits(15, base);	//pts
	m_startBit += 1;

	while (frameSize)
	{
		int32_t writeSize = min((int32_t)sizeof(m_pack) - m_startBit / 8, frameSize);
		memcpy(m_pack + m_startBit / 8, framePtr, writeSize);
		appendData(m_pack, sizeof(m_pack));
		flushData();
		frameSize -= writeSize;
		framePtr += writeSize;
		if (frameSize)
		{
			setTsPacketHeader(0x101, false, 0xFF);
		}
	}
	m_audioCache.clear();
	m_audioTime = -1;
	return true;
}

void CMediaWriterTs::setTsPacketHeader(uint16_t pid, bool isStart, uint8_t startBytes, int64_t pcr)
{
	memset(m_pack, 0xFF, sizeof(m_pack));
	m_startBit = 0;
	//TS_Packet_Header
	writeBits(8, 0x47);	//sync_byte
	writeBits(1, 0);	//transport_error_indicator
	writeBits(1, isStart ? 1 : 0);	//payload_unit_start_indicator
	writeBits(1, 0);	//transport_priority
	writeBits(13, pid);	//PID
	writeBits(2, 0);	//transport_scrambling_control
	writeBits(2, 1 | (pcr >= 0 ? 2 : 0));	//adaptation_field_control
	writeBits(4, m_pidCounter[pid]);	//continuity_counter
	m_pidCounter[pid] = (m_pidCounter[pid] + 1) % 16;
	if (pcr>=0)
	{
		writeBits(8, 7);
		int32_t wb = 0;

		writeBits(8, 0x50);

		uint64_t base = (pcr * 90) % 0x200000000;
		uint32_t ext = (pcr * 27000) % 300;
		writeBits(32, base >> 1);
		writeBits(1, base & 1);
		m_startBit += 6;
		writeBits(9, ext);
	
	}
	if (startBytes != 0xFF)
	{
		writeBits(8, startBytes);
		m_startBit += startBytes * 8;
	}
}

void CMediaWriterTs::makeTsPat()
{
	setTsPacketHeader(0);
	int32_t patStart = m_startBit;
	//TS_PAT
	writeBits(8, 0);	//table_id
	writeBits(1, 1);	//section_syntax_indicator
	writeBits(1, 0);	//zero
	m_startBit += 2;	//reserved_1
	writeBits(12, 5 + 4 + (uint32_t)m_patProgs.size() * 4);	//section_length
	writeBits(16, 1);	//transport_stream_id
	m_startBit += 2;	//reserved_2
	writeBits(5, 0);	//version_number
	writeBits(1, 1);	//current_next_indicator
	writeBits(8, 0);	//section_number
	writeBits(8, 0);	//last_section_number
	for (auto i = m_patProgs.begin(); i != m_patProgs.end(); ++i)
	{
		//TS_PAT_PROGRAM
		writeBits(16, i->first);	//program_number
		m_startBit += 3;			//reserved_3
		writeBits(13, i->second);	//program_map_PID
	}
	writeBits(32, crc32(m_pack + patStart / 8, (m_startBit - patStart) / 8));	//CRC_32

	appendData(m_pack, sizeof(m_pack));
	flushData();
}

void CMediaWriterTs::makeTsPmt(uint16_t programNumber)
{
	int32_t sectLength = 0;
	int32_t pcrPID = 0;
	for (auto i = m_pmtStreams.begin(); i != m_pmtStreams.end(); ++i)
	{
		if (i->programNumber == programNumber)
		{
			sectLength += 5 + i->ES_info.size();
			if (0 == pcrPID) pcrPID = i->elementary_PID;
		}
	}
	setTsPacketHeader(m_patProgs[programNumber]);
	int32_t pmtStart = m_startBit;
	writeBits(8, 2);	//table_id
	writeBits(1, 1);	//section_syntax_indicator
	writeBits(1, 0);	//zero
	m_startBit += 2;	//reserved_1
	writeBits(12, 9 + 4 + sectLength);	//section_length
	writeBits(16, programNumber);	//program_number
	m_startBit += 2;	//reserved_2
	writeBits(5, 0);	//version_number
	writeBits(1, 1);	//current_next_indicator
	writeBits(8, 0);	//section_number
	writeBits(8, 0);	//last_section_number
	m_startBit += 3;	//reserved_3
	writeBits(13, pcrPID);	//PCR_PID
	m_startBit += 4;	//reserved_4
	writeBits(12, 0);	//program_info_length
	for (auto i = m_pmtStreams.begin(); i != m_pmtStreams.end(); ++i)
	{
		if (i->programNumber == programNumber)
		{
			//TS_PAT_PROGRAM
			writeBits(8, i->stream_type);	//stream_type
			m_startBit += 3;			//reserved_5
			writeBits(13, i->elementary_PID);	//elementary_PID
			m_startBit += 4;			//reserved_6
			writeBits(12, i->ES_info.size());	//ES_info_length
			memcpy(m_pack + m_startBit / 8, i->ES_info.data(), i->ES_info.size());
			m_startBit += i->ES_info.size() * 8;
		}
	}
	writeBits(32, crc32(m_pack + pmtStart / 8, (m_startBit - pmtStart) / 8));	//CRC_32
	appendData(m_pack, sizeof(m_pack));
	flushData();
}

uint32_t CMediaWriterTs::crc32(const uint8_t* data, uint32_t size)
{
	static	uint32_t		crc32Table[256];
	static	bool			created = false;

	if (!created)
	{

		for (uint32_t i = 0; i < 256; i++) {
			uint32_t k = 0;
			for (uint32_t j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1) {
				k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);
			}
			crc32Table[i] = k;
		}
		created = true;
	}

	uint32_t	crc32 = 0xffffffff;
	while (size--)
	{
		crc32 = (crc32 << 8) ^ crc32Table[((crc32 >> 24) ^ *data++) & 0xFF];
	}
	return crc32;
}
