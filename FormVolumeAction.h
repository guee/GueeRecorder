#ifndef FORMVOLUMEACTION_H
#define FORMVOLUMEACTION_H

#include <QWidget>
#include "./MediaCodec/SoundRecorder.h"

namespace Ui {
class FormVolumeAction;
}

class FormVolumeAction : public QWidget
{
    Q_OBJECT

public:
    explicit FormVolumeAction(SoundDevInfo& dev, QWidget *parent = nullptr);
    ~FormVolumeAction();

private slots:
    void on_horizontalSlider_valueChanged(int value);

private:
    Ui::FormVolumeAction *ui;
    SoundDevInfo& m_dev;
};

#endif // FORMVOLUMEACTION_H
