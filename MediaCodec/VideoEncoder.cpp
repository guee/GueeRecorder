#include "VideoEncoder.h"
#include "MediaWriter.h"


GueeVideoEncoder::GueeVideoEncoder(QObject* parent)
    : QThread(parent)
{
	m_encodeing		= false;
	memset( &m_videoParams, 0, sizeof( m_videoParams ) );
    m_x264Handle	= nullptr;
	memset( &m_x264Param, 0, sizeof( m_x264Param ) );

    m_videoParams.encoder		= VE_X264;
    m_videoParams.profile		= VF_BaseLine;
    m_videoParams.presetX264	= VP_x264_VeryFast;
    m_videoParams.outputCSP 	= Vid_CSP_I420;
    m_videoParams.width         = 800;
    m_videoParams.height		= 600;
    m_videoParams.frameRate 	= 25;
    m_videoParams.refFrames		= -1;
    m_videoParams.gopMax		= -1;
    m_videoParams.gopMin		= -1;
    m_videoParams.vfr			= false;
    m_videoParams.annexb		= false;
    m_videoParams.bitrate		= 1000;
    m_videoParams.BFrames		= -1;
    m_videoParams.BFramePyramid	= 0;
}

GueeVideoEncoder::~GueeVideoEncoder()
{
    endEncode();
}

bool GueeVideoEncoder::bindStream( GueeMediaStream* stream )
{
	if ( m_encodeing ) return false;
    if ( nullptr == stream )
	{
        return false;
	}
	else
	{
        m_mediaStream = stream;
	}
	return true;
}

bool GueeVideoEncoder::startEncode( const SVideoParams* videoParams )
{
	if ( m_encodeing ) return false;
	if ( videoParams )
	{
        if ( videoParams->height < 16 || videoParams->width < 16 ||
            videoParams->frameRate <= 0.0f || videoParams->frameRate > 240.0f ||
            videoParams->vbvBuffer < 0 || videoParams->bitrateMax < 0 ||
            videoParams->encoder < VE_X264 || videoParams->encoder > VE_INTEL ||
            videoParams->profile < VF_Auto || videoParams->profile > VF_High ||
            videoParams->rateMode < VR_AverageBitrate || videoParams->rateMode > VR_ConstantQP )
		{
			return false;
		}
		m_videoParams	= *videoParams;
	}
    switch( m_videoParams.encoder )
	{
	case VE_X264:
		if ( !set264Params() )
			return false;
		m_x264Handle	= x264_encoder_open( &m_x264Param );
        if ( nullptr == m_x264Handle )
			return false;
		x264_encoder_parameters( m_x264Handle, &m_x264Param );
		m_encodeing		= true;
        start();
		break;
	case VE_CUDA:
		break;
	case VE_NVENC:
		break;
	case VE_INTEL:
		break;
	}
    return m_encodeing;
}


void GueeVideoEncoder::endEncode()
{
	if ( !m_encodeing ) return;
    switch( m_videoParams.encoder )
	{
	case VE_X264:
		m_encodeing	= false;
        m_waitPendQueue.wakeAll();
        m_waitIdlePool.wakeAll();
        if ( isRunning() )
		{
            wait();
		}
		if ( m_x264Handle )
		{
			x264_encoder_close( m_x264Handle );
            m_x264Handle	= nullptr;
		}
		break;
	case VE_CUDA:
		break;
	case VE_NVENC:
		break;
	case VE_INTEL:
		break;
	}
}

bool GueeVideoEncoder::putFrame( int64_t millisecond, const uint8_t* buf, int32_t pitch )
{
	if ( !m_encodeing ) return false;
    switch( m_videoParams.encoder )
	{
	case VE_X264:
        return putFrameX264( millisecond, buf, pitch );
	case VE_CUDA:
		break;
	case VE_NVENC:
		break;
	case VE_INTEL:
		break;
	}
	return false;
}

bool GueeVideoEncoder::putFrameX264( int64_t millisecond, const uint8_t* buf, int32_t pitch )
{

	if ( !m_encodeing ) return false;
    x264_picture_t* picin = popCachePool();
    if (picin == nullptr) return false;
    picin->i_pts = millisecond * m_x264Param.i_timebase_num / m_x264Param.i_timebase_den / 1000;

    for(int plan = 0; plan < m_csp_tab.planes; ++plan)
    {
        int cpsize = 0;
        int ch = m_x264Param.i_height / (plan == 0 ? 1 : m_csp_tab.heightFix);
        int cw = pitch / (plan == 0 ? 1 : m_csp_tab.widthFix[plan]);
        if (picin->img.i_stride[plan] == cw)
        {
            cpsize = cw * ch;
            memcpy(picin->img.plane[plan], buf, static_cast<ulong>(cpsize));
            buf += cpsize;
        }
        else
        {
            cpsize = min(picin->img.i_stride[plan], cw);
            for (int h=0; h < ch; ++h)
            {
                memcpy(picin->img.plane[plan] + h * picin->img.i_stride[0], buf, static_cast<ulong>(cpsize));
                buf += cpsize;
            }
        }
    }

    m_mtxPendQueue.lock();
    m_picPendQueue.push_back(picin);
    m_waitPendQueue.wakeAll();
    m_mtxPendQueue.unlock();

	return	true;
}

void GueeVideoEncoder::run()
{
	int32_t			ret		= 0;
    x264_nal_t*		nalBuf	= nullptr;
    int32_t			nalNum  = 0;
    x264_picture_t	picout;

    GueeMediaStream::H264Frame*   frame = reinterpret_cast<GueeMediaStream::H264Frame*>(malloc(sizeof(GueeMediaStream::H264Frame) + sizeof(GueeMediaStream::H264Frame::NAL) * 256));

    memset(&picout, 0, sizeof(picout));

    auto putNals = [&]{
        frame->pts = picout.i_pts;
        frame->dts = picout.i_dts;
        frame->nalCount = nalNum;
        frame->payload = ret;
        for (int i = 0; i < nalNum; ++i)
        {
            frame->nals[i].nalData = nalBuf[i].p_payload;
            frame->nals[i].nalSize = nalBuf[i].i_payload;
            frame->nals[i].nalType = NalUnitType(nalBuf[i].i_type);
        }
        return m_mediaStream->putVideoFrame(frame);
    };


	if ( !m_x264Param.b_repeat_headers )
	{
        ret	= x264_encoder_headers( m_x264Handle, &nalBuf, &nalNum );
		if ( ret > 0 )
		{
            putNals();
		}
	}
    x264_picture_t* picin = nullptr;
    while(m_encodeing)
    {
        m_mtxPendQueue.lock();
        m_waitPendQueue.wait(&m_mtxPendQueue);
        if (!m_picPendQueue.isEmpty())
        {
            picin = m_picPendQueue.front();
            m_picPendQueue.pop_front();
        }
        m_mtxPendQueue.unlock();
        if ( m_encodeing == false ) break;
        ret	= x264_encoder_encode( m_x264Handle, &nalBuf, &nalNum, picin, &picout );
        //ret = 0;
        //sleep(1);
        picout.i_pts = picin->i_pts;
        m_mtxIdlePool.lock();
        m_picIdlePool.push_back(picin);
        qDebug() << "push to idle Frame:" << m_picIdlePool.size();
        m_waitIdlePool.wakeAll();
        m_mtxIdlePool.unlock();

		if ( ret < 0 )
		{
			//编码出错
            break;
		}
		else if ( ret == 0 )
		{
			//编码成功,但被缓存了
		}
		else
		{
            putNals();
		}
	}
    //int iMaxDelayedFrames	= x264_encoder_maximum_delayed_frames( m_x264Handle );
	int	iDelayedFrames		= x264_encoder_delayed_frames( m_x264Handle );
	while ( iDelayedFrames )
	{
        ret	= x264_encoder_encode( m_x264Handle, &nalBuf, &nalNum, nullptr, &picout );
		if ( ret > 0 )
		{
            putNals();
		}
		iDelayedFrames		= x264_encoder_delayed_frames( m_x264Handle );
	}

}

x264_picture_t* GueeVideoEncoder::popCachePool()
{
    x264_picture_t* picin = nullptr;
    m_mtxIdlePool.lock();
    if (!m_picIdlePool.empty())
    {
        picin = m_picIdlePool.last();
        m_picIdlePool.pop_back();
    }
    m_mtxIdlePool.unlock();
    if (picin == nullptr)
    {
        if (m_picPendQueue.count() < m_maxPendQueue)
        {
            picin = new x264_picture_t;
            x264_picture_alloc(picin, m_x264Param.i_csp, m_x264Param.i_width, m_x264Param.i_height);
        }
        else if(m_videoParams.onlineMode)
        {
            qDebug() << "Skip Frame";
            m_mtxPendQueue.lock();
            picin = m_picPendQueue.first();
            m_picPendQueue.pop_front();
            m_mtxPendQueue.unlock();
        }
        else
        {
            qDebug() << "Wait Frame begin";
            m_mtxIdlePool.lock();
            m_waitIdlePool.wait(&m_mtxIdlePool);
            if ( m_encodeing == false )
            {
                m_mtxIdlePool.unlock();
                return nullptr;
            }
            picin = m_picIdlePool.last();
            qDebug() << "Waited idle Frame:" << m_picIdlePool.size();
            m_picIdlePool.pop_back();

        }
    }

    return picin;
}

const GueeVideoEncoder::s_csp_tab& GueeVideoEncoder::getCspTable( EVideoCSP eFormat )
{
	static const s_csp_tab csp_tab[] =
	{
		{ 3, { 1, 2, 2 }, 2	},	//[Vid_CSP_I420]
		{ 3, { 1, 2, 2 }, 2	},	//[Vid_CSP_YV12]
        { 2, { 1, 1, 0 }, 2	},	//[Vid_CSP_NV12]
        { 2, { 1, 1, 0 }, 2	},	//[Vid_CSP_NV21]
		{ 3, { 1, 2, 2 }, 1	},	//[Vid_CSP_I422]

		{ 3, { 1, 2, 2 }, 1	},	//[Vid_CSP_YV16]
        { 2, { 1, 1, 0 }, 1	},	//[Vid_CSP_NV16]
        { 1, { 2, 0, 0 }, 1	},	//[Vid_CSP_YUY2]
        { 1, { 2, 0, 0 }, 1	},	//[Vid_CSP_UYVY]

        { 1, { 4, 0, 0 }, 1	},	//[Vid_CSP_V210]

		{ 3, { 1, 1, 1 }, 1	},	//[Vid_CSP_I444]
		{ 3, { 1, 1, 1 }, 1	},	//[Vid_CSP_YV24]
        { 1, { 3, 0, 0 }, 1	},	//[Vid_CSP_BGR]
        { 1, { 4, 0, 0 }, 1	},	//[Vid_CSP_BGRA]
        { 1, { 3, 0, 0 }, 1	}	//[Vid_CSP_RGB]
	};
	return csp_tab[eFormat];
}

int32_t GueeVideoEncoder::EVideoCSP_To_x264CSP( EVideoCSP eFormat )
{
	switch( eFormat )
	{
	case Vid_CSP_I420:	return X264_CSP_I420;
	case Vid_CSP_YV12:	return X264_CSP_YV12;
	case Vid_CSP_NV12:	return X264_CSP_NV12;
	case Vid_CSP_NV21:	return X264_CSP_NV21;
	case Vid_CSP_I422:	return X264_CSP_I422;

	case Vid_CSP_YV16:	return X264_CSP_YV16;
	case Vid_CSP_NV16:	return X264_CSP_NV16;
    case Vid_CSP_YUY2:	return X264_CSP_YUYV;
    case Vid_CSP_UYVY:	return X264_CSP_UYVY;

	case Vid_CSP_V210:	return X264_CSP_V210;

	case Vid_CSP_I444:	return X264_CSP_I444;
	case Vid_CSP_YV24:	return X264_CSP_YV24;

	case Vid_CSP_BGR:	return X264_CSP_BGR;
    case Vid_CSP_BGRA:	return X264_CSP_BGRA;

    case Vid_CSP_RGB:	return X264_CSP_RGB;
    }
	return X264_CSP_NONE;
}

bool GueeVideoEncoder::set264Params()
{
    if ( !set264BaseParams()
         || !set264FrameParams()
         || !set264BitrateParams()
         || !set264NalHrdParams()
         || !set264AnalyserParams()
         || !set264StreamParams()
         || !set264OtherParams()
         )
	{
		return false;
	}
	return true;
}

bool GueeVideoEncoder::set264BaseParams()
{
	int		iRet	= 0;
	//x264_param_default_preset 内部会调用 x264_param_default，因此必须在其它参数设置之前调用它。
	memset( &m_x264Param, 0, sizeof( m_x264Param ) );
	string	tuneString;

    if ( m_videoParams.psyTune > 0 )
	{
        tuneString	= x264_tune_names[ m_videoParams.psyTune - 1 ];
	}
    if ( m_videoParams.fastDecode )
	{
		if ( !tuneString.empty() ) tuneString	+= "+";
		tuneString	+= x264_tune_names[6];
	}
    if ( m_videoParams.onlineMode )
	{
		if ( !tuneString.empty() ) tuneString	+= "+";
		tuneString	+= x264_tune_names[7];
        m_maxPendQueue = 2;
    }
    else
    {
        m_maxPendQueue = static_cast<int>(m_videoParams.frameRate + 1);
        m_maxPendQueue = std::min(std::max(m_maxPendQueue, 5), 30);
    }
    iRet	= x264_param_default_preset( &m_x264Param, x264_preset_names[m_videoParams.presetX264], tuneString.c_str() );
	if ( 0 != iRet )
		return false;
    switch ( m_videoParams.profile )
	{
	case VF_Auto:
		break;
	case VF_BaseLine:
		iRet	= x264_param_apply_profile( &m_x264Param, x264_profile_names[0] );
		break;
	case VF_Main:
		iRet	= x264_param_apply_profile( &m_x264Param, x264_profile_names[1] );
		break;
	case VF_High:
		iRet	= x264_param_apply_profile( &m_x264Param, x264_profile_names[2] );
		break;
	}
	if ( 0 != iRet )
		return false;
    m_x264Param.i_threads = m_videoParams.threadNum;	//设置为 1 是单线程，0 是自动线程数。

    if ( m_videoParams.onlineMode )
    {
        //是否使用线程来分片			/* Whether to use slice-based threading. */
        //								//当 i_threads 不为1，但 b_sliced_threads 为 0 时，输入的帧数要达到 i_threads 帧，才会有编码数据输出。
        //								//因此，要么 i_threads 设置为1，要么给 b_sliced_threads 也设置值，否则在实时的流编码时，会有延迟。
        //多个线程编码时，同时使用分片线程，可以降低内存消耗，并对编码时CPU消耗也有一定程度的降低。
        m_x264Param.b_sliced_threads	= 1;
    }
    else
    {
        m_x264Param.b_sliced_threads	= 0;
    }
    m_x264Param.i_lookahead_threads = 1;//多个线程进行预测分析			/* multiple threads for lookahead analysis */
//    m_x264Param.b_deterministic;	//是否使用线程进行未确定的优化	/* whether to allow non-deterministic optimizations when threaded */
//	m_x264Param.b_cpu_independent;	//强制规范行为，而不是依赖于 cpu 的优化算法	/* force canonical behavior rather than cpu-dependent optimal algorithms */
//	m_x264Param.i_sync_lookahead;	//同步预测缓冲区大小			/* threaded lookahead buffer */

    //视频属性		/* Video Properties */
    m_csp_tab = getCspTable(m_videoParams.outputCSP);
    m_x264Param.i_csp		= EVideoCSP_To_x264CSP(m_videoParams.outputCSP);	//编码的比特流的色彩空间		/* CSP of encoded bitstream */
    //（看x264源码得知）x264 要求必须宽高必须是 YUV 宏像素的整数倍。
    int alignW = m_csp_tab.planes > 1 ? m_csp_tab.widthFix[1] : 1;
    int alignH = m_csp_tab.heightFix;
    m_x264Param.i_width	= (m_videoParams.width + alignW - 1) / alignW * alignW;
    m_x264Param.i_height = (m_videoParams.height + alignH -1) / alignH * alignH;

	//裁剪矩形参数: 添加到那些隐式定义的非 mod16 的视频分辨率。
	/* Cropping Rectangle parameters: added to those implicitly defined by
       non-mod16 video resolutions. */
	//（看x264源码得知）裁剪的宽高也要求是宏像素的整数倍，所以这里不需要设置了，YUV420无法编码出奇数像素宽高的视频。
	//m_x264Param.crop_rect.i_left	= 0;
	//m_x264Param.crop_rect.i_top		= 0;
	//m_x264Param.crop_rect.i_right	= 2;
	//m_x264Param.crop_rect.i_bottom	= 2;
    m_x264Param.i_fps_num = static_cast<uint32_t>(m_videoParams.frameRate * 10000.0f); //帧率的分子
    m_x264Param.i_fps_den = 10000;	//帧率的分母
    int mcd = maximumCommonDivisor(m_x264Param.i_fps_num, m_x264Param.i_fps_den);
    m_x264Param.i_fps_num /= mcd;
    m_x264Param.i_fps_den /= mcd;

	////日志
	//m_x264Param.pf_log;				//日志回调函数
	//m_x264Param.p_log_private;		//
#ifdef	_DEBUG
	m_x264Param.i_log_level	= X264_LOG_DEBUG;			//日志级别
#else
	m_x264Param.i_log_level	= X264_LOG_NONE;			//日志级别
#endif
	//m_x264Param.b_full_recon;		//是否显示日志
	//m_x264Param.psz_dump_yuv;		//重建帧的文件名

	return true;
}

bool GueeVideoEncoder::set264FrameParams()
{
	//比特流参数	/* Bitstream parameters */

	//参考帧的最大数量				/* Maximum number of reference frames */
    if ( m_videoParams.refFrames > 0 ) m_x264Param.i_frame_reference = m_videoParams.refFrames;
    //m_x264Param.i_dpb_size;
    /* Force a DPB size larger than that implied by B-frames and reference frames.
										 * Useful in combination with interactive error resilience. */
	//IDR关键帧最大间隔。			/* Force an IDR keyframe at this interval */
    if ( m_videoParams.gopMax > 0 )
	{
        m_x264Param.i_keyint_max	= m_videoParams.gopMax;
	}
	//场景切换时，与前一个IDR帧间隔小于此值时，编码为 I 帧，而不是 IDR	/* Scenecuts closer together than this are coded as I, not IDR. */
    if ( m_videoParams.gopMin > 0 && m_videoParams.gopMin < m_videoParams.gopMax )
	{
        m_x264Param.i_keyint_min	= m_videoParams.gopMin;
	}
	else if ( m_x264Param.i_keyint_min >= m_x264Param.i_keyint_max )
	{
		m_x264Param.i_keyint_min	= m_x264Param.i_keyint_max / 2;
	}
	//m_x264Param.i_scenecut_threshold;//画面变化超出此阈值时插入 I 帧。		/* how aggressively to insert extra I frames */
	//m_x264Param.b_intra_refresh;		//是否使用周期性的帧内刷新代替IDR帧。		/* Whether or not to use periodic intra refresh instead of IDR frames. */

	//两个P 帧之间 B 帧的数量。	/* how many b-frame between 2 references pictures */
    if ( m_videoParams.onlineMode )
	{
		m_x264Param.i_bframe			= 0;
		m_x264Param.i_bframe_pyramid	= 0;
	}
    else if ( m_videoParams.BFrames >= 0 && m_videoParams.BFrames < m_x264Param.i_keyint_max )
	{
        m_x264Param.i_bframe		= m_videoParams.BFrames;
	//m_x264Param.i_bframe_adaptive;	//自适应 B 帧判定, 可选取值：X264_B_ADAPT_FAST 等
	//m_x264Param.i_bframe_bias;		//控制B帧替代 P 帧的概率，范围-100 ~ +100，该值越高越容易插入B帧，默认 0.
	////允许部分B帧为参考帧			/* Keep some B-frames as references: 0=off, 1=strict hierarchical, 2=normal */
        m_x264Param.i_bframe_pyramid	= m_videoParams.BFramePyramid;
	}
	//m_x264Param.b_open_gop;			//Close GOP是指帧间的预测都是在GOP中进行的。使用Open GOP，后一个GOP会参考前一个GOP的信息
	//m_x264Param.b_bluray_compat;		//是否支持蓝光碟
	//m_x264Param.i_avcintra_class;

	//m_x264Param.b_deblocking_filter;	//去块滤波器开关，alphac0 和 beta 是去块滤波器参数
	//m_x264Param.i_deblocking_filter_alphac0;		/* [-6, 6] -6 light filter, 6 strong */
	//m_x264Param.i_deblocking_filter_beta;		/* [-6, 6]  idem */

	//m_x264Param.b_cabac;				//自适应算术编码 cabac 开关
	//m_x264Param.i_cabac_init_idc;	//给出算术编码初始化时表格的选择

	//m_x264Param.b_interlaced;		//隔行扫描
	//m_x264Param.b_constrained_intra;

	//m_x264Param.i_level_idc;		//编码复杂度，算术编码器 IDC 等级
	//m_x264Param.i_frame_total;		//需要编码的总帧数，0表示未知	/* number of frames to encode if known, else 0 */
	////量化
	//m_x264Param.i_cqm_preset;		//自定义量化矩阵(CQM), 初始化量化模式为flat
	//m_x264Param.psz_cqm_file;		//读取 JM 格式的外部量化矩阵文件，忽略其它 cqm 选项。	/* filename (in UTF-8) of CQM file, JM format */
	//m_x264Param.cqm_4iy;
	//m_x264Param.cqm_4py;
	//m_x264Param.cqm_4ic;
	//m_x264Param.cqm_4pc;
	//m_x264Param.cqm_8iy;
	//m_x264Param.cqm_8py;
	//m_x264Param.cqm_8ic;
	//m_x264Param.cqm_8pc;
	return true;
}

bool GueeVideoEncoder::set264BitrateParams()
{
	//码率控制参数		/* Rate control parameters */
    if ( m_videoParams.bitrate ) m_x264Param.rc.i_bitrate	= m_videoParams.bitrate;
    switch( m_videoParams.rateMode )
	{
	case VR_AverageBitrate:
		m_x264Param.rc.i_rc_method	= X264_RC_ABR;
		m_x264Param.rc.i_vbv_max_bitrate	= 0;
		m_x264Param.rc.i_vbv_buffer_size	= 0;
		break;
	case VR_VariableBitrate:
		m_x264Param.rc.i_rc_method	= X264_RC_ABR;
        m_x264Param.rc.i_vbv_max_bitrate	= m_videoParams.bitrateMax;
        m_x264Param.rc.i_vbv_buffer_size	= m_videoParams.vbvBuffer;
		break;
	case VR_ConstantBitrate:
		m_x264Param.rc.i_rc_method	= X264_RC_CRF;
        //m_x264Param.rc.i_vbv_max_bitrate	= m_videoParams.bitrate;
        //m_x264Param.rc.i_vbv_buffer_size	= m_videoParams.bitrate;
		break;
	case VR_ConstantQP:
		m_x264Param.rc.i_rc_method			= X264_RC_CQP;
        m_x264Param.rc.i_vbv_max_bitrate	= m_videoParams.bitrateMax;
        m_x264Param.rc.i_vbv_buffer_size	= m_videoParams.vbvBuffer;
		break;
	}
//	m_x264Param.rc.i_rc_method;		//码率控制方式：X264_RC_CQP恒定质量,X264_RC_CRF恒定码率,  X264_RC_ABR平均码率	/* X264_RC_* */
//	//指定量化品质，值越大质量越差。
//	m_x264Param.rc.i_qp_constant;	//指定P帧的量化值，0 - 51，0表示无损	/* 0 to (51 + 6*(x264_bit_depth-8)). 0=lossless */
//	m_x264Param.rc.i_qp_min;			//允许的最小量化值，默认10				/* min allowed QP value */
//	m_x264Param.rc.i_qp_max;			//允许的最大量化值，默认51				/* max allowed QP value */
//	m_x264Param.rc.i_qp_step;		//量化步长，即相邻两帧之间量化值之差的最大值	/* max QP step between frames */
//	m_x264Param.rc.i_bitrate;		//平均码率大小
//	m_x264Param.rc.f_rf_constant;	//1pass VBR, nominal QP. 实际质量，值越大图像越花,越小越清晰	/* 1pass VBR, nominal QP */
//	m_x264Param.rc.f_rf_constant_max;//最大码率因子，该选项仅在使用CRF并开启VBV时有效，图像质量的最大值，可能会导致VBV下溢。	/* In CRF mode, maximum CRF as caused by VBV */
//	m_x264Param.rc.f_rate_tolerance;	//允许的误差
	
	//如果设置vbv-maxrate则vbv-bufsize必须设置
	//一般设置为 vbv-maxrate = vbv-bufsize = a*bitrate。
	//a=0，不启用VBV机制，编码性能最好，适用于硬盘文件编码；但它输出的码率波动性大，有可能某些帧的比特数过高，不适用于有实际带宽限制的流媒体传输。
	//0<a<1，这样的设置没有什么意义，得不到任何的好处。
	//a=1，等于CBR，CBR是一种复杂和平滑场景都不大讨好的码率控制方法，一般不采用这一方法。
	//a>1，对每帧数据有限制，但又可以暂时超过平均码率，适用于流媒体传输。
	//对于某些特殊场景编码，例如电脑屏幕编码，它的特点是I帧纹理细节丰富，编码数据极大，P帧变化很小或者根本没变化，p帧数据很小。
	//这种情况，如果不设置vbv参数（保持缺省值0），I帧数据压不下来，码流会周期性的高低变动；有可能造成网络拥塞，数据包丢失，解码端花屏。
	//如果设置a=1，I帧得到限制，可以压下来，但编码质量下降太多，主观质量差。
	//因此需要根据网络设置成a>1，使得I帧可以暂时有限度的大于平均码率，而P帧编码时候还能把平均码率降下来。

//	m_x264Param.rc.i_vbv_max_bitrate;//平均码率模式下，最大瞬时码率，默认0
//								//单位是kbps。使用它等于限制了I帧最大的码率输出（一般都是I帧达到maxrate的限制，除非bitrate设置过低不合理）。
//								//而I帧质量的降低，会拉低整个视频序列的视频质量。因此只在真正有最大码率限制的情况下才去设定它。
//	m_x264Param.rc.i_vbv_buffer_size;//码率控制缓冲区的大小，单位kbit，默认0
//								//在VBR和ABR情况下，可以设置vbv-maxrate和vbv-bufsize
//								//（很多情况下设置为vbv-maxrate = vbv-bufsize = bitrate，注意vbv-bufsize的量纲是不同，即最大缓存1s的数据）。
//								//在RC_ABR码率控制方法下，如果vbv-maxrate == bitrate这时候其实进行的是CBR码率控制方法，
//								//encoder力争控制每一帧输出都稳定在bitrate上。



//	m_x264Param.rc.f_vbv_buffer_init;//设置码率控制缓冲区（VBV）缓冲达到多满(百分比)，才开始回放，范围0~1.0，默认0.9	/* <=1: fraction of buffer_size. >1: kbit */
//								//设置播放之前必须先载入多少码流到VBV缓冲中。如果值小于1，那么大小就为 vbv-init * vbv-bufsize。如果大于1，则是以kbits为单位的值。
//								//ABR时不要设置这个参数，可以完全忽略这个参数。

//	m_x264Param.rc.f_ip_factor;		//I 帧和 P 帧之间的量化因子（QP）比值，默认1.4
//	m_x264Param.rc.f_pb_factor;		//P 帧和 B 帧之间的量化因子（QP）比值，默认1.3

//    /* VBV filler: force CBR VBV and use filler bytes to ensure hard-CBR.
//     * Implied by NAL-HRD CBR. */
//	m_x264Param.rc.b_filler;

//	m_x264Param.rc.i_aq_mode;		//自适应量化（AQ）模式。 0：关闭AQ			/* psy adaptive QP. (X264_AQ_*) */

//	m_x264Param.rc.f_aq_strength;	//自适应量化强度。减少平坦区域块效应和纹理区域模糊效应的强度。
//								//强度越高，高频信息消减越多，应对平坦区域的块效益就越好，但是对于纹理区域的模糊就越大。
//								//一般来说画面动态较高就选低点的aq-strength（0.6-0.8），以免在平坦区域浪费太多码率，而损伤了动态区域；
//								//在静态画面较多的场景就选高点aq-strength（>=1.0），防止暗部因比特不够而产生色带。
//								//一般情况下如果aq-strength=0，强制aq-mode为Disabled。
//								//在MB-tree的码率控制下，如果aq-mode为Disabled，强制aq-mode为Variance AQ，aq-strength为0。

//	m_x264Param.rc.b_mb_tree;		//是否开启基于 macroblock 的qp控制方法		/* Macroblock-tree ratecontrol. */
    m_x264Param.rc.i_lookahead = 10;		//决定 mbtree 向前预测的帧数

//	/* 2pass */
//	m_x264Param.rc.b_stat_write;		//是否将统计数据写入到文件psz_stat_out中	/* Enable stat writing in psz_stat_out */
//	m_x264Param.rc.psz_stat_out;		//输出文件用于保存第一次编码统计数据		/* output filename (in UTF-8) of the 2pass stats file */
//	m_x264Param.rc.b_stat_read;		//是否从文件psz_stat_in中读入统计数据		/* Read stat from psz_stat_in and use it */
//	m_x264Param.rc.psz_stat_in;		//输入文件存有第一次编码的统计数据			/* input filename (in UTF-8) of the 2pass stats file */

//	/* 2pass params (same as ffmpeg ones) */
//	m_x264Param.rc.f_qcompress;		//量化曲线(quantizer curve)压缩因子。0.0 => 恒定比特率，1.0 => 恒定量化值。		/* 0.0 => cbr, 1.0 => constant qp */
//	m_x264Param.rc.f_qblur;			//时间上模糊量化，减少QP的波动(after curve compression)		/* temporally blur quants */
//	m_x264Param.rc.f_complexity_blur;//时间上模糊复杂性，减少QP的波动(before curve compression)	/* temporally blur complexity */
//	m_x264Param.rc.zones;			//码率控制覆盖								/* ratecontrol overrides */
//	m_x264Param.rc.i_zones;			/* number of zone_t's */
//	m_x264Param.rc.psz_zones;		// 指定区的另一种方法						/* alternate method of specifying zones */
	return true;
}

bool GueeVideoEncoder::set264NalHrdParams()
{
	/* NAL HRD
	* Uses Buffering and Picture Timing SEIs to signal HRD
	* The HRD in H.264 was not designed with VFR in mind.
	* It is therefore not recommendeded to use NAL HRD with VFR.
	* Furthermore, reconfiguring the VBV (via x264_encoder_reconfig)
	* will currently generate invalid HRD. */
//	m_x264Param.i_nal_hrd;

//	//Vui参数集视频可用性信息视频标准化选项
//	//视频的宽高比，值为 0 < x <= 65535。 /* they will be reduced to be 0 < x <= 65535 and prime */
//	m_x264Param.vui.i_sar_height;
//	m_x264Param.vui.i_sar_width;
//	m_x264Param.vui.i_overscan;		/* 0=undef, 1=no overscan, 2=overscan */
//	 /* see h264 annex E for the values of the following */
//	m_x264Param.vui.i_vidformat;		//视频原始类型，默认"undef"，component/pal/ntsc/secam/mac/undef
//	m_x264Param.vui.b_fullrange;		//样本亮度和色度的计算方式，默认"off"，可选项：off/on
//	m_x264Param.vui.i_colorprim;		//原始色度格式，默认"undef"，可选项：undef/bt709/bt470m/bt470bg，smpte170m/smpte240m/film
//	m_x264Param.vui.i_transfer;		//转换方式，默认"undef"，可选项：undef/bt709/bt470m/bt470bg/linear,log100/log316/smpte170m/smpte240m
//	m_x264Param.vui.i_colmatrix;		//色度矩阵设置，默认"undef",undef/bt709/fcc/bt470bg,smpte170m/smpte240m/GBR/YCgCo
//	m_x264Param.vui.i_chroma_loc;	//色度样本指定，范围0~5，默认0	/* both top & bottom */
	return true;
}

bool GueeVideoEncoder::set264AnalyserParams()
{
	//编码分析参数		/* Encoder analyser parameters */
//	m_x264Param.analyse.intra;		//帧内分区			/* intra partitions */
//	m_x264Param.analyse.inter;		//帧间分区			/* inter partitions */
//	m_x264Param.analyse.b_transform_8x8;
//	m_x264Param.analyse.i_weighted_pred;		//P 帧权重		/* weighting for P-frames */
//	m_x264Param.analyse.b_weighted_bipred;	//B 帧隐式加权	/* implicit weighting for B-frames */
//	m_x264Param.analyse.i_direct_mv_pred;	//时间空间运动向量预测模式	/* spatial vs temporal mv prediction */
//	m_x264Param.analyse.i_chroma_qp_offset;	//色度量化步长偏移量
//	m_x264Param.analyse.i_me_method;			//运动估计算法 (X264_ME_*)	/* motion estimation algorithm to use (X264_ME_*) */
//	m_x264Param.analyse.i_me_range;			//整像素运动估计搜索范围 (from predicted mv)	/* integer pixel motion estimation search range (from predicted mv) */
//	m_x264Param.analyse.i_mv_range;			//运动矢量最大长度. -1 = auto, based on level	/* maximum length of a mv (in pixels). -1 = auto, based on level */
//	m_x264Param.analyse.i_mv_range_thread;	//线程之间的最小运动向量缓冲.  -1 = auto, based on number of threads.	/* minimum space between threads. -1 = auto, based on number of threads. */
//	m_x264Param.analyse.i_subpel_refine;		//亚像素运动估计质量		/* subpixel motion estimation quality */
//	m_x264Param.analyse.b_chroma_me;			//亚像素色度运动估计和P帧的模式选择		/* chroma ME for subpel and mode decision in P-frames */
//	m_x264Param.analyse.b_mixed_references;	//允许每个宏块的分区有它自己的参考号	/* allow each mb partition to have its own reference number */
//	m_x264Param.analyse.i_trellis;			//Trellis量化提高效率，对每个8x8的块寻找合适的量化值，需要CABAC，	/* trellis RD quantization */
//										//0 ：即关闭  1：只在最后编码时使用  2：在所有模式决策上启用
//	m_x264Param.analyse.b_fast_pskip;		//快速P帧跳过检测			/* early SKIP detection on P-frames */
//	m_x264Param.analyse.b_dct_decimate;		//P帧变换系数阈值			/* transform coefficient thresholding on P-frames */
//	m_x264Param.analyse.i_noise_reduction;	//自适应伪盲区				/* adaptive pseudo-deadzone */
//	m_x264Param.analyse.f_psy_rd;			//Psy RD强度				/* Psy RD strength */
//	m_x264Param.analyse.f_psy_trellis;		//Psy Trellis强度			/* Psy trellis strength */
    m_x264Param.analyse.b_psy	= m_videoParams.psyTune != eTuneNone;				//Psy优化开关，可能会增强细节	/* Toggle all psy optimizations */

//	m_x264Param.analyse.b_mb_info	= 0;			/* Use input mb_info data in x264_picture_t */
//	m_x264Param.analyse.b_mb_info_update;	/* Update the values in mb_info according to the results of encoding. */

//	/* the deadzone size that will be used in luma quantization */
//	m_x264Param.analyse.i_luma_deadzone[2]; //亮度量化中使用的盲区大小，{ 帧间, 帧内 }	/* {inter, intra} */

//	m_x264Param.analyse.b_psnr;				//计算和打印PSNR信息		/* compute and print PSNR stats */
//	m_x264Param.analyse.b_ssim;				// 计算和打印SSIM信息		/* compute and print SSIM stats */
	return true;
}

bool GueeVideoEncoder::set264StreamParams()
{

	//帧打包填充标志	/* frame packing arrangement flag */
//	m_x264Param.i_frame_packing;

	/* Muxing parameters */
    m_x264Param.b_aud	= 0;				//生成访问单元分隔符	/* generate access unit delimiters */
	m_x264Param.b_repeat_headers	= 0;	/* put SPS/PPS before each keyframe */
    m_x264Param.b_annexb	= m_videoParams.annexb;	//值为true，则NALU之前是4字节前缀码0x00000001
								//设置为0，包的前 4 字节为数据长度，可以直接封装为 FLV/MP4
								//设置为1，包的前 3 字节或 4 字节为前缀码。00 00 00 01 或 00 00 01

								/* if set, place start codes (4 bytes) before NAL units, otherwise place size (4 bytes) before NAL units. */
//	m_x264Param.i_sps_id;			//sps和pps的id号		/* SPS and PPS id number */
    m_x264Param.b_vfr_input	= m_videoParams.vfr;	//VFR输入。1：可变帧率，时间基和时间戳用于码率控制  0：仅帧率用于码率控制
								/* VFR input.  If 1, use timebase and timestamps for ratecontrol purposes. If 0, use fps only. */
	//如果 b_vfr_input 和 b_pulldown 同时为 0(false)，x264 内部会重设时间基为帧率的倒数。
    m_x264Param.b_pulldown	= m_videoParams.onlineMode;		/* use explicity set timebase for CFR */

//	if ( m_videoParams.bVfr && m_videoParams.isOnlineMode )
//	{
//		m_x264Param.i_timebase_num = 1;
//		m_x264Param.i_timebase_den = 10000;
//	}
//	else
//	{
//		m_x264Param.i_timebase_num = m_x264Param.i_fps_den;		//时间基的分子	/* Timebase numerator */
//		m_x264Param.i_timebase_den = m_x264Param.i_fps_num;		//时间基的分母	/* Timebase denominator */
//	}
//	m_x264Param.b_tff;
	return true;
}

bool  GueeVideoEncoder::set264OtherParams()
{
    /* Pulldown:
	 * The correct pic_struct must be passed with each input frame.
	 * The input timebase should be the timebase corresponding to the output framerate. This should be constant.
	 * e.g. for 3:2 pulldown timebase should be 1001/30000
	 * The PTS passed with each frame must be the PTS of the frame after pulldown is applied.
	 * Frame doubling and tripling require b_vfr_input set to zero (see H.264 Table D-1)
	 *
	 * Pulldown changes are not clearly defined in H.264. Therefore, it is the calling app's responsibility to manage this.
	 */
    //m_x264Param.b_pic_struct;

	/* Fake Interlaced.
	 *
	 * Used only when b_interlaced=0. Setting this flag makes it possible to flag the stream as PAFF interlaced yet
	 * encode all frames progessively. It is useful for encoding 25p and 30p Blu-Ray streams.
	 */

    //m_x264Param.b_fake_interlaced;

	/* Don't optimize header parameters based on video content, e.g. ensure that splitting an input video, compressing
	 * each part, and stitching them back together will result in identical SPS/PPS. This is necessary for stitching
	 * with container formats that don't allow multiple SPS/PPS. */
    //m_x264Param.b_stitchable;

    //m_x264Param.b_opencl;            /* use OpenCL when available */
    //m_x264Param.i_opencl_device;     /* specify count of GPU devices to skip, for CLI users */
    //m_x264Param.opencl_device_id;  /* pass explicit cl_device_id as void*, for API users */
    //m_x264Param.psz_clbin_file;    /* filename (in UTF-8) of the compiled OpenCL kernel cache file */

    /* Slicing parameters */
    //m_x264Param.i_slice_max_size;    /* Max size per slice in bytes; includes estimated NAL overhead. */
    //m_x264Param.i_slice_max_mbs;     /* Max number of MBs per slice; overrides i_slice_count. */
    //m_x264Param.i_slice_min_mbs;     /* Min number of MBs per slice */
    //m_x264Param.i_slice_count;       /* Number of slices per frame: forces rectangular slices. */
    //m_x264Param.i_slice_count_max;   /* Absolute cap on slices per frame; stops applying slice-max-size
                                /* and slice-max-mbs if this is reached. */
	return true;
}

int GueeVideoEncoder::maximumCommonDivisor(int num, int den)
{
    while(true)
    {
        int c = num % den;
        if (!c) break;
        num = den;
        den = c;
    }
    return den;
}
