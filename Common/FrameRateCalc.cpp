#include "FrameRateCalc.h"
FrameRateCalc::FrameRateCalc()
{
    m_windowMS = 1000;
}

FrameRateCalc::~FrameRateCalc()
{
    if (m_frames)
    {
        free(m_frames);
        m_frames = nullptr;
    }
}

void FrameRateCalc::setWindowMs(int32_t windowMS)
{
    m_windowMS = uint32_t(std::max(1, std::min(1000*30, windowMS)));
}

float FrameRateCalc::fps() const
{
    if (m_count == 0) return 0.0f;
    float seconds;
    if (m_status == sync_Paused)
    {
        seconds = static_cast<float>(m_pointMS - m_beginMS) / 1000.0f;
    }
    else
    {
        uint32_t ms = uint32_t(elapsed_milli());
        seconds = static_cast<float>(std::max(m_pointMS, ms) - m_beginMS) / 1000.0f;
    }
    return static_cast<float>(m_count) / seconds;
}

float FrameRateCalc::add()
{
    if (m_status == sync_Paused)
    {
        float seconds = static_cast<float>(m_pointMS - m_beginMS) / 1000.0f;
        return static_cast<float>(m_count) / seconds;
    }
    if (m_count == m_size)
    {
        uint32_t* f = static_cast<uint32_t*>(malloc(static_cast<size_t>(m_size + 1024) * sizeof(uint32_t)));
        if (f)
        {
            if (m_frames)
            {
                int32_t s = (m_index + m_size - m_count) % m_size;
                for (int32_t i = 0; i < m_count; ++i)
                {
                    f[i] = m_frames[s];
                    s = (s + 1) % m_size;
                }
                m_index = m_count;
                free(m_frames);
            }
            m_frames = f;
            m_size += 1024;
        }
    }
    uint32_t  tim = uint32_t(elapsed_milli());
    m_frames[m_index] = tim;
    int32_t beg = (m_index + m_size - m_count) % m_size;
    if (m_count < m_size) ++m_count;
    m_index = ( m_index + 1 ) % m_size;
    while(m_count > 2)
    {
        int32_t nex = (beg + 1) % m_size;
        if (m_frames[beg] + m_windowMS >= tim)
        {
            break;
        }
        else if (m_frames[nex] + m_windowMS >= tim &&
                 (tim - m_windowMS) - m_frames[beg] <= m_frames[nex] - (tim - m_windowMS) )
        {
            break;
        }
        beg = nex;
        --m_count;
    }
    m_beginMS = m_frames[beg];
    m_pointMS = tim + ( m_count == 1 ? m_windowMS : ( tim - m_beginMS ) / static_cast<uint32_t>( m_count - 1 ) );
    float seconds = static_cast<float>(m_pointMS - m_beginMS) / 1000.0f;
    return static_cast<float>(m_count) / seconds;
}

void FrameRateCalc::onStart()
{
    if (m_status == sync_Stoped)
    {
        if (m_frames)
        {
            free(m_frames);
            m_frames = nullptr;
        }
        m_size = 0;
        m_count = 0;
        m_index = 0;
        m_beginMS = 0;
        m_pointMS = 0;
    }
    FrameTimestamp::onStart();
}
