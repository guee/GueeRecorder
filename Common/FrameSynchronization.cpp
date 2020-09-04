#include "FrameSynchronization.h"
using namespace std;
using namespace std::chrono;
FrameSynchronization::FrameSynchronization()
{
    m_status    = sync_Stoped;
    m_fps       = 30.0;
    m_beginMicroS = 0;
    m_beginFrame  = 0;
    m_durationFrame  = 0;
    m_autoRestart   = false;
    m_frameCount    = 0;
    m_nextFrame     = 0;
}

bool FrameSynchronization::init(float fps, double startSecond, double durationSecond, bool autoRestart)
{
    if (fps <= 0.0f) return false;
    startSecond = std::max(startSecond, 0.0);
    durationSecond = std::max(durationSecond, 0.0);
    m_status = sync_Stoped;
    m_fps = static_cast<double>(fps);
    m_beginMicroS = static_cast<int64_t>(std::round(startSecond * 1000000.0));
    m_durationTimer = durationSecond;
    m_beginFrame = static_cast<int64_t>(std::round(startSecond * m_fps));
    m_nextFrame = 0;
    m_autoRestart = autoRestart;
    m_frameCount = 0;
    if (durationSecond > 0.0)
    {
        m_durationFrame = static_cast<int64_t>(ceil(durationSecond * m_fps));
        m_durationFrame = std::max(int64_t(1), m_durationFrame);
    }
    else
    {
        m_durationFrame = 0;
    }
    return true;
}

void FrameSynchronization::start()
{
    if (m_status == sync_Stoped)
    {
        m_initPoint = steady_clock::now();
        m_nextFrame = 0;
    }
    else if (m_status == sync_Paused)
    {
        m_initPoint += ( steady_clock::now() - m_pausePoint );
    }
    m_status = sync_Syncing;
}

void FrameSynchronization::seek(double seekToSecond)
{
    TimePoint currPoint = steady_clock::now();
    seekToSecond = std::max(0.0, seekToSecond - m_beginMicroS / 1000000.0);
    if (m_durationFrame && seekToSecond >= m_durationTimer)
    {
        if (m_autoRestart)
        {
            m_initPoint = currPoint;
            m_nextFrame = 0;
            if (m_status == sync_Stoped || m_status == sync_Paused)
            {
                m_pausePoint = currPoint;
                m_status = sync_Paused;
            }
        }
        else
        {
            stop();
        }
        return;
    }
    int64_t seekMicro = static_cast<int64_t>(seekToSecond * 1000000.0);
    m_initPoint = currPoint - microseconds(seekMicro);
    m_nextFrame = static_cast<int64_t>(std::round(seekToSecond * m_fps));
    if (m_status == sync_Stoped || m_status == sync_Paused)
    {
        m_pausePoint = currPoint;
        m_status = sync_Paused;
    }
}

void FrameSynchronization::pause()
{
    if (m_status == sync_Stoped)
    {
        m_pausePoint = steady_clock::now();
        m_initPoint = m_pausePoint;
        m_nextFrame = 0;
    }
    else if (m_status == sync_Syncing)
    {
        m_pausePoint = steady_clock::now();
    }
    m_status = sync_Paused;
}

void FrameSynchronization::stop()
{
    m_status = sync_Stoped;
}

int64_t FrameSynchronization::isNextFrame()
{
    if (m_status != sync_Syncing) return -1;
    TimePoint currPoint = steady_clock::now();
    int64_t microSecond = duration_cast<microseconds>(currPoint - m_initPoint).count();
    int64_t frameIndex = static_cast<int64_t>( microSecond / 1000000.0 * m_fps );
    if (frameIndex < m_nextFrame)
    {
        return -1;
    }
    m_nextFrame = frameIndex + 1;

    if (m_durationFrame && frameIndex >= m_durationFrame)
    {
        if (m_autoRestart)
        {
            m_initPoint = currPoint;
            m_nextFrame = 0;
        }
        else
        {
            stop();
            return -1;
        }
    }
    ++m_frameCount;
    return microSecond + m_beginMicroS;
}

int64_t FrameSynchronization::waitNextFrame(bool compact)
{
    if (m_status != sync_Syncing) return -1;
    TimePoint currPoint = steady_clock::now();
    int64_t microSecond = duration_cast<microseconds>(currPoint - m_initPoint).count();
    double frameIndex = microSecond / 1000000.0 * m_fps;
    int64_t nearFrame = static_cast<int64_t>(frameIndex);

    if (m_durationFrame && nearFrame >= m_durationFrame)
    {
        if (m_autoRestart)
        {
            m_initPoint = currPoint;
            m_nextFrame = 0;
            ++m_frameCount;
            return microSecond + m_beginMicroS;
        }
        else
        {
            stop();
            return -1;
        }
    }
    if (m_nextFrame >= nearFrame)
    {
        ++m_nextFrame;
        std::this_thread::sleep_for(microseconds(static_cast<int64_t>(m_nextFrame / m_fps * 1000000.0) - microSecond));
    }
    else if (compact && round(frameIndex) == nearFrame )
    {
        m_nextFrame = nearFrame;
    }
    else
    {
        m_nextFrame = static_cast<int64_t>(ceil(frameIndex));
        std::this_thread::sleep_for(microseconds(static_cast<int64_t>(m_nextFrame / m_fps * 1000000.0) - microSecond));
    }
    ++m_frameCount;
    return duration_cast<microseconds>(steady_clock::now() - m_initPoint).count() + m_beginMicroS;
}
