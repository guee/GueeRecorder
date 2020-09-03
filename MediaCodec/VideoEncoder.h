#pragma once

#include "x264.h"
#include "VideoCodec.h"
#include <QtCore>
#include "MediaStream.h"

using namespace std;

class CMediaWriter;
class CVideoEncoder : public QThread
{
public:
    CVideoEncoder(QObject* parent = nullptr);
    ~CVideoEncoder();
    bool bindStream( CMediaStream* stream );
	bool startEncode( const SVideoParams* videoParams );
	void endEncode();
    const SVideoParams* getParams() { return &m_videoParams; }
    bool putFrame( int64_t millisecond, const uint8_t* buf, int32_t pitch);
private:

    struct	s_csp_tab
    {
        int32_t	planes;
        int32_t	widthFix[3];
        int32_t	heightFix;
    };

    bool						m_encodeing;
	SVideoParams				m_videoParams;
    CMediaStream*               m_mediaStream = nullptr;
    s_csp_tab                   m_csp_tab;

	x264_t*						m_x264Handle;
	x264_param_t				m_x264Param;

    QWaitCondition              m_waitPendQueue;
    QMutex                      m_mtxPendQueue;
    QVector<x264_picture_t*>	m_picPendQueue;
    int32_t                     m_maxPendQueue = 0;

    QWaitCondition              m_waitIdlePool;
    QMutex                      m_mtxIdlePool;
    QVector<x264_picture_t*>	m_picIdlePool;


	struct s_rect
	{
		int32_t		x, y, width, height;
		s_rect() { x=y=width=height=0; }
		s_rect(int32_t _x, int32_t _y, int32_t _width, int32_t _height)
		{
			x = _x; y = _y; width = _width; height = _height;
		}
	};


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

    bool putFrameX264(int64_t millisecond, const uint8_t* buf, int32_t pitch);



    x264_picture_t*	popCachePool();
    static inline int maximumCommonDivisor(int num, int den);

    void run();

};

