#ifndef FRAMERATECALC_H
#define FRAMERATECALC_H
#include<chrono>

class FrameRateCalc
{
public:
    FrameRateCalc(){
        m_keepMS = 1000;
    }
    //开始计时
    //keepMS    计算帧率的时间窗口，单位为毫秒
    void start(uint32_t keepMS = 1000)
    {
        m_keepMS = std::min(uint32_t(1000*10), std::max(uint32_t(1), keepMS));
        m_initPoint = std::chrono::steady_clock::now();
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
    //从开始至今经过的毫秒数
    uint32_t elapsed() const
    {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_initPoint).count());
    }
    //取得用当前时间计算的帧率
    float fps() const
    {
        if (m_count == 0) return 0.0f;
        uint32_t ms = elapsed();
        float seconds = static_cast<float>(std::max(m_pointMS, ms) - m_beginMS) / 1000.0f;
        return static_cast<float>(m_count) / seconds;
    }
    //添加一帧，并返回帧率
    float add()
    {
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
        uint32_t  tim = elapsed();
        m_frames[m_index] = tim;
        int32_t beg = (m_index + m_size - m_count) % m_size;
        if (m_count < m_size) ++m_count;
        m_index = ( m_index + 1 ) % m_size;
        while(m_count > 2)
        {
            int32_t nex = (beg + 1) % m_size;
            if (m_frames[beg] + m_keepMS >= tim)
            {
                break;
            }
            else if (m_frames[nex] + m_keepMS >= tim &&
                     (tim - m_keepMS) - m_frames[beg] <= m_frames[nex] - (tim - m_keepMS) )
            {
                break;
            }
            beg = nex;
            --m_count;
        }
        m_beginMS = m_frames[beg];
        m_pointMS = tim + ( m_count == 1 ? m_keepMS : ( tim - m_beginMS ) / static_cast<uint32_t>( m_count - 1 ) );
        float seconds = static_cast<float>(m_pointMS - m_beginMS) / 1000.0f;
        return static_cast<float>(m_count) / seconds;
    }
private:
    uint32_t* m_frames = nullptr;
    int32_t m_size = 0;  //循环队列分配的长度
    int32_t m_count = 0; //循环队列中记录的帧数
    int32_t m_index = 0; //当前写入位置的索引
    uint32_t m_keepMS = 0;
    uint32_t m_beginMS = 0;
    uint32_t m_pointMS = 0;
    typedef	std::chrono::steady_clock::time_point	TimePoint;
    TimePoint	m_initPoint;	//初始的系统计时器数，用于计算已经经过的时间。
};

#endif // FRAMERATECALC_H
