#pragma once
#include <stdint.h>
#include <stdio.h> 
#include <malloc.h>
#include <memory.h>

#pragma pack (2) /*指定按2字节对齐*/
struct WAVEHEADER
{
    uint32_t   uRiff;			// "RIFF"
	uint32_t   uSize;			// Size
	uint32_t   uWave;			// "WAVE"
};

struct DATA_BLOCK
{
	uint32_t	uDataID;		// 'd','a','t','a'
	uint32_t	uDataSize;
};
struct SWaveFormat
{
	uint16_t	wFormatTag;         /* format type */
	uint16_t	nChannels;          /* number of channels (i.e. mono, stereo...) */
	uint32_t	nSamplesPerSec;     /* sample rate */
	uint32_t	nAvgBytesPerSec;    /* for buffer estimation */
	uint16_t	nBlockAlign;        /* block size of data */
	uint16_t	wBitsPerSample;     /* number of bits per sample of mono data */
	uint16_t	cbSize;             /* the count in bytes of the size of */
									/* extra information (after cbSize) */
};
#pragma pack () /*取消指定对齐，恢复缺省对齐*/

class CWaveFile
{
public:
	CWaveFile() {}
	~CWaveFile() { closeFile(); }

	//打开文件，用于读
    bool openFile( const char* szFileName );
	//打开文件，用于写。
	//pWfxInfo 结构指定 PCM 数据的格式。如果参数为 NULL，则表示追加，格式信息从文件中读入。
    bool openFile( const char* szFileName, const SWaveFormat* pWfxInfo );
	void closeFile();

	uint32_t readSamples( uint32_t uCount, void* pData );
	bool appendSamples( uint32_t uCount, const void* pData );

	const SWaveFormat* getWaveFormat() { return m_pWfxInfo; }
	bool seekToSecond( double dSecond );
	bool seekToSample( uint32_t uSample );

	double currentSecond();
	uint32_t currentSample();

	uint32_t byteCountAsSamples() { return m_pWfxInfo ? m_pWfxInfo->nBlockAlign : 0; }
	uint32_t samplesPerSecond() { return m_pWfxInfo ? m_pWfxInfo->nSamplesPerSec : 0; }
	uint32_t sampleCount() { return m_pWfxInfo ? m_uWaveSize / m_pWfxInfo->nBlockAlign : 0; }
	uint16_t channelCount() { return m_pWfxInfo ? m_pWfxInfo->nChannels : 0; }

	enum ESampleBits
	{
		eSampleBit8i,
		eSampleBit16i,
		eSampleBit24i,
		eSampleBit32i,
		eSampleBit24In32i,
		eSampleBit32f,
	}; 
	ESampleBits sampleBits();
private:
	FILE*			m_hFile			= nullptr;
	SWaveFormat*	m_pWfxInfo		= nullptr;
	uint32_t		m_uWaveOffset	= 0;
	uint32_t		m_uWaveSize		= 0;
	uint32_t		m_uReadOffset	= 0;
	bool			m_bIsWrite		= false;
    static uint32_t MakeFourCC( uint8_t ch0, uint8_t ch1, uint8_t ch2, uint8_t ch3 )
	{
		return ( (uint32_t)ch0 | ( (uint32_t)ch1 << 8 ) | ( (uint32_t)ch2 << 16 ) | ( (uint32_t)ch3 << 24 ) );
	}
	bool updateHead();
};
