#ifndef FORMAUDIOREC_H
#define FORMAUDIOREC_H

#include <QWidget>
#include <QAudioInput>
#include "./MediaCodec/SoundRecorder.h"

#include "VideoSynthesizer.h"

namespace Ui {
class FormAudioRec;
}

class FormAudioRec : public QWidget
{
    Q_OBJECT

public:
    explicit FormAudioRec(QWidget *parent = nullptr);
    ~FormAudioRec();
    void timerEvent(QTimerEvent* event);
    void enterEvent(QEvent *event);
    void resetAudioRecordUI();
private slots:
    void on_pushButtonSndCallback_clicked(bool checked);

    void on_pushButtonSndMicInput_clicked(bool checked);

    void on_pushButtonVolMic_clicked();

    void on_pushButtonVolCB_clicked();

private:
    Ui::FormAudioRec *ui;
    VideoSynthesizer& m_video;

    void popupRecDevs(const QPoint& pos, bool isCallbackDev);
};

#endif // FORMAUDIOREC_H
