#ifndef FRAMETIMESTAMP_H
#define FRAMETIMESTAMP_H
#include<chrono>
#include <algorithm>

class FrameTimestamp
{
public:
    FrameTimestamp()
    {
        m_prefixTime = std::chrono::microseconds(0);
    }
    virtual ~FrameTimestamp()
    {

    }
    enum EStatus
    {
        sync_Stoped,
        sync_Syncing,
        sync_Paused,
    };
    //设置前置时间
    void  setPrefixTime(int64_t prefixMS)
    {
        m_prefixTime = std::chrono::milliseconds(prefixMS);
    }
    //开始计时，以及在暂停计时后恢复计时
    void start()
    {
        onStart();
    }
    void pause()
    {
        onPause();
    }
    void stop()
    {
        onStop();
    }

    EStatus status() const { return m_status; }

    //取得自开始计时以来，经过的微秒数
    int64_t elapsed() const
    {
        if (m_status == sync_Paused)
            return std::chrono::duration_cast<std::chrono::microseconds>(m_pausePoint - m_initPoint + m_prefixTime).count();
        else if (m_status == sync_Syncing)
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_initPoint + m_prefixTime).count();
        return -1;
    }
    //取得自开始计时以来，经过的毫秒数
    int64_t elapsed_milli() const
    {
        if (m_status == sync_Paused)
            return std::chrono::duration_cast<std::chrono::milliseconds>(m_pausePoint - m_initPoint + m_prefixTime).count();
        else if (m_status == sync_Syncing)
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_initPoint + m_prefixTime).count();
        return -1;
    }
    //取得自开始计时以来，经过的秒数
    int64_t elapsed_second() const
    {
        if (m_status == sync_Paused)
            return std::chrono::duration_cast<std::chrono::seconds>(m_pausePoint - m_initPoint + m_prefixTime).count();
        else if (m_status == sync_Syncing)
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_initPoint + m_prefixTime).count();
        return -1;
    }
    //取得自开始计时以来，经过的秒数
    double elapsed_second_float() const
    {
        if (m_status == sync_Paused)
            return std::chrono::duration_cast<std::chrono::duration<double>>(m_pausePoint - m_initPoint + m_prefixTime).count();
        else if (m_status == sync_Syncing)
            return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - m_initPoint + m_prefixTime).count();
        return -1;
    }
protected:
    typedef	std::chrono::steady_clock::time_point	TimePoint;

    std::chrono::microseconds   m_prefixTime;   //前置时间
    TimePoint	m_initPoint;	//初始的系统计时器数，用于计算已经经过的时间。
    TimePoint	m_pausePoint;	//暂停时的系统计时器数。
    EStatus     m_status = sync_Stoped;

    virtual void onStart();
    virtual void onPause();
    virtual void onStop();
};

#endif // FRAMETIMESTAMP_H
