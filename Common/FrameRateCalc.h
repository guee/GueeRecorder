#ifndef FRAMERATECALC_H
#define FRAMERATECALC_H
#include <chrono>

#include "FrameTimestamp.h"

class FrameRateCalc : public FrameTimestamp
{
public:
    FrameRateCalc();
    ~FrameRateCalc();
    //设置计算帧率的时间窗口（毫秒，1～30000）
    void setWindowMs(int32_t windowMS);

    //取得用当前时间计算的帧率
    float fps() const;
    //添加一帧，并返回帧率
    float add();
private:
    uint32_t* m_frames = nullptr;
    int32_t m_size = 0;  //循环队列分配的长度
    int32_t m_count = 0; //循环队列中记录的帧数
    int32_t m_index = 0; //当前写入位置的索引
    uint32_t m_windowMS = 0;
    uint32_t m_beginMS = 0;
    uint32_t m_pointMS = 0;

    virtual void onStart();
};

#endif // FRAMERATECALC_H
