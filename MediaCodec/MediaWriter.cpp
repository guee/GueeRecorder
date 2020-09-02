#include "MediaWriter.h"

CMediaWriter::CMediaWriter(CMediaStream& stream)
	: m_stream(stream)
{
	m_isOpened = false;
	m_isEnabled = true;
	m_isAdtsAAC = false;
	m_totalBytes = 0;

	m_stream.appendWriter(this);
}

CMediaWriter::~CMediaWriter(void)
{
	onCloseWrite();
	m_stream.removeWriter(this);
}

bool CMediaWriter::setFilePath( const string& szPath )
{
	if ( m_isOpened || szPath.empty() ) return false;
	m_filePath	= szPath;
	return true;
}

void CMediaWriter::setEnable(bool enable)
{
	m_isEnabled = enable;
}

bool CMediaWriter::onWriteHeader()
{
	putBE32( 1 );
	if ( !appendData((unsigned char*)&m_stream.m_sps.front(), (int32_t)m_stream.m_sps.size() ) ) return false;
	putBE32( 1 );
	if ( !appendData((unsigned char*)&m_stream.m_pps.front(), (int32_t)m_stream.m_pps.size() ) ) return false;
	putBE24( 1 );
	if ( !appendData((unsigned char*)&m_stream.m_sei.front(), (int32_t)m_stream.m_sei.size() ) ) return false;

	return flushData();
}

bool CMediaWriter::onWriteVideo(const H264Frame * frame)
{
	if (m_stream.videoParams().annexb )
	{
		for ( int32_t i = 0; i < frame->nalCount; ++i )
		{
			if ( !appendData(frame->nals[i].nalData, frame->nals[i].nalSize) ) return false;
		}
	}
	else
	{
		for ( int32_t i = 0; i < frame->nalCount; ++i )
		{
			i == 0 ? putBE32( 1 ) : putBE24( 1 );
			if (!appendData(frame->nals[i].nalData + 4, frame->nals[i].nalSize - 4)) return false;
		}
	}
	return flushData();
}

bool CMediaWriter::onWriteAudio(const AUDFrame * frame)
{
	if ( !appendData(frame->data, frame->size) ) return false;
	return flushData();
}

bool CMediaWriter::onOpenWrite()
{
	if ( m_isOpened ) return false;
//	if (m_isEnabled)
	m_fileWrite.open(m_filePath, ios::out | ios::binary);
	m_isOpened = m_fileWrite.is_open();
	m_totalBytes = 0;
	return m_isOpened;
}

void CMediaWriter::onCloseWrite()
{
	if ( !m_isOpened ) return;
	if ( m_fileWrite.is_open() )
	{
		flushData();
		m_fileWrite.close();
	}
	m_isOpened	= false;
}
