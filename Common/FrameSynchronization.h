#ifndef FRAMESYNCHRONIZATION_H
#define FRAMESYNCHRONIZATION_H
#include<chrono>

class FrameSynchronization
{
public:
    FrameSynchronization();
    enum EStatus
    {
        sync_Stoped,
        sync_Syncing,
        sync_Paused,
    };

    //初始化，设置帧率及有效的时间范围，如果不设置时间范围，则时间会一直增长。
    //fps          设置每秒的帧数，必须大于0。如不设置，则默认为30。
    //startSecond  开始的时间，单位为秒，默认值0。如果参数设置小于0，则自动修正为0。
    //durationMS   持续时间，单位为秒，默认值0，表示永远持续。如果参数设置小于0，则自动修正为0。
    //autoRestart  表示在时间超过了持续时间后，是否又从头开始，默认 false 置为停止状态。
    bool init(float fps, double startSecond = 0.0, double durationSecond = 0.0, bool autoRestart = false);
    float fps() const {return float(m_fps);}
    //开始计时，以及在暂停计时后恢复计时
    void start();
    //把时间点设置为指定的秒
    //如果小于init中设置的起始时间，则参数重置为起始时间。
    //如果大于init中设置的结束（起始＋持续）时间，则参数重置为结束时间。如果未设置自动从头开始，则计时结束。
    void seek(double seekToSecond);
    //暂停计时
    void pause();
    //停止计时
    void stop();
    //取得当前的计时状态
    EStatus status() const { return m_status; }

    //以当前时间计算帧编号。检查当前时刻是否到达了新帧的起始时间点
    //使用方式为先调用本函数，如果返回值>=0才截图或绘制
    //如果尚未到达新帧的时间点，则返回-1，否则返回值是当前时间（微秒数）。
    //当到达了新帧时间点时，将会把新帧编号加1，再次调用时仍会检查是否到达新帧时间点。
    int64_t isNextFrame();

    //以当前时间计算帧编号。等待直到到达新帧的时间点才返回
    //使用方式为先截图或绘制，然后再调用本函数等待到达新帧的时间点。
    //compact   是否是紧凑模式，
    //          true：则将尽量减少丢帧，如果上一帧完成的时长大于一帧的时长，
    //          但超出部分未达到当前帧之后的一半帧时长时，将立即返回。
    //          false：总是到达当前帧的时间点才返回。
    //返回值是当前时间（微秒数），返回时新帧的编号会设置为当前帧编号。
    int64_t waitNextFrame(bool compact = true);
    //取得新帧的编号。
    int64_t newFrameIndex() const
    {
        return (m_durationFrame ? m_nextFrame % m_durationFrame : m_nextFrame) + m_beginFrame;
    }
    //取得自开始计时以来，经过的微秒数
    int64_t elapsed()
    {
        if (m_status == sync_Paused)
            return std::chrono::duration_cast<std::chrono::microseconds>(m_pausePoint - m_initPoint).count() + m_beginMicroS;
        else if (m_status == sync_Syncing)
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_initPoint).count() + m_beginMicroS;
        return -1;
    }
private:
    typedef	std::chrono::steady_clock::time_point	TimePoint;
    TimePoint	m_initPoint;	//初始的系统计时器数，用于计算已经经过的时间。
    TimePoint	m_pausePoint;	//暂停时的系统计时器数。
    EStatus     m_status;

    double      m_fps;
    int64_t     m_beginMicroS;
    double      m_durationTimer;
    int64_t     m_beginFrame;
    int64_t     m_durationFrame;
    bool        m_autoRestart;

    int64_t     m_frameCount;
    int64_t     m_nextFrame;

};

#endif // FRAMESYNCHRONIZATION_H
