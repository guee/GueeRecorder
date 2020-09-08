#pragma once
#include <stdint.h>
//视频编码器的类型
enum EVideoEncoder
{
	VE_X264,				//x264 软件编码器
	VE_CUDA,				//nvidia CUDA 硬件加速编码器
	VE_NVENC,				//nvidia NVENC 硬件编码器
    VE_INTEL,				//intel 核显加速编码器
};
static const char * const video_encoder_names[] = { "x264", "nvcuda", "nvenc", "intel", 0 };

//视频编码配置，profile。
enum EVideoProfile
{
	VF_Auto,
	VF_BaseLine,
	VF_Main,
	VF_High
};
static const char * const video_profile_names[] = { "auto", "baseline", "main", "high", 0 };

//用于各种视频编码器的 Preset 配置
enum EVideoPreset_x264
{
	VP_x264_UltraFast,
	VP_x264_SuperFast,
	VP_x264_VeryFast,
	VP_x264_Faster,
	VP_x264_Fast,
	VP_x264_Medium,
	VP_x264_Slow,
	VP_x264_Slower,
	VP_x264_VerySlow,
	VP_x264_Placebo
};
static const char * const video_preset_x264_names[] = { "UltraFast", "SuperFast", "VeryFast", "Faster", "Fast",
"Medium", "Slow", "Slower", "VerySlow", "Placebo", 0 };

enum EVideoPreset_Cuda
{
	VP_Cuda_PSP,		//width 320  height 240
	VP_Cuda_iPod,		//width 320  height 240
	VP_Cuda_AVCHD,		//width 1920 height 1080
	VP_Cuda_BD,			//width 1920 height 1080
	VP_Cuda_HDV1440,	//width 1440 height 1080
	VP_Cuda_ZuneHD,		//width 720  height 480
	VP_Cuda_FlipCam
};
static const char * const video_preset_cuda_names[] = { "PSP", "iPod", "AVCHD", "BlurayDisk", "HDV1440", "ZuneHD", "FlipCam", 0 };

enum EVideoPreset_Nvenc
{
	VP_Nvenc_Default,
	VP_Nvenc_HighPerformance,
	VP_Nvenc_HighQuality,
	VP_Nvenc_BlurayDisk,
	VP_Nvenc_LowLatencyDefault,
	VP_Nvenc_LowLatencyHP,
	VP_Nvenc_LowLatencyHQ,
	VP_Nvenc_LosslessDefault,
	VP_Nvenc_LosslessHP
};
static const char * const video_preset_nvenc_names[] = { "Default", "HighPerformance", "HighQuality", "BlurayDisk", "LowLatencyDefault",
"LowLatencyHP", "LowLatencyHQ", "LosslessDefault", "LosslessHP", 0 };

enum EVideoPreset_Intel
{
	VP_Intel_Speed,
	VP_Intel_Balanced,
	VP_Intel_Quality
};
static const char * const video_preset_intel_names[] = { "Speed", "Balanced", "Quality", 0 };

//视频编码器码率控制方式设置
enum EVideoRateMode
{
	VR_AverageBitrate,		//平均码率(ABR)		
							//x264 使用 X264_RC_ABR平均码率
							//Nvenc	NV_ENC_PARAMS_RC_VBR
							//Cuda	RC_VBR

	VR_VariableBitrate,		//可变码率(VBR)		
							//x264 使用 X264_RC_ABR平均码率 进行控制
							//Nvenc	NV_ENC_PARAMS_RC_VBR
							//Cuda	RC_VBR

	VR_ConstantBitrate,		//固定码率(CBR)		
							//x264 使用 X264_RC_CRF 进行控制
							//Nvenc	NV_ENC_PARAMS_RC_CBR
							//Cuda	RC_CBR

	VR_ConstantQP,			//恒定质量(CQP)		
							//x264 使用 X264_RC_CQP恒定质量
							//Nvenc	NV_ENC_PARAMS_RC_CBR, 如果 VBV 参数有值，则使用 NV_ENC_PARAMS_RC_VBR_MINQP
							//Cuda	RC_CQP / RC_VBR_MINQP
};

enum ESampleBits
{
	eSampleBit8i = (1 << 16) | 8,
	eSampleBit16i = (2 << 16) | 16,		//支持 AAC 编码输出
	eSampleBit24i = (3 << 16) | 24,
	eSampleBit32i = (4 << 16) | 32,		//支持 AAC 编码输出
	eSampleBit24In32i = (5 << 16) | 32,
	eSampleBit32f = (6 << 16) | 32,		//支持 AAC 编码输出
};

//音频编码，
enum	EAudioCodec
{
	AC_UNKNOW,

	AC_AAC,
	AC_PCM,
	AC_MP3,

	AC_AC3,
	AC_DTS,
	AC_MP2AAC
};

enum EVideoCodec
{
	VC_UNKNOW,
	VC_H264,
	VC_XVID,
	VC_DIVX,
	VC_MPG1,
	VC_MPG2,
};

enum	EVideoCSP
{
	Vid_CSP_I420,			//IYUV  /* yuv 4:2:0 planar */	YYYY YYYY UU VV
	Vid_CSP_YV12,			//  /* yvu 4:2:0 planar */	YYYY YYYY VV UU
	Vid_CSP_NV12,			//  /* yuv 4:2:0, with one y plane and one packed u+v */	YYYY YYYY UVUV
	Vid_CSP_NV21,			//  /* yuv 4:2:0, with one y plane and one packed v+u */	YYYY YYYY VUVU
	Vid_CSP_I422,			//  /* yuv 4:2:2 planar */	YYYY YYYY UUUU VVVV

	Vid_CSP_YV16,			//  /* yvu 4:2:2 planar */	YYYY YYYY VVVV UUUU
	Vid_CSP_NV16,			//  /* yuv 4:2:2, with one y plane and one packed u+v */	YYYY YYYY UVUVUVUV
    Vid_CSP_YUY2,			//  /* yuv 4:2:2, YUYV YUYV
    Vid_CSP_UYVY,			//  /* yuv 4:2:2, UYVY UYVY

	Vid_CSP_V210,			//0x0008  /* 10-bit yuv 4:2:2 packed in 32 */

	Vid_CSP_I444,			//0x0009  /* yuv 4:4:4 planar */
	Vid_CSP_YV24,			//0x000a  /* yvu 4:4:4 planar */
    Vid_CSP_BGR,			//0x000b  /* packed bgr 24bits   */
    Vid_CSP_BGRA,			//0x000c  /* packed bgr 32bits   */	A8R8G8B8
    Vid_CSP_RGB,			//0x000d  /* packed rgb 24bits   */
};

static const char * const video_color_space_names[] = { "I420/IYUV", "YV12", "NV12", "NV21", "I422",
"YV16", "NV16", "YUY2", "UYVY", "V210",
"I444", "YV24", "AYUV", "RGB", "BGR",
"ARGB", "ARGB10", 0 };
enum	EPsyTuneType
{
	eTuneNone,
	eTuneFilm,
	eTuneAnimation,
	eTuneGrain,
	eTuneStillimage,
	eTunePsnr,
	eTuneSsim
};

struct	SVideoParams
{
	bool			enabled;		//编解码时是否需要视频
	EVideoEncoder	encoder;		//当前的编解码器
	EVideoProfile	profile;
	EVideoPreset_x264	presetX264;
	EVideoPreset_Cuda	presetCuda;
	EVideoPreset_Nvenc	presetNvenc;
	EVideoPreset_Intel	presetIntel;
    EVideoCSP		outputCSP;	//输出视频的像素格式。
    EPsyTuneType	psyTune;	//
	int32_t		width;			//视频的宽度
    int32_t		height;         //视频的高度

    float		frameRate;      //视频的帧率，如果 bVfr 和 isOnlineMode 都为 false，时间基也会设置为帧率的倒数。
    bool		vfr;			//可变帧率。如果设置为true，则当帧不连续时保持码率恒定，视频质量高。设置为false则输入帧数量少码率就低。
    bool		onlineMode;     //是否是在线模式，即输入的图像是否是实时采集的，输出的视频流也要尽量减少延迟。
								//设置 true 时会禁止编码 B 帧，输入帧数据后立即返回，如果编码速度过慢则丢弃帧。
								//设置 false 时不会丢弃帧，如果缓冲队列满，输入帧数据后等待缓冲队列有空位时才返回。
	bool		annexb;			//设置为false，包的前 4 字节为数据长度;设置为true，包的前 3 字节或 4 字节为前缀码。00 00 00 01 或 00 00 01
	bool		optimizeStill;	//对图像静止不变的部分进行编码优化。
	bool		fastDecode;	//使编码出的视频能够被快速解码。
    int32_t		threadNum;		//编码线程数。0为自动线程数，否则就是指定的线程数量。
	EVideoRateMode	rateMode;	//码率控制方式
	int32_t		bitrate;		//平均码率，如果为 0 就自动计算出一个码率。
    int32_t		bitrateMax;     //最大码率
    int32_t		bitrateMin;     //最小码率，默认为0，不使用最小码率设置。否则根据文件写入速度或网络上传速度自动重设码率。
	int32_t		vbvBuffer;		//码率可变时，进行码率控制的缓存大小
    int32_t		gopMax;         //最大关键帧间隔（小于0则使用默认值）
    int32_t		gopMin;         //最小关键帧间隔（小于0则使用默认值）
	int32_t		refFrames;		//向前参考帧的最大数量。（小于0则使用默认值）
	int32_t		BFrames;		//两个P 帧之间 B 帧的数量。（小于0则使用默认值）
	int32_t		BFramePyramid;	//0：不使用 B 帧为参考帧，1或2：使用B帧为参考帧。
};

struct	SPcmFormat
{
    ESampleBits sampleBits;
    int32_t		samplesRate;	//音频的采样率
    int32_t		channels;		//音频的声道数，0表示没有音频，1=单声道，2=双声道
    uint32_t	channelMask;	//如果声道数大于2，可以设置声道MASK, 参考微软WAVEFORMATEXTENSIBLE结构中dwChannelMask的说明。
};

struct	SAudioParams : public SPcmFormat
{
	bool		enabled;		//编解码时是否需要音频
	EAudioCodec	eCodec;			//音频编解码器
	bool		isOnlineMode;	//是否是在线模式，即输入的声音数据是否是实时采集的。
								//当为在线模式时，如果输入的声音采样数与实际时间有差异时，会自动修正声音数据，减少时间差异。
	bool		useADTS;
    uint32_t	encLevel;		//1,2,3,4 对应 aac 的 MAIN,LOW,SSR,LTP。其它值都使用LOW。
    int32_t		bitrate;        //音频编码的码率（kbps），如果为 小于等于 0 就使用编码器支持的最大码率。
};

enum NalUnitType
{
	NalUnknown = 0,
	NalSlice = 1,
	NalSlice_dpa = 2,
	NalSlice_dpb = 3,
	NalSlice_dpc = 4,
	NalSlice_idr = 5,
	NalSei = 6,
	NalSps = 7,
	NalPps = 8,
	NalAud = 9,
	NalEoSeq = 10,
	NalEoStream = 11,
	NalFill = 12,
	//MVC_EXTENSION_ENABLE
	NalPrefix = 14,
	NalSupSps = 15,
	NalSlcExt = 20,
	NalVDRD = 24,
};

static const char * const nal_unit_names[] = { "Unknow", "Slice", "Slice_DPA", "Slice_DPB", "Slice_DPC",
	"Slice_IDR", "SEI", "SPS", "PPS", "AUD","EoSeq", "EoStream", "Fill", "", "Prefix",
	"SupSps", "SlcExt", "VDRD", 0 };

enum	EMediaType
{
	eMedTypeNone,
	eMedTypeVideo,
	eMedTypeAudio,
	eMedTypeMidi,
	eMedTypeText
};
