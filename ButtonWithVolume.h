#ifndef BUTTONWITHVOLUME_H
#define BUTTONWITHVOLUME_H

#include <QPushButton>

class ButtonWithVolume : public QPushButton
{
public:
    ButtonWithVolume(QWidget* parent);
    void paintEvent(QPaintEvent* event);
    void updateAmplitude(qreal amp);
private:
    qreal m_amplitude = 0.0;
    QImage m_imgWave;
};

#endif // BUTTONWITHVOLUME_H
