#pragma once
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <algorithm>
#include <mutex>
#include <fstream>
using namespace std; 

#include "H264Codec.h"

class CMediaWriter;

class IOnReadFile
{
public:
    IOnReadFile();
    virtual ~IOnReadFile();
	virtual bool seek(int64_t pos, int32_t dir) = 0;
	virtual int32_t read(uint8_t* buf, int32_t size)	= 0;
};

class CMediaStream
{
public:
	CMediaStream();
	virtual ~CMediaStream();

	struct H264Frame
	{
        int64_t		pts;        //输入帧的 pts
        int64_t		dts;        //输入帧的 dts
        int64_t		ptsTimeMS;  //pts 和 dts 对应的时间由内部计算，不需要输入。
		int64_t		dtsTimeMS;
        bool		isKeyFrame; //内部判断是否是关键帧，不需要输入。
        //以下数据需要输入
        int32_t		nalCount;
		int32_t		payload;
		struct NAL
		{
			NalUnitType	nalType;
			int32_t		nalSize;
			const uint8_t*	nalData;
		};
        NAL		nals[1];        //
	};

	struct AUDFrame
	{
		int64_t		timestamp;
		int32_t		size;
		const uint8_t*	data;
	};
	//seek 时（仅解码状态），定位到关键帧的方式
	enum KeyFindAsSeek
	{
		Seek_Absolute,	//绝对定位到指定位置，不需要定位到关键帧
		Seek_Previous,	//定位到指定位置之前的关键帧
		Seek_After,		//定位到指定位置之后的关键帧
		Seek_Near,		//定们到和指定位置最近的关键帧
	};

	bool setVideoParam(const SVideoParams& params);
	bool setAudioParams(const SAudioParams& params);
	const SVideoParams& videoParams() const { return m_videoParams; }
	const SAudioParams& audioParams() const { return m_audioParams; }
	bool hasVideo() const { return m_hasVideo; }
	bool hasAudio() const { return m_hasAudio; }

	int32_t writerCount() const { return (int32_t)m_writers.size(); }
	CMediaWriter* writer(int32_t i) { return (i >= 0 && i < m_writers.size()) ? m_writers[i] : nullptr; }

	//开始分解、另存音视频流
	//如果 cbRead 有值，则是以文件方式读入，不能调用 putFileStream 接口。
	//否则不会调用 cbRead，需要由上层调用 putFileStream 输入数据。
	bool startParse(IOnReadFile* cbRead = nullptr);
	void endParse();
	bool getNextFrame();
	bool putFileStream(const uint8_t* data, int32_t length);
	bool putVideoStream(const uint8_t* data, int32_t length);
	bool putAudioStream(const uint8_t* data, int32_t length);

	bool putVideoFrame(H264Frame* frame);
	bool putAudioFrame(AUDFrame* frame);

	bool putVideoFrame(const uint8_t* frame, int32_t length, int64_t pts, int64_t dts);
	bool putAudioFrame(const uint8_t* frame, int32_t length, int64_t timestamp);

	const string& sps() const { return m_sps; }
	const string& pps() const { return m_pps; }
	const string& sei() const { return m_sei; }
	int64_t	videoFrameCount() const { return m_videoFrameNum; }
	int64_t videoTotalSize() const { return m_videoTotalSize; }
	int64_t videoDuration() const { return m_videoParams.frameRateNum ?
		m_videoDuration + 1000 * m_videoParams.frameRateDen / m_videoParams.frameRateNum :
		m_videoDuration; }
	const uint8_t* audioSpecificConfig(int32_t* size = nullptr) const { if (size) *size = m_audioSpecificConfigSize; return m_audioSpecificConfig; }
	int64_t audioFrameCount() const { return m_audioFrameNum; }
	int64_t audioTotalSize() const { return m_audioTotalSize; }
	int64_t audioDuration() const {
		return m_audioParams.samplesPerSec ?
			m_audioDuration + 1000 * m_aduioFrameSamples / m_audioParams.samplesPerSec :
			m_audioDuration;
	}
protected:
	friend CMediaWriter;
	SVideoParams	m_videoParams;
	SAudioParams	m_audioParams;
	IOnReadFile*	m_onReadFile;
	bool			m_hasVideo;		//是否存在视频 
	bool			m_hasAudio;		//是否存在音频
	bool			m_isOpened;		//文件是否已经打开完成，成功打开了文件才能写入。
	bool			m_headIsGeted;	//文件的头部是否已经获取到。
									//对于 h.264 的流，得到了 SPS/PPS 才算是得到了头数据。
	bool			m_dtsCompress;	//

	string			m_sps;			//这里记录的 SPS/PPS/SEI 都不包含前导字节。
	string			m_pps;
	string			m_sei;

	//视频相关的信息
	int32_t			m_videoDelayFrame;	//如果有 B 帧，则该值不为0。根据B帧的设置，值为 1 或 2.
	int64_t			m_videoDelayMS;	//第一个关键帧的 dts * -1 换算成毫秒数
	int64_t			m_videoInitDelta;	//当使用紧缩 dts 时，它是第一帧和第二帧 dts 的差值转换为毫秒数。
	int64_t			m_videoFrameNum;	//视频的累计帧数
	int64_t			m_videoDuration;	//已经保存的视频时长
	int64_t			m_videoTotalSize;
	//音频相关的信息
	int64_t			m_audioDelay;	//音频的第一帧的时间 * -1。
	int64_t			m_audioFrameNum;	//音频的累计帧数
	int64_t			m_audioDuration;	//已经保存的音频时长
	int64_t			m_audioTotalSize;
	uint8_t			m_audioSpecificConfig[8];
	uint16_t		m_audioSpecificConfigSize;
	uint32_t		m_aduioFrameSamples;		//编码时每次要输入的采样数


	bool putVideoSlice(const uint8_t* data, int32_t length);
	virtual bool onGetNextFrame();
	virtual bool onPutFileStream(const uint8_t* data, int32_t length);

	bool onWriteHeader();
	bool onWriteVideo(const H264Frame * frame);
	bool onWriteAudio(const AUDFrame * frame);

	void parseADTS(const uint8_t* data);
	void parseAAC_SequenceHeader(const uint8_t* data);
	void makeAAC_SequenceHeader();
	void checkSpsPpsSei(NalUnitType type, const uint8_t* data, int32_t length);

	inline  uint64_t dbl2int(double value) {
		return *((uint64_t*)&value);
	}
	inline  uint16_t endianFix16(uint16_t x)
	{
		return (x << 8) + (x >> 8);
	}
	inline  uint32_t endianFix32(uint32_t x)
	{
		return (x << 24) + ((x << 8) & 0xff0000) + ((x >> 8) & 0xff00) + (x >> 24);
	}
	inline  uint64_t endianFix64(uint64_t x)
	{
		return endianFix32(x >> 32) + ((uint64_t)endianFix32(x & 0xFFFFFFFF) << 32);
	}
private:
	bool			m_videoStreamBegin;
	bool			m_audioStreamBegin;
	bool			m_autoCheckAnnexb;
	bool			m_nalBufLoc;	//缓存当前帧时，使用输入的 buffer，而不使用 m_nalBuffer
	string			m_nalBuffer;	//缓存当前帧的数据
	string			m_lastVideo;	//缓存当前流的数据
	string			m_lastAudio;	//缓存当前流的数据
	
	H264Frame*		m_curFrame;
	struct FrameCache
	{
		H264Frame*	vid;
		AUDFrame*	aud;
		FrameCache() {
			vid = nullptr;
			aud = nullptr;
		}
	};
	map<int64_t, FrameCache>	m_frameCaches;
	bool writeToCache(H264Frame* vid, AUDFrame* aud);
	mutex			m_mutexWrite;
	vector<CMediaWriter*>	m_writers;
	//解码SPS,获取视频图像宽、高和帧率信息
	void h264_decode_sps(const uint8_t * spsBuf, uint32_t nLen);
	// H264的NAL起始码防竞争机制 @param buf SPS数据内容
	void de_emulation_prevention(uint8_t* buf, uint32_t* buf_size);
	uint32_t u(uint32_t BitCount, uint8_t * buf, uint32_t &nStartBit);
	int32_t Se(uint8_t *pBuff, uint32_t nLen, uint32_t &nStartBit);
	uint32_t Ue(uint8_t *pBuff, uint32_t nLen, uint32_t &nStartBit);

	inline int64_t convertTimebaseMs(int64_t ts)
	{
		return (ts * m_videoParams.frameRateDen * 10000 / m_videoParams.frameRateNum + 5) / 10;
	}

	union	SADTS
	{
		//由于 CPU 是小端序字节，而且位域的声明是低位在上面，
		//因此声明中的位域顺序与 ADTS 的顺序倒置，并且转换到结构时，要把ADTS字节流作为 uint64 转换为大端序。
		struct
		{
			//fixed
			uint64_t	syncword : 12;	//12	0xFFF
			uint64_t	ID : 1;	//13
			uint64_t	layer : 2;	//15
			uint64_t	protection_absent : 1;	//16

			uint64_t	profile : 2;	//18
			uint64_t	sampling_frequency_index : 4;	//22
			uint64_t	private_bit : 1;	//23
			uint64_t	channel_configuration : 3;	//26
			uint64_t	original_copy : 1;	//27
			uint64_t	home : 1;	//28
									//uint64_t	emphasis : 2;	//30

									//variable
			uint64_t	copyright_identiflcation_bit : 1;	//29
			uint64_t	copyright_identiflcation_start : 1;	//30
			uint64_t	aac_frame_length : 13;	//43
			uint64_t	adts_buffer_fullness : 11;	//54
			uint64_t	num_of_raw_data_blocks_in_frame : 2;	//56
		};
		uint64_t	adts;
	};
	const int32_t	AAC_Sampling_Frequency_Table[16] =
	{ 96000, 88200, 64000, 48000,
		44100, 32000, 24000, 22050,
		16000, 12000, 11025, 8000,
		7350, 0, 0, 0 };

	void restartMember();
	bool appendWriter(CMediaWriter* writer);
	bool removeWriter(CMediaWriter* writer);
};
