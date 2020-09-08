#include "ButtonWithVolume.h"
#include <QPainter>

ButtonWithVolume::ButtonWithVolume(QWidget* parent)
    :QPushButton(parent)
{

}

void ButtonWithVolume::paintEvent(QPaintEvent *event)
{
    QPainter pnt(this);
    pnt.fillRect(rect(), QColor(0,0,0));
    pnt.fillRect(QRect(0, 0, width(), int32_t(m_amplitude * height())), QColor(255,0,0));
}

void ButtonWithVolume::updateAmplitude(qreal amp)
{
    m_amplitude = amp;
    update();
}
