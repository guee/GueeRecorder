#pragma once
#include "MediaWriter.h"

class GueeMediaWriterFlv: public GueeMediaWriter
{
public:
    GueeMediaWriterFlv(GueeMediaStream& stream);
    virtual ~GueeMediaWriterFlv(void);

protected:
	virtual bool onWriteHeader();
    virtual bool onWriteVideo(const GueeMediaStream::H264Frame * frame);
    virtual bool onWriteAudio(const GueeMediaStream::AUDFrame * frame);
	virtual	void onCloseWrite();

	/* offsets for packed values */
	#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
	#define FLV_AUDIO_SAMPLERATE_OFFSET  2
	#define FLV_AUDIO_CODECID_OFFSET     4

	#define FLV_VIDEO_FRAMETYPE_OFFSET   4

	/* bitmasks to isolate specific values */
	#define FLV_AUDIO_CHANNEL_MASK    0x01
	#define FLV_AUDIO_SAMPLESIZE_MASK 0x02
	#define FLV_AUDIO_SAMPLERATE_MASK 0x0c
	#define FLV_AUDIO_CODECID_MASK    0xf0

	#define FLV_VIDEO_CODECID_MASK    0x0f
	#define FLV_VIDEO_FRAMETYPE_MASK  0xf0

	enum
	{
		FLV_HEADER_FLAG_HASVIDEO = 1,
		FLV_HEADER_FLAG_HASAUDIO = 4,
	};
	enum
	{
		FLV_TAG_TYPE_AUDIO = 0x08,
		FLV_TAG_TYPE_VIDEO = 0x09,
		FLV_TAG_TYPE_META  = 0x12,
	};
	enum
	{
		FLV_MONO   = 0,
		FLV_STEREO = 1,
	};

	enum
	{
		FLV_SAMPLESSIZE_8BIT  = 0,
		FLV_SAMPLESSIZE_16BIT = 1 << FLV_AUDIO_SAMPLESSIZE_OFFSET,
	};

	enum
	{
		FLV_SAMPLERATE_SPECIAL = 0, /**< signifies 5512Hz and 8000Hz in the case of NELLYMOSER */
		FLV_SAMPLERATE_11025HZ = 1 << FLV_AUDIO_SAMPLERATE_OFFSET,
		FLV_SAMPLERATE_22050HZ = 2 << FLV_AUDIO_SAMPLERATE_OFFSET,
		FLV_SAMPLERATE_44100HZ = 3 << FLV_AUDIO_SAMPLERATE_OFFSET,
	};
	
	enum FlvCodecId
	{
		FLV_CODECID_H264 = 7,

		FLV_CODECID_PEPCM = 0 << FLV_AUDIO_CODECID_OFFSET,		//0 = Linear PCM, platform endian
		FLV_CODECID_ADPCM = 1 << FLV_AUDIO_CODECID_OFFSET,		//1 = ADPCM
		FLV_CODECID_MP3 = 2 << FLV_AUDIO_CODECID_OFFSET,		//2 = MP3
		FLV_CODECID_LEPCM = 3 << FLV_AUDIO_CODECID_OFFSET,		//3 = Linear PCM, little endian
		FLV_CODECID_Ne16K = 4 << FLV_AUDIO_CODECID_OFFSET,		//4 = Nellymoser 16 kHz mono
		FLV_CODECID_Ne8K = 5 << FLV_AUDIO_CODECID_OFFSET,		//5 = Nellymoser 8 kHz mono
		FLV_CODECID_Nelly = 6 << FLV_AUDIO_CODECID_OFFSET,		//6 = Nellymoser
		FLV_CODECID_G711A = 7 << FLV_AUDIO_CODECID_OFFSET,		//7 = G.711 A-law logarithmic PCM
		FLV_CODECID_G711MU = 8 << FLV_AUDIO_CODECID_OFFSET,		//8 = G.711 mu-law logarithmic PCM
		FLV_CODECID_RESERVED = 9 << FLV_AUDIO_CODECID_OFFSET,	//9 = reserved
		FLV_CODECID_AAC = 10 << FLV_AUDIO_CODECID_OFFSET,		//10 = AAC
		FLV_CODECID_SPEEX = 11 << FLV_AUDIO_CODECID_OFFSET,		//11 = Speex
		FLV_CODECID_MP3_8K = 14 << FLV_AUDIO_CODECID_OFFSET,	//14 = MP3 8 kHz
		FLV_CODECID_DEV_SPEC = 15 << FLV_AUDIO_CODECID_OFFSET	//15 = Device-specific sound
	};


	enum
	{
		FLV_FRAME_KEY   = ( 1 << FLV_VIDEO_FRAMETYPE_OFFSET ) | FLV_CODECID_H264,
		FLV_FRAME_INTER = ( 2 << FLV_VIDEO_FRAMETYPE_OFFSET ) | FLV_CODECID_H264,
	};

	typedef enum
	{
		AMF_DATA_TYPE_NUMBER      = 0x00,
		AMF_DATA_TYPE_BOOL        = 0x01,
		AMF_DATA_TYPE_STRING      = 0x02,
		AMF_DATA_TYPE_OBJECT      = 0x03,
		AMF_DATA_TYPE_NULL        = 0x05,
		AMF_DATA_TYPE_UNDEFINED   = 0x06,
		AMF_DATA_TYPE_REFERENCE   = 0x07,
		AMF_DATA_TYPE_MIXEDARRAY  = 0x08,
		AMF_DATA_TYPE_OBJECT_END  = 0x09,
		AMF_DATA_TYPE_ARRAY       = 0x0a,
		AMF_DATA_TYPE_DATE        = 0x0b,
		AMF_DATA_TYPE_LONG_STRING = 0x0c,
		AMF_DATA_TYPE_UNSUPPORTED = 0x0d,
	} AMFDataType;

	void putAmfString( const char *str )	{
		uint16_t	len	= (uint16_t)strlen( str );
		putBE16( len );
		appendData( (uint8_t*)str, len );
	}
	void putAmfDouble( double d )	{
		putByte( AMF_DATA_TYPE_NUMBER );
		putBE64( dbl2int( d ) );
	}
	void putAmfBool(bool b) {
		putByte(AMF_DATA_TYPE_BOOL);
		putByte(b ? 1 : 0);
	}
	void reputAmfBE24Length( uint32_t length, uint32_t offset )	{
		m_buffer[offset]	= ( length >> 16 ) & 0xFF;
		m_buffer[offset + 1] = ( length >> 8 ) & 0xFF;
		m_buffer[offset + 2] = ( length >> 0 ) & 0xFF;
	}


	uint32_t	m_writeFramerateOffset;	//如果是可变帧率，那么就要在结束时计算帧率并写入，因此暂存文件中写入帧率值的位置。
	uint32_t	m_writeDurationOffset;	//暂存文件中写入视频时长值的位置。
	uint32_t	m_writeFileSizeOffset;	//暂存文件中写入文件长度值的位置。
	uint32_t	m_writeVideoRateOffset;	//暂存文件中写入视频码率值的位置。
	uint32_t	m_videoDataTotalSize;
	uint8_t		m_aacTagHeader;
};
