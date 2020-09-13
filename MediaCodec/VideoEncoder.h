#pragma once

#include "x264.h"
#include <QtCore>
#include <QThread>
#include "MediaStream.h"
#include "./Common/FrameRateCalc.h"

using namespace std;

class GueeMediaWriter;
class GueeVideoEncoder : public QThread
{
    Q_OBJECT
public:
    GueeVideoEncoder(QObject* parent = nullptr);
    ~GueeVideoEncoder();
    bool bindStream( GueeMediaStream* stream );
	bool startEncode( const SVideoParams* videoParams );
	void endEncode();
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

    int64_t m_prevFrameTime = 0;
    int64_t m_prevFramePts = 0;

    FrameRateCalc               m_encodeFPS;
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

    bool putFrameX264(int64_t microsecond, uint8_t* const plane[3], int32_t* pitch);

    x264_picture_t*	popCachePool();
    static inline int maximumCommonDivisor(int num, int den);

    void run();

};

