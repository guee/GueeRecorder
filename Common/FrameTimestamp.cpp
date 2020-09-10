#include "FrameTimestamp.h"

void FrameTimestamp::onStart()
{
    if (m_status == sync_Stoped)
    {
        m_initPoint = std::chrono::steady_clock::now();
    }
    else if (m_status == sync_Paused)
    {
        m_initPoint += std::chrono::steady_clock::now() - m_pausePoint;
    }
    m_status = sync_Syncing;
}

void FrameTimestamp::onPause()
{
    if (m_status == sync_Stoped)
    {
        m_pausePoint = std::chrono::steady_clock::now();
        m_initPoint = m_pausePoint;
    }
    else if (m_status == sync_Syncing)
    {
        m_pausePoint = std::chrono::steady_clock::now();
    }
    m_status = sync_Paused;
}

void FrameTimestamp::onStop()
{
    m_status = sync_Stoped;
}
