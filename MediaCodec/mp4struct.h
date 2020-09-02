#pragma once
#include "MediaStream.h"

class FourCC
{
public:
	FourCC() {
		m_fourCC = 0;
	}
	FourCC(uint32_t fcc) {
		m_fourCC = cpuIsLittleEndian() ?
			((fcc & 0xFF) << 24) | ((fcc & 0xFF00) << 8) | ((fcc & 0xFF0000) >> 8) | ((fcc) >> 24) : fcc;
	}

	FourCC(char ch1, char ch2, char ch3, char ch4) {
		m_fourCh[0] = ch1; m_fourCh[1] = ch2; m_fourCh[2] = ch3; m_fourCh[3] = ch4;
	}
	FourCC& operator = (uint32_t fcc) {
		m_fourCC = cpuIsLittleEndian() ?
			((fcc & 0xFF) << 24) | ((fcc & 0xFF00) << 8) | ((fcc & 0xFF0000) >> 8) | ((fcc) >> 24) : fcc;
		return *this;
	}
	operator uint32_t() {
		return cpuIsLittleEndian() ?
			((m_fourCC & 0xFF) << 24) | ((m_fourCC & 0xFF00) << 8) | ((m_fourCC & 0xFF0000) >> 8) | ((m_fourCC) >> 24) : m_fourCC;
	}
	const char* toArray() {
		return m_fourCh;
	}
	static bool cpuIsLittleEndian()
	{
		static const uint16_t endian = 0x0102;
		return ((const uint8_t*)&endian)[0] == 0x02;
	}

private:
	union
	{
		char		m_fourCh[4];
		uint32_t	m_fourCC;
	};
};

struct Mp4Struct
{
#pragma pack(push,1)
	struct Box_Base
	{
		uint32_t	size;		//当 size == 0 时表示是文件最后一个 box；当 size == 1 时，largeSize 有效，否则不存在 largeSize。
		FourCC		type;
	};
	struct Box_BaseLarge : public Box_Base
	{
		uint64_t	largeSize;
	};
	typedef Box_Base Box_mdat;
	typedef Box_Base Box_moov;	//Movie Box（moov）
	typedef Box_Base Box_mdia;
	typedef Box_Base Box_minf;	//Media Information Box（minf）
	typedef Box_Base Box_dinf;	//Data Information Box（dinf)
	typedef Box_Base Box_stbl;	//Sample Table Box (stbl) 
	struct Box_ftyp		//File Type Box（ftyp）
	{
		uint32_t	majorBrand;
		uint32_t	minorVersion;
		uint32_t	compatibleBrands[1];
	};
	union uint32_16
	{
		uint32_t	value;
		struct
		{
			uint16_t	low;
			uint16_t	high;
		};
		const uint32_16& operator = (uint32_t v) {
			value = v;
			return *this;
		}
		operator uint32_t()	{
			return value;
		}
	};
	union uint16_8
	{
		uint16_t	value;
		struct
		{
			uint8_t	low;
			uint8_t	high;
		};
		const uint16_8& operator = (uint16_t v) {
			value = v;
			return *this;
		}
		operator uint16_t() {
			return value;
		}
	};
	struct Box_FillHead
	{
		uint8_t		version;	//box版本，0或1，一般为0
		uint8_t		flags[3];	//
	};
	struct Box_FillHead32 : public Box_FillHead
	{
		uint32_t	creationTime;	//创建时间（相对于UTC时间1904 - 01 - 01零点的秒数）
		uint32_t	modificationTime;	//修改时间
	};
	struct Box_FillHead64 : public Box_FillHead
	{
		uint64_t	creationTime;	//创建时间（相对于UTC时间1904 - 01 - 01零点的秒数）
		uint64_t	modificationTime;	//修改时间
	};
	struct Box_mvhd32 : public Box_FillHead32	//Movie Header Box（mvhd）
	{
		uint32_t	timeScalc;	//文件媒体在1秒时间内的刻度值，可以理解为1秒长度的时间单元数
		uint32_t	duration;	//该track的时间长度，用duration和time scale值可以计算track时长，比如audio track的time scale = 8000, duration = 560128，时长为70.016，video track的time scale = 600, duration = 42000，时长为70
		uint32_16	rate;		//推荐播放速率，高16位和低16位分别为小数点整数部分和小数部分，即[16.16] 格式，该值为1.0（0x00010000）表示正常前向播放
		uint16_8	volume;		//与rate类似，[8.8] 格式，1.0（0x0100）表示最大音量
		uint8_t		reserved[10];
		uint32_t	matrix[9];
		uint32_t	preDefined[6];
		uint32_t	nextTrackId;	//下一个track使用的id号
	};
	struct Box_mvhd64 : public Box_FillHead64	//Movie Header Box（mvhd）
	{
		uint32_t	timeScalc;	//文件媒体在1秒时间内的刻度值，可以理解为1秒长度的时间单元数
		uint64_t	duration;	//该track的时间长度，用duration和time scale值可以计算track时长，比如audio track的time scale = 8000, duration = 560128，时长为70.016，video track的time scale = 600, duration = 42000，时长为70
		uint32_16	rate;		//推荐播放速率，高16位和低16位分别为小数点整数部分和小数部分，即[16.16] 格式，该值为1.0（0x00010000）表示正常前向播放
		uint16_8	volume;		//与rate类似，[8.8] 格式，1.0（0x0100）表示最大音量
		uint8_t		reserved[10];
		uint32_t	matrix[9];
		uint32_t	preDefined[6];
		uint32_t	nextTrackId;	//下一个track使用的id号
	};

	struct Box_tkhd32 : public Box_FillHead32	//Track Header Box
	{
		//flags 按位或操作结果值，预定义如下：0x000001 track_enabled，否则该track不被播放；
		//								0x000002 track_in_movie，表示该track在播放中被引用；
		//								0x000004 track_in_preview，表示该track在预览时被引用。
		//一般该值为7，如果一个媒体所有track均未设置track_in_movie和track_in_preview，将被理解为所有track均设置了这两项；对于hint track，该值为0
		uint32_t	trackId;	//id号，不能重复且不能为0
		uint32_t	reserved;
		uint32_t	duration;	//该track的时间长度，用duration和time scale值可以计算track时长，比如audio track的time scale = 8000, duration = 560128，时长为70.016，video track的time scale = 600, duration = 42000，时长为70
		uint32_t	reserved2[2];
		uint16_t	layer;		//视频层，默认为0，值小的在上层
		uint16_t	alternateGroup;	//track分组信息，默认为0表示该track未与其他track有群组关系
		uint16_8	volume;		//与rate类似，[8.8] 格式，1.0（0x0100）表示最大音量
		uint16_t	reserved3;
		uint32_t	matrix[9];
		uint32_16	width;		//宽
		uint32_16	height;		//高，均为 [16.16] 格式值，与sample描述中的实际画面大小比值，用于播放时的展示宽高
	};
	struct Box_tkhd64 : public Box_FillHead64	//Track Header Box
	{
		uint32_t	trackId;	//id号，不能重复且不能为0
		uint32_t	reserved;
		uint64_t	duration;	//该track的时间长度，用duration和time scale值可以计算track时长，比如audio track的time scale = 8000, duration = 560128，时长为70.016，video track的time scale = 600, duration = 42000，时长为70
		uint32_t	reserved2[2];
		uint16_t	layer;		//视频层，默认为0，值小的在上层
		uint16_t	alternateGroup;	//track分组信息，默认为0表示该track未与其他track有群组关系
		uint16_8	volume;		//[8.8] 格式，如果为音频track，1.0（0x0100）表示最大音量；否则为0
		uint16_t	reserved3;
		uint32_t	matrix[9];
		uint32_16	width;		//宽
		uint32_16	height;		//高，均为 [16.16] 格式值，与sample描述中的实际画面大小比值，用于播放时的展示宽高
	};

	struct Box_mdhd32 : public Box_FillHead32	//Media Header Box
	{
		uint32_t	timeScalc;	//文件媒体在1秒时间内的刻度值，可以理解为1秒长度的时间单元数
		uint32_t	duration;	//track的时间长度
		uint16_t	language;
		uint16_t	preDefined;
	};
	struct Box_mdhd64 : public Box_FillHead64	//Media Header Box
	{
		uint32_t	timeScalc;	//文件媒体在1秒时间内的刻度值，可以理解为1秒长度的时间单元数
		uint64_t	duration;	//track的时间长度
		uint16_t	language;
		uint16_t	preDefined;
	};

	struct Box_hdlr : public Box_FillHead	//Handler Reference Box（hdlr）
	{
		uint32_t	preDefined;
		FourCC		handlerType;	//在media box中，该值为4个字符：
									//“vide”— video track
									//“soun”— audio track
									//“hint”— hint track
		uint32_t	reserved[3];
		char		name[1];
	};

	struct Box_vmhd : public Box_FillHead	//Video Media Header Box（vmhd） 
	{
		uint16_t	graphicsMode;	//视频合成模式，为0时拷贝原始图像，否则与opcolor进行合成
		uint16_t	opColor[3];		//｛red，green，blue｝
	};

	struct Box_smhd : public Box_FillHead	//Sound Media Header Box（smhd） 
	{
		uint16_8	balance;	//立体声平衡，[8.8] 格式值，一般为0，-1.0表示全部左声道，1.0表示全部右声道
		uint16_t	reserved;
	};

	struct Box_dref : public Box_FillHead	//Data Information Box（dinf）
	{
		uint32_t	entryCount;
	};

	struct SampleEntry
	{
		uint8_t	reserved[6];
		uint16_t	dataReferenceIndex;
	};
	struct SampleEntryHint : public SampleEntry
	{
		uint8_t	data[1];
	};
	struct SampleEntryVisual : public SampleEntry
	{
		uint16_t	preDefine;
		uint16_t	reserved2;
		uint32_t	preDefine2[3];
		uint16_t	width;
		uint16_t	height;
		uint32_16	horizresolution;
		uint32_16	vertresolution;
		uint32_t	reserved3;
		uint16_t	frame_count;
		char		compressorname[32];
		uint16_t	depth;
		int16_t		preDefined3;
	};
	struct SampleEntryAudio : public SampleEntry
	{
		uint32_t	reserved2[2];
		uint16_t	channelCount;
		uint16_t	sampleSize;
		uint16_t	preDefined;
		uint16_t	reserved3;
		uint32_t	sampleRate;
	};
	//https://blog.csdn.net/u013752202/article/details/80557459
	//(sample descriptions)采样描述box
	struct Box_stsd : public Box_FillHead
	{
		uint32_t	entryCount;
	};

	//*(decoding)(time to stamp)采样时戳因映射表 stts box
	struct Box_stts : public Box_FillHead
	{
		uint32_t	timeToSample;	//条目数
		struct
		{
			uint32_t	sampleCount;	//有相同duration的连续sample的数目
			uint32_t	sampleDuration;	//每个sample的duration
		}samples[1];
	};

	//ctts	(composition)time to sample	//CTS - DTS
	typedef Box_stts Box_ctts;

	struct Box_stss : public Box_FillHead	// (sync sample table)关键帧列表box
	{
		uint32_t	syncSample;		//关键帧的数量
		uint32_t	samples[1];		//关键帧编号的列表
	};

	//*(sample to chunk)sample和chunk映射表 stsc box
	//具有相同 sample 个数的 chunk 组成 entry
	struct	Box_stsc : public Box_FillHead
	{
		uint32_t	sampleToChunk;	//条目数量
		struct
		{
			uint32_t	firstChunk;	//这个table使用的第一个chunk序号
			uint32_t	sampleCount;	//当前entry内每个trunk的sample数目
			uint32_t	descriptionId;	//在 stsd 中的序号
		}chunks[1];
	};
	//*(chunk offset)每个chunk相对于文件头的偏移(大于2G的文件为stco64)box
	struct Box_stco : public Box_FillHead	
	{
		uint32_t	chunkCount;		//chunk offset的数目
		uint32_t	offsets[1];		//字节偏移量从文件开始到当前chunk。这个表根据chunk number索引，第一项就是第一个trunk，第二项就是第二个trunk
	};
	struct Box_co64 : public Box_FillHead	//(chunk offset)每个chunk相对于文件头的偏移(大于2G的文件为stco64)box
	{
		uint32_t	chunkCount;		//chunk offset的数目
		uint64_t	offsets[1];		//字节偏移量从文件开始到当前chunk。这个表根据chunk number索引，第一项就是第一个trunk，第二项就是第二个trunk
	};
	struct Box_stsz : public Box_FillHead	// (sample size)每个sample的大小(有可能为stz2)box
	{
		uint32_t	sampleSize;	//如果所有的sample有相同的长度，这个字段就是这个值。否则，这个字段的值就是0。那些长度存在sample size表中
		uint32_t	sampleCount;	//sample size的数目
		uint32_t	sizes[1];
	};
#pragma pack(pop)
	struct Mp4Box
	{
		Box_BaseLarge	head;
		uint64_t		doneSize;
		string			data;
		Mp4Box*			parent;
		vector<Mp4Box*>	childs;
		Mp4Box()
		{
			head.size = 0;
			head.type = uint32_t(0);
			head.largeSize = 0xFFFFFFFFFFFFFFFF;
			doneSize = 0;
			parent = nullptr;
		}
		Mp4Box(const FourCC& box, Mp4Box* p, uint32_t initSize = 0)
		{
			head.size = 0;
			head.type = box;
			head.largeSize = 0xFFFFFFFFFFFFFFFF;
			doneSize = 0;
			parent = p;
			data.resize(initSize);
			if (p)
			{
				p->childs.push_back(this);
			}
		}
		Mp4Box(const Box_BaseLarge& boxHead, Mp4Box* p)
		{
			head = boxHead;
			if (head.size == 0)
			{
				head.largeSize = 0xFFFFFFFFFFFFFFFF;
			}
			else if (head.size != 1)
			{
				head.largeSize = head.size;
			}
			doneSize = 0;
			parent = p;
			if (p)
			{
				p->childs.push_back(this);
			}
		}
		~Mp4Box()
		{
			for (auto i = childs.begin(); i != childs.end(); ++i)
			{
				(*i)->parent = nullptr;
				delete (*i);
			}
			childs.clear();
			if (parent)
			{
				for (auto i = parent->childs.begin(); i != parent->childs.end(); ++i)
				{
					if ((*i) == this)
					{
						parent->childs.erase(i);
						do
						{
							parent->doneSize -= doneSize;
							parent = parent->parent;
						} while (parent);
						break;
					}
				}
			}
		}
		bool doDone(int64_t len, Mp4Box**cur = nullptr )
		{
			Mp4Box*	p = this;
			bool	ret = false;
			do
			{
				p->doneSize += len;
				if (cur && p->doneSize == p->head.largeSize)
				{
					*cur = p->parent;
					ret = true;
				}
				p = p->parent;
			} while (p);
			return ret;
		}

		Mp4Box* find(FourCC type)
		{
			if (head.type == type)
				return this;
			for (auto i = childs.begin(); i != childs.end(); ++i)
			{
				Mp4Box*	p = (*i)->find(type);
				if (p) return p;
			}
			return nullptr;
		}

		Mp4Box* findParent(FourCC type)
		{
			Mp4Box*	p = parent;
			while (p)
			{
				if (p->head.type == type)
					return p;
				p = p->parent;
			}
			return nullptr;
		}
		void resetSize()
		{
			doneSize = data.size();
			for (auto i = childs.begin(); i != childs.end(); ++i)
			{
				(*i)->resetSize();
				doneSize += (*i)->doneSize;
			}
			doneSize += sizeof(Box_Base);
			if (doneSize <= 0xFFFFFFFF)
			{
				head.size = (uint32_t)doneSize;
			}
			else
			{
				doneSize += 8;
				head.size = 1;
			}
			head.largeSize = doneSize;
		}
		void printInfo()
		{
			Mp4Box*	p = parent;
			while (p)
			{
				p = p->parent;
				printf("  ");
			}
			const char* t = head.type.toArray();
			printf("%c%c%c%c  %lld (%lld)\n", t[0], t[1], t[2], t[3], head.largeSize, doneSize/* - doneSize*/);

			for (auto i = childs.begin(); i != childs.end(); ++i)
				(*i)->printInfo();
		}

	};

	struct avcC_Info
	{
		uint8_t	configurationVersion;
		uint8_t	AVCProfileIndication;
		uint8_t	profile_compatibility;
		uint8_t	AVCLevelIndication;
		uint8_t	lengthSizeMinusOne;
		uint8_t	numOfSequenceParameterSets;
		uint8_t* spsEntry;
		uint8_t	numOfPictureParameterSets;
		uint8_t* ppsEntry;
	};
	struct btrt_Info
	{
		uint32_t bufferSizeDB;
		uint32_t maxBitrate;
		uint32_t avgBitrate;
	};
	struct esds_Info
	{
		//ES_DescrTag
		uint16_t	ES_ID;
		uint16_t	dependsOn_ES_IS;
		char*		URLString;
		uint16_t	OCR_ES_id;
		//DecoderConfigDescriptor TAG
		uint8_t	objectTypeIndication;
		uint8_t	streamType;
		uint32_t bufferSizeDB;
		uint32_t maxBitrate;
		uint32_t avgBitrate;
		//DecSpecificInfotag
		uint16_t specificSize;
		uint8_t* specificBytes;
	};

	//struct MP4CodecTagTab
	//{
	//	FourCC	codec;
	//	uint8_t	typeInd;
	//};
	//const MP4CodecTagTab mp4_obj_type[] = {
	//	{ CODEC_ID_MOV_TEXT  , 0x08 },
	//{ CODEC_ID_MPEG4     , 0x20 },
	//{ CODEC_ID_H264      , 0x21 },
	//{ CODEC_ID_AAC       , 0x40 },
	//{ CODEC_ID_MP4ALS    , 0x40 }, /* 14496-3 ALS */
	//{ CODEC_ID_MPEG2VIDEO, 0x61 }, /* MPEG2 Main */
	//{ CODEC_ID_MPEG2VIDEO, 0x60 }, /* MPEG2 Simple */
	//{ CODEC_ID_MPEG2VIDEO, 0x62 }, /* MPEG2 SNR */
	//{ CODEC_ID_MPEG2VIDEO, 0x63 }, /* MPEG2 Spatial */
	//{ CODEC_ID_MPEG2VIDEO, 0x64 }, /* MPEG2 High */
	//{ CODEC_ID_MPEG2VIDEO, 0x65 }, /* MPEG2 422 */
	//{ CODEC_ID_AAC       , 0x66 }, /* MPEG2 AAC Main */
	//{ CODEC_ID_AAC       , 0x67 }, /* MPEG2 AAC Low */
	//{ CODEC_ID_AAC       , 0x68 }, /* MPEG2 AAC SSR */
	//{ CODEC_ID_MP3       , 0x69 }, /* 13818-3 */
	//{ CODEC_ID_MP2       , 0x69 }, /* 11172-3 */
	//{ CODEC_ID_MPEG1VIDEO, 0x6A }, /* 11172-2 */
	//{ CODEC_ID_MP3       , 0x6B }, /* 11172-3 */
	//{ CODEC_ID_MJPEG     , 0x6C }, /* 10918-1 */
	//{ CODEC_ID_PNG       , 0x6D },
	//{ CODEC_ID_JPEG2000  , 0x6E }, /* 15444-1 */
	//{ CODEC_ID_VC1       , 0xA3 },
	//{ CODEC_ID_DIRAC     , 0xA4 },
	//{ CODEC_ID_AC3       , 0xA5 },
	//{ CODEC_ID_DTS       , 0xA9 }, /* mp4ra.org */
	//{ CODEC_ID_VORBIS    , 0xDD }, /* non standard, gpac uses it */
	//{ CODEC_ID_DVD_SUBTITLE, 0xE0 }, /* non standard, see unsupported-embedded-subs-2.mp4 */
	//{ CODEC_ID_QCELP     , 0xE1 },
	//{ CODEC_ID_MPEG4SYSTEMS, 0x01 },
	//{ CODEC_ID_MPEG4SYSTEMS, 0x02 },
	//{ CODEC_ID_NONE      ,    0 },
	//};
};
//MP4文件格式
//http://www.cnblogs.com/qq260250932/p/4282304.html
