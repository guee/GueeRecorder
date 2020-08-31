#ifndef	_ENCODE_PARAMS_HEADER
#define	_ENCODE_PARAMS_HEADER
//视频编码器的类型
enum EVideoEncoder
{
	VE_X264,				//x264 软件编码器
	VE_NVENC,				//nvidia NVENC 硬件编码器
    VE_AMD_VCE,
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

enum EVideoPreset_Amd
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
enum EBitrateMode
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
	Vid_CSP_AYUV,
	Vid_CSP_BGR,			//0x000d  /* packed rgb 24bits   */
	Vid_CSP_RGB,			//0x000b  /* packed bgr 24bits   */

	Vid_CSP_BGRA,			//0x000c  /* packed bgr 32bits   */	A8R8G8B8
	Vid_CSP_BGRA10,			//0x000c  /* packed bgr 32bits   */	A2R10G10B10
};

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
	EVideoEncoder	eEncoder;		//当前选择的编码器
	EVideoProfile	eProfile;	
	EVideoPreset_x264	ePresetX264;
	EVideoPreset_Nvenc	ePresetNvenc;
    EVideoPreset_Amd	ePresetAmd;
    EVideoPreset_Intel	ePresetIntel;
	EVideoCSP		eOutputCSP;		//输出视频的像素格式。当前只支持 I420(YUV420)
	EPsyTuneType	ePsyTune;		//
    int32_t			iWidth;			//视频的宽度
    int32_t			iHeight;		//视频的高度
    float			fFrameRate;     //视频的帧率，如果 bVfr 和 isOnlineMode 都为 false，时间基也会设置为帧率的倒数。
    bool			bVfr;			//可变帧率。如果设置为true，则当帧不连续时保持码率恒定，视频质量高。设置为false则输入帧数量少码率就低。
    bool			isOnlineMode;	//是否是在线模式，即输入的图像是否是实时采集的，输出的视频流也要尽量减少延迟。
								//设置 true 时会禁止编码 B 帧，输入帧数据后立即返回，如果编码速度过慢则丢弃帧。
								//设置 false 时不会丢弃帧，如果缓冲队列满，输入帧数据后等待缓冲队列有空位时才返回。
    bool			bAnnexb;		//设置为false，包的前 4 字节为数据长度;设置为true，包的前 3 字节或 4 字节为前缀码。00 00 00 01 或 00 00 01
    bool			bMaxSpeed;		//最大化速度。
    bool			bOptimizeStill;	//对图像静止不变的部分进行编码优化。
    bool			bFastDecode;	//使编码出的视频能够被快速解码。
    int32_t			iStretchMode;	//如果输入图像和视频分辨率不一致，自动缩放的模式。
                                    //	0:不缩放，1:如果大于则剪裁边缘，小于则居中，
                                    //  2:缩放到视频分辨率，3:保持长宽比缩放并居中，4:保持长宽比缩放填满视频，超出部分剪裁掉。
    EBitrateMode    eBitrateMode;	//码率控制方式
    int32_t			iBitrate;		//平均码率，如果为 0 就自动计算出一个码率。
    int32_t			iBitrateMax;	//最大码率
    int32_t			iBitrateMin;	//最小码率，默认为0，不使用最小码率设置。否则根据文件写入速度或网络上传速度自动重设码率。
    int32_t			iVbvBuffer;		//码率可变时，进行码率控制的缓存大小
    int32_t			iGopMax;		//最大关键帧间隔（小于0则使用默认值）
    int32_t			iGopMin;		//最小关键帧间隔（小于0则使用默认值）
    int32_t			iRefFrames;		//向前参考帧的最大数量。（小于0则使用默认值）
    int32_t			iBFrames;		//两个P 帧之间 B 帧的数量。（小于0则使用默认值）
    int32_t			iBFramePyramid;	//0：不使用 B 帧为参考帧，1或2：使用B帧为参考帧。
};


//音频编码器，
enum	EAudioEncoder
{
	Aud_Enc_PCM,
	Aud_Enc_AAC,
	Aud_Enc_Mp3,
};

enum EAudioSampleBits
{
	eSampleBit8i	= ( 1 << 8 ) | 8,
	eSampleBit16i	= ( 2 << 8 ) | 16,		//支持 AAC 编码输出
	eSampleBit24i	= ( 3 << 8 ) | 24,
	eSampleBit32i	= ( 4 << 8 ) | 32,		//支持 AAC 编码输出
	eSampleBit24In32i	= ( 5 << 8 ) | 32,
	eSampleBit32f	= ( 6 << 8 ) | 32,		//支持 AAC 编码输出
};

struct	SAudioFormat
{
	EAudioSampleBits	sampleFormat;
	uint32_t			samplesPerSec;	//音频的采样率
	uint32_t			channels;		//音频的声道数，0表示没有音频，1=单声道，2=双声道
	uint32_t			channelMask;	//如果声道数大于2，可以设置声道MASK, 参考微软WAVEFORMATEXTENSIBLE结构中dwChannelMask的说明。
};

struct	SAudioParams : public SAudioFormat
{
	EAudioEncoder	eEncoder;	//音频编码器
	bool		isOnlineMode;	//是否是在线模式，即输入的声音数据是否是实时采集的。
								//当为在线模式时，如果输入的声音采样数与实际时间有差异时，会自动修正声音数据，减少时间差异。
	bool		useADTS;
	uint32_t	encLevel;			//1,2,3,4 对应 aac 的 MAIN,LOW,SSR,LTP。其它值都使用LOW。
	int32_t		bitratePerChannel;	//音频编码的码率（kbps），如果为 小于等于 0 就使用编码器支持的最大码率 / ( abs(bitratePerChannel) + 1 )。
};

struct  SInputImage
{
    int64_t     timestamp;
    EVideoCSP   csp;
    int32_t     planeCount;
    int32_t     stride[4];
    int32_t     plane[4];
};

//typedef	struct	SEncStatusConnectFail
//{
//	DWORD		dwErrorCode;	//错误代码
//	DWORD		dwTryLaterMS;	//在多少毫秒后会尝试再次连接。如果值为 0xFFFFFFFF 表示不会再次尝试
//	DWORD		dwRetryCount;	//重试连接的次数。
//}*PSEncStatusConnectFail;

//typedef	struct	SEncStatusUploadBitrate
//{
//	INT64		iConnectUploadMSeconds;	//当前连接已经上传的视频时长（毫秒）
//	INT64		iTotalUploadMSeconds;	//累计上传的视频时长（毫秒）
//	DWORD		uCurrentUploadBitrate;	//当前上传比特率
//	DWORD		uAverageUploadBitrate;	//平均上传比特率（最近一次连接）
//}*PSEncStatusUploadBitrate;

//typedef	struct	SEncStatusDiscardPacks
//{
//	INT64		iDiscardStartMSeconds;	//丢弃帧的起始时间（毫秒）
//	INT64		iDiscardMSeconds;		//丢弃的时长（毫秒）
//	DWORD		uDiscardFrames;			//丢弃的帧数
//	DWORD		uAverageUploadBitrate;	//平均上传比特率（最近一次连接）
//}*PSEncStatusDiscardPacks;

//typedef	struct	SEncStatusEncodeFps
//{
//	INT64		iRecordTime;	//从开始录制到当前经过的时长（毫秒）。
//	INT64		iInpFrameNum;	//当前正在输入帧的编号。
//	INT64		iEncFrameNum;	//当前正在编码帧的编号。
//	INT64		iEncFrameCount;	//实际编码完成的帧数，不包括跳过的帧数。
//	float		fAvgEncodeFps;	//平均的编码帧率。
//	float		fCurEncodeFps;	//当前编码的帧率。
//	float		fAvgInputFps;	//平均的输入帧率。
//	float		fCurInputFps;	//当前输入的帧率。

//}*PSEncStatusEncodeFps;



////编码及保存、上传的状态，通过回调函数通知上层调用者。
//enum	IEncoder_ENotify
//{
//	eEncNotifySuccess,			//成功，通常不需要通知该值

//	eEncNotifyStarted,			//编码已经开始。

//	eEncNotifyStoped,			//编码已经停止。
//								//dwValue	= 错误代码（DWORD）。
//								//ptrValue	= 0。

//	eEncNotifyEncodeError,		//编码器错误，例如编码过程中编码器返回失败。
//								//dwValue	= 错误代码；
//								//ptrValue	= 0。

//	eEncNotifyEncodeFps,		//编码的帧率等信息，大约每秒计算并通知一次。
//								//dwValue	= 0；
//								//ptrValue	= PSEncStatusEncodeFps。

//	eEncNotifyWriteFileFail,	//写入文件失败。
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= 错误代码（DWORD）。

//	eEncNotifyDisconnected,		//上传时连接被断开。
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= 错误代码（DWORD）。

//	eEncNotifyReConnectStart,	//开始重新连接。
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= 重连计数（DWORD）从1开始。

//	eEncNotifyReConnectDone,	//重新连接成功。
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= 重连计数（DWORD）。
//								//回调返回后，重连计数器清0，下次断开重连时，计数器又从1开始。

//	eEncNotifyReConnectFail,	//重新连接失败
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= PSEncStatusConnectFail

//	eEncNotifyUploadBitrate,	//当前的上传速率，大约每秒计算并通知一次。
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= PSEncStatusUploadBitrate。

//	eEncNotifyDiscardPacks,		//因为上传缓慢，丢弃了部分编码后的视频数据。
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= PSEncStatusDiscardPacks。

//	eEncNotifyDiscardFrame,		//因为编码缓慢，丢弃了输入的一帧画面。
//								//dwValue	= 为丢弃的帧编号；
//								//ptrValue	= 0。

//	eEncNotifyResetBitrate,		//因为上传缓慢，自动重设了视频的输出码率
//								//dwValue	= 为重设后的码率；
//								//ptrValue	= 0。

//	eEncNotifyResetPreset,		//因为编码缓慢，自动重设了视频的编码质量(仅对于 x264 编码)
//								//dwValue	= 为重设后的 EVideoPreset；
//								//ptrValue	= 0。

//	eEncNotify_AcceditDone,		//授权验证完成。
//								//dwValue	= TRUE 或 FALSE
//								//ptrValue	= LPCWSTR 失败的文字说明。

//	eEncNotify_GetCloudFail,	//取得 Mix_RdCloud 云直播地址失败
//								//dwValue	= 当前输出文件的 Index；
//								//ptrValue	= LPCWSTR 失败的文字说明。

//	eEncNotify_Count
//};


////回调函数，通知上层调用者当前的编码状态
//typedef	VOID	( WINAPI* fnEncoderNotifyCB )( IEncoder_ENotify	eNotify,		//通知
//												DWORD			dwValue,		//值（如果状态没有值，则为0）
//												DWORD_PTR		ptrValue,		//
//												LPVOID			pCbParam	//用户参数，把调用者通过 GvStartEncode 函数设置的 pCbUserParam 原样返回。
//												);

#endif
