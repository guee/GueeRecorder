#pragma once

#include "x264.h"
#include <QtCore>
#include <QThread>
#include "MediaStream.h"
#include "./Common/FrameRateCalc.h"

using namespace std;
typedef void (*close_step_progress)(void* param);
class GueeMediaWriter;
class GueeVideoEncoder : public QThread
{
    Q_OBJECT
public:
    GueeVideoEncoder(QObject* parent = nullptr);
    ~GueeVideoEncoder();
    bool bindStream( GueeMediaStream* stream );
	bool startEncode( const SVideoParams* videoParams );
    void endEncode(close_step_progress fun, void* param);
    const SVideoParams* getParams() { return &m_videoParams; }
    bool putFrame( int64_t microsecond, const uint8_t* buf, int32_t pitch);
    bool putFrame( int64_t microsecond, uint8_t* const plane[3], int32_t* pitch);
    float encodeFps() const { return m_encodeFPS.fps(); }
private:

    struct	s_csp_tab
    {
        int32_t	planes;
        int32_t	widthFix[3];
        int32_t	heightFix;
    };

    //bool						m_encodeing;
	SVideoParams				m_videoParams;
    GueeMediaStream*            m_mediaStream = nullptr;
    s_csp_tab                   m_csp_tab;

	x264_t*						m_x264Handle;
	x264_param_t				m_x264Param;

    QSemaphore                  m_waitPendQueue;
    QMutex                      m_mtxPendQueue;
    QVector<x264_picture_t*>	m_picPendQueue;
    int32_t                     m_maxPendQueue = 0;

    QSemaphore                  m_waitIdlePool;
    QMutex                      m_mtxIdlePool;
    QVector<x264_picture_t*>	m_picIdlePool;

    typedef x264_t* (*p_x264_encoder_open)( x264_param_t * );
    typedef int (*p_x264_encoder_reconfig)( x264_t *, x264_param_t * );
    typedef void (*p_x264_encoder_parameters)( x264_t *, x264_param_t * );
    typedef int (*p_x264_encoder_headers)( x264_t *, x264_nal_t **, int * );
    typedef int (*p_x264_encoder_encode)( x264_t *, x264_nal_t **, int *, x264_picture_t*, x264_picture_t* );
    typedef void (*p_x264_encoder_close)( x264_t * );
    typedef int (*p_x264_encoder_delayed_frames)( x264_t * );
    typedef int (*p_x264_encoder_maximum_delayed_frames)( x264_t * );
    typedef void (*p_x264_encoder_intra_refresh)( x264_t * );
    typedef int (*p_x264_encoder_invalidate_reference)( x264_t *, int64_t );

    typedef void (*p_x264_picture_init)( x264_picture_t * );
    typedef int (*p_x264_picture_alloc)( x264_picture_t *, int, int, int );
    typedef void (*p_x264_picture_clean)( x264_picture_t * );

    typedef int (*p_x264_param_default_preset)( x264_param_t *, const char *, const char * );
    typedef void (*p_x264_param_apply_fastfirstpass)( x264_param_t * );
    typedef int (*p_x264_param_apply_profile)( x264_param_t *, const char * );

    p_x264_encoder_open m_x264_encoder_open = nullptr;
    p_x264_encoder_reconfig m_x264_encoder_reconfig = nullptr;
    p_x264_encoder_parameters m_x264_encoder_parameters = nullptr;
    p_x264_encoder_headers m_x264_encoder_headers = nullptr;
    p_x264_encoder_encode m_x264_encoder_encode = nullptr;
    p_x264_encoder_close m_x264_encoder_close = nullptr;
    p_x264_encoder_delayed_frames m_x264_encoder_delayed_frames = nullptr;
    p_x264_encoder_maximum_delayed_frames m_x264_encoder_maximum_delayed_frames = nullptr;
    p_x264_encoder_intra_refresh m_x264_encoder_intra_refresh = nullptr;
    p_x264_encoder_invalidate_reference m_x264_encoder_invalidate_reference = nullptr;

    p_x264_picture_init m_x264_picture_init = nullptr;
    p_x264_picture_alloc m_x264_picture_alloc = nullptr;
    p_x264_picture_clean m_x264_picture_clean = nullptr;

    p_x264_param_default_preset m_x264_param_default_preset = nullptr;
    p_x264_param_apply_fastfirstpass m_x264_param_apply_fastfirstpass = nullptr;
    p_x264_param_apply_profile m_x264_param_apply_profile = nullptr;

    QLibrary        m_libX264;

    int64_t m_prevFrameTime = 0;
    int64_t m_prevFramePts = 0;

    FrameRateCalc               m_encodeFPS;

	inline const s_csp_tab& getCspTable( EVideoCSP eFormat );
	inline int32_t EVideoCSP_To_x264CSP( EVideoCSP eFormat );

	bool set264Params();
	bool set264BaseParams();
	bool set264FrameParams();
	bool set264BitrateParams();
	bool set264NalHrdParams();
	bool set264AnalyserParams();
	bool set264StreamParams();
	bool set264OtherParams();

    bool putFrameX264(int64_t microsecond, uint8_t* const plane[3], int32_t* pitch);

    x264_picture_t*	popCachePool();
    static inline int maximumCommonDivisor(int num, int den);

    void run();

};

