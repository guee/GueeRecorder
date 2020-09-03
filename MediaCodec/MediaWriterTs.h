#pragma once
#include "MediaWriter.h"
class GueeMediaWriterTs :
    public GueeMediaWriter
{
public:
    GueeMediaWriterTs(GueeMediaStream& stream);
    ~GueeMediaWriterTs();

	virtual bool onWriteHeader();
    virtual bool onWriteVideo(const GueeMediaStream::H264Frame * frame);
    virtual bool onWriteAudio(const GueeMediaStream::AUDFrame * frame);

#pragma pack(push,1)
	//http://www.cnblogs.com/tocy/p/media_container_6-mpegts.html
	struct	TS_Packet_Header
	{
		uint32_t	sync_byte : 8;	//（同步字节）：固定为0x47;该字节由解码器识别，使包头和有效负载可相互分离。
		uint32_t	transport_error_indicator : 1;	//（传输错误标志）：‘1’表示在相关的传输包中至少有一个不可纠正的错误位。当被置1后，在错误被纠正之前不能重置为0。
		uint32_t	payload_unit_start_indicator : 1;	//（负载起始标志）：为1时，表示当前TS包的有效载荷中包含PES或者PSI的起始位置；在前4个字节之后会有一个调整字节，其的数值为后面调整字段的长度length。因此有效载荷开始的位置应再偏移1+[length]个字节。
		uint32_t	transport_priority : 1;	//（传输优先级标志）：‘1’表明当前TS包的优先级比其他具有相同PID， 但此位没有被置‘1’的TS包高。
		uint32_t	PID : 13;	// 指示存储与分组有效负载中数据的类型。PID值0x0000—0x000F保留。其中0x0000为PAT保留；0x0001为CAT保留；0x1fff为分组保留，即空包。
		uint32_t	transport_scrambling_control : 2;	//（加扰控制标志）：表示TS流分组有效负载的加密模式。空包为‘00’，如果传输包包头中包括调整字段，不应被加密。其他取值含义是用户自定义的。
		uint32_t	adaptation_field_control : 2;	//（适配域控制标志）：表示包头是否有调整字段或有效负载。‘00’为ISO/IEC未来使用保留；‘01’仅含有效载荷，无调整字段；‘10’ 无有效载荷，仅含调整字段；‘11’ 调整字段后为有效载荷，调整字段中的前一个字节表示调整字段的长度length，有效载荷开始的位置应再偏移[length]个字节。空包应为‘10’。
		uint32_t	continuity_counter : 4;	//（连续性计数器）：随着每一个具有相同PID的TS流分组而增加，当它达到最大值后又回复到0。范围为0~15。
	};
	//节目关联表Program Association Table (PAT) 0x0000	TS流中包含一个或者多个PAT表。PAT表由PID为0x0000的TS包传送，其作用是为复用的每一路传送流提供出所包含的节目和节目编号，以及对应节目的PMT的位置即PMT的TS包的PID值，同时还提供NIT的位置，即NIT的TS包的PID的值。
	//节目映射表Program Map Tables (PMT)	PMT在传送流中用于指示组成某一套节目的视频、音频和数据在传送流中的位置，即对应的TS包的PID值，以及每路节目的节目时钟参考（PCR）字段的位置。 
	//条件接收表Conditional Access Table (CAT) 0x0001
	//网络信息表Network Information Table(NIT) 0x0010
	//传输流描述表Transport Stream Description Table(TSDT) 0x02

	struct	TS_PAT
	{
		uint64_t	table_id : 8;	//固定为0x00，标志是该表是PAT
		uint64_t	section_syntax_indicator : 1;	//段语法标志位，固定为1
		uint64_t	zero : 1;	//0
		uint64_t	reserved_1 : 2;	//保留位
		uint64_t	section_length : 12;	//表示后面的有用的字节数，包括CRC32
		uint64_t	transport_stream_id : 16;	//该传输流的ID，区别于一个网络中其它多路复用的流
		uint64_t	reserved_2 : 2;	//保留位
		uint64_t	version_number : 5;	//范围0-31，表示PAT的版本号
		uint64_t	current_next_indicator : 1;	//发送的PAT是当前有效还是下一个PAT有效
		uint64_t	section_number : 8;	//分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
		uint64_t	last_section_number : 8;	//表示PAT最后一个分段的号码。
		uint32_t	CRC_32 : 32;
	};
	struct	TS_PAT_PROGRAM
	{
		uint32_t	program_number : 16;	//节目号
		uint32_t	reserved_3 : 3;
		union {
			uint32_t	program_map_PID : 13;	//节目映射表（PMT）的PID号，节目号为大于等于1时，对应的ID为program_map_PID。一个PAT中可以有多个program_map_PID。
			uint32_t	network_PID : 13;	//网络信息表（NIT）的PID,节目号为0时对应ID为network_PID。
		};
	};

	struct TS_PMT
	{
		uint64_t	table_id : 8;	//固定为0x02，标志该表是PMT 表。
		uint64_t	section_syntax_indicator : 1;	//对于PMT表，设置为1 。
		uint64_t	zero : 1;	//0
		uint64_t	reserved_1 : 2;	//
		uint64_t	section_length : 12;	//表示这个字节后面有用的字节数，包括CRC32 。
		uint64_t	program_number : 16;	//它指出该节目对应于可应用的Program map PID 。
		uint64_t	reserved_2 : 2;
		uint64_t	version_number : 5;	//指出PMT 的版本号。
		uint64_t	current_next_indicator : 1;	//当该位置’1’时，当前传送的Program map section可用；当该位置’0’时，指示当前传送的Program map section不可用，下一个TS流的Programmap section 有效。
		uint64_t	section_number : 8;	//总是置为0x00（因为PMT表里表示一个service的信息，一个section 的长度足够）。
		uint64_t	last_section_number : 8;	//该域的值总是0x00 。

		uint32_t	reserved_3 : 3;
		uint32_t	PCR_PID : 13;	//节目时钟参考所在TS分组的PID。节目中包含有效PCR字段的传送流中PID 。
		uint32_t	reserved_4 : 4;
		uint32_t	program_info_length : 12;	//12bit域，前两位为00。该域指出跟随其后对节目信息的描述的byte 数。

		uint32_t	CRC_32 : 32;
	};
	struct	TS_PMT_STREAM
	{
		uint64_t	stream_type : 8;	//指示特定PID的节目元素包的类型。该处PID由elementary PID 指定。
		uint64_t	reserved_5 : 3;
		uint64_t	elementary_PID : 13;
		uint64_t	reserved_6 : 4;
		uint64_t	ES_info_length : 12;
	};
	//PES相关资料
	//http://blog.csdn.net/cabbage2008/article/details/49848937
	//http://blog.csdn.net/u013354805/article/details/51591229
	struct TS_PES
	{
		uint32_t	packet_start_code_prefix : 24;
		uint32_t	stream_id : 8;

		uint16_t	PES_packet_length;

		uint8_t		fix_bit : 2;		//标志位，固定为 10
		uint8_t		PES_scrambling_control : 2;		//PES 加扰控制
		uint8_t		PES_priority : 1;		//PES 优先级
		uint8_t		data_alignment_indicator : 1;		//数据定位指示符
		uint8_t		copyright : 1;		//版权
		uint8_t		original_or_copy : 1;		//原始的或复制的

		uint8_t		PTS_DTS_flags : 2;		//时间戳标记
		uint8_t		ESCR_flag : 1;
		uint8_t		ES_rate_flag : 1;
		uint8_t		DSM_trick_mode_flag : 1;
		uint8_t		additional_copy_info_flag : 1;
		uint8_t		PES_CRC_flag : 1;
		uint8_t		PES_extension_flag : 1;

		uint8_t		PES_header_data_length;
	};

#pragma pack(pop)
	struct TS_Stream
	{
		uint16_t	programNumber;
		uint16_t	stream_type;	//指示特定PID的节目元素包的类型。该处PID由elementary PID 指定。
		uint16_t	elementary_PID;
		vector<uint8_t>	ES_info;
	};
	map<uint16_t, uint8_t> m_pidCounter;
	map<uint16_t, uint16_t> m_patProgs;
	vector<TS_Stream> m_pmtStreams;
	string		m_videoCache;
	string		m_audioCache;
	int64_t	m_audioTime;

	uint8_t m_pack[188];
	int32_t m_startBit;
	uint32_t crc32(const uint8_t* data, uint32_t size);
	void setTsPacketHeader(uint16_t pid, bool isStart = true, uint8_t startBytes = 0, int64_t pcr = -1);
	void makeTsPat();
	void makeTsPmt(uint16_t programNumber);
	void writeBits(uint8_t bits, uint32_t data);
};


