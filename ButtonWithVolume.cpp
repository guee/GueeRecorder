#include "ButtonWithVolume.h"
#include <QPainter>

ButtonWithVolume::ButtonWithVolume(QWidget* parent)
    :QPushButton(parent)
    ,m_imgWave(24,24, QImage::Format_ARGB32)
{
    m_imgWave.fill(Qt::GlobalColor::white);
    for (int y = 0; y < m_imgWave.height(); ++y)
    {
        uint8_t* pix = m_imgWave.scanLine(y);
        for ( int x=0; x < m_imgWave.width(); ++x)
        {
            pix[3] = 0;
            pix += 4;
        }
    }
}

void ButtonWithVolume::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter pnt(this);
   // pnt.eraseRect(rect());
    pnt.drawImage(rect(), m_imgWave, m_imgWave.rect());
    QFont fnt(this->font());
    fnt.setPixelSize(8);
    pnt.setFont(fnt);
    pnt.setPen(QColor(255,255,255, 160));
    pnt.drawText(QRect(0,0,width(), 12), Qt::AlignCenter, QString("%1%").arg(int(m_volume*100)));
}

void ButtonWithVolume::updateAmplitude(qreal amp)
{
    m_amplitude = amp;
    uint8_t* pix = m_imgWave.bits();
    uint8_t* las = pix + (m_imgWave.width() - 1) * 4;
    int siz = m_imgWave.bytesPerLine();
    int hei = qMax(1, int(amp * m_imgWave.height() / m_volume));
    int lmt = m_imgWave.height() - hei;
    //qDebug() << hei << m_imgWave.height();
    for (int y = 0; y < m_imgWave.height(); ++y)
    {
        memmove(pix, pix + 4, siz - 4);
        pix += siz;
        if (y >= lmt)
        {
            las[3] = 255 * (y - lmt) / hei + lmt;
        }
        else
        {
            las[3] = 0;
        }
        //las[0] = las[1] = las[2] = las[3] = 255;
        las += siz;
    }
    update();
}

void ButtonWithVolume::updateVolume(qreal volume)
{
    m_volume = volume;
}
