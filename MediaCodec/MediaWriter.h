#pragma once
#include "MediaStream.h"

class GueeMediaWriter
{
public:
    GueeMediaWriter(GueeMediaStream& stream);
    virtual ~GueeMediaWriter(void);

    bool setFilePath( const string& szPath );
	void setEnable( bool enable = true );
protected:
    friend GueeMediaStream;
    GueeMediaStream&	m_stream;
    typedef GueeMediaStream::H264Frame H264Frame;
    typedef GueeMediaStream::AUDFrame AUDFrame;
	ofstream		m_fileWrite;
    string			m_filePath;
    bool			m_isOpened;
	bool			m_isEnabled;
	bool			m_isAdtsAAC;

	virtual bool onWriteHeader();
	virtual bool onWriteVideo(const H264Frame * frame);
	virtual bool onWriteAudio(const AUDFrame * frame);
	virtual	bool onOpenWrite();
	virtual	void onCloseWrite();

	string		m_buffer;
	int64_t		m_totalBytes;
	//uint8_t*		m_cacheBuffer;		//缓存 
	//uint32_t		m_cacheOffset;		//当前已经写入的缓存字节数
	//uint32_t		m_cacheAllocate;	//当前缓存分配的字节数
	//uint64_t		m_totalBytes;		//累计写入的字节数
	inline bool appendData( const uint8_t* data, uint32_t size )
	{
		m_buffer.append((const char*)data, size);
		return true;
	}
	inline uint32_t cacheSize()
	{
		return uint32_t(m_buffer.length());
	}
	virtual bool flushData()
	{
		//将音视频数据存入视频文件中
		if ( !m_buffer.empty() )
		{
			m_fileWrite.write(m_buffer.c_str(), m_buffer.length());
			if ( m_fileWrite.bad() ) return false;
			m_totalBytes	+= m_buffer.length();
			m_buffer.clear();
		}
		return true;
	}
	inline void putByte( uint8_t b ) { appendData( &b, 1 ); }
	inline void putTag( const char* tag ) { while ( *tag ) appendData( (uint8_t*)tag++, 1 ); }
	inline void putBE16( uint16_t val ) { putByte( val >> 8 ); putByte( val & 0xFF ); }
	inline void putBE24( uint32_t val ) { putBE16( ( val >> 8 ) & 0xFFFF ); putByte( val & 0xFF ); }
	inline void putBE32( uint32_t val ) {
		putByte( ( val >> 24 ) & 0xFF );
		putByte( ( val >> 16 ) & 0xFF );
		putByte( ( val >> 8 ) & 0xFF );
		putByte( ( val ) & 0xFF );
	}
	inline void putBE64( uint64_t val ) { putBE32( val >> 32 ); putBE32( val & 0xFFFFFFFF ); }
	inline void putLE16( uint16_t val ) { appendData( (uint8_t*)&val, 2 ); }
	inline void putLE24( uint32_t val ) { appendData( (uint8_t*)&val, 3 ); }
	inline void putLE32( uint32_t val ) { appendData( (uint8_t*)&val, 4 ); }
	inline void putLE64( uint64_t val ) { appendData( (uint8_t*)&val, 8 ); }

    inline  uint64_t dbl2int( double value )
    {
        uint64_t* ptr = reinterpret_cast<uint64_t*>(&value);
        return *ptr;
	}
	inline  uint16_t endianFix16( uint16_t x )
	{
		return ( x << 8 ) + ( x >> 8 );
	}
	inline  uint32_t endianFix32( uint32_t x )
	{
		return ( x << 24 ) + ( ( x << 8 ) & 0xff0000 ) + ( ( x >> 8 ) & 0xff00 ) + ( x >> 24 );
	}
	inline  uint64_t endianFix64( uint64_t x )
	{
		return endianFix32( x >> 32 ) + ( (uint64_t)endianFix32( x & 0xFFFFFFFF ) << 32 );
	}


	bool reputAmfDoubleToFile( int32_t positin, double value )
	{
		if ( !m_isOpened ) return false;
		uint64_t x = endianFix64( dbl2int( value ) );
		streampos	oldPos	= m_fileWrite.tellp();
		m_fileWrite.seekp( positin, ios::beg );
		m_fileWrite.write( (char*)&x, 8 );
		m_fileWrite.seekp( oldPos, ios::beg );
		return m_fileWrite.good();
	}

	bool reputAviDwordToFile( int32_t positin, uint32_t value )
	{
		if ( !m_isOpened ) return false;
		streampos	oldPos	= m_fileWrite.tellp();
		m_fileWrite.seekp( positin, ios::beg );
		m_fileWrite.write((char*)&value, 4 );
		m_fileWrite.seekp( oldPos, ios::beg );
		return m_fileWrite.good();
	}

};
