#pragma once
#include "H264Codec.h"
#include <QtCore>

class IMediaWrite
{
public:
    virtual ~IMediaWrite();
    virtual	bool setFilePath( const QString& path )	= 0;
};

class IVideoEncoder
{
public:
    virtual ~IVideoEncoder();
	//如果 mediaWrite 为 NULL，则清除所有的写入对象。
	virtual	bool bindWrite( IMediaWrite* mediaWrite )	= 0;
	virtual	bool startEncode( const SVideoParams* videoParams )	= 0;
	virtual	void endEncode()	= 0;
	virtual	const SVideoParams* getParams()	= 0;
	virtual	bool putFrame( int64_t millisecond, EVideoCSP inpCSP, const uint8_t* buf, int32_t pitch, int32_t width = 0, int32_t height = 0 )	= 0;
};

class IAudioEncoder
{
public:
    virtual ~IAudioEncoder();
	virtual	bool bindWrite( IMediaWrite* mediaWrite )	= 0;
	virtual	bool startEncode( const SAudioParams* videoParams )	= 0;
	virtual	void endEncode()	= 0;
	virtual	const SAudioParams* getParams()	= 0;
	//设置输入的音频数据格式。
	//当输入和输出的音频格式不一样时，或者需要把多个输入合并成一个输出时，才需要调用。
	//参数：inputIndex			设置指定通道的输入音频的格式。从 0 开始小于 8 的值。
	//		其它参数		和 BeginResample 一样。
	//备注：初始时只有 0 号输入通道可以使用，且格式与 startEncode 设置的格式一样。
	//		如果要为输入设置不同的格式，或者使用多个输入合成一个声音，就调用本函数。
	virtual	bool setInput( const SAudioFormat* format, int32_t inputIndex ) 	= 0;
	//关闭输入通道。关闭后如果要再使用，就必须调用 setInput 重新打开。
	//参数：iInputIndex		要关闭的通道。
	virtual	bool closeInput( int32_t inputIndex = 0 )	= 0;
	virtual	bool putFrame( const uint8_t* waveBuf, int32_t sampleCount, int32_t inputIndex = 0 )	= 0;
};

class IMediaDecoder
{
public:
    virtual ~IMediaDecoder();
	virtual bool start()	= 0;
	virtual bool pause()	= 0;
	virtual void stop()	= 0;
    virtual bool openFile(const QString& path)	= 0;
	virtual void closeFile()	= 0;
};

enum	EMediaFileFormat
{
	eFmtH264Raw,
	eFmtFlv,
	eFmtTs,
	eFmtAvi,
};
