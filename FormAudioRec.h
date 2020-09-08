#ifndef FORMAUDIOREC_H
#define FORMAUDIOREC_H

#include <QWidget>
#include <QAudioInput>
#include "./MediaCodec/SoundRecorder.h"

#include "VdeoSynthesizer.h"

namespace Ui {
class FormAudioRec;
}

class FormAudioRec : public QWidget
{
    Q_OBJECT

public:
    explicit FormAudioRec(QWidget *parent = nullptr);
    ~FormAudioRec();
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    void timerEvent(QTimerEvent* event);
    void resetAudioRecordUI();
private slots:
    void on_pushButtonSndCallback_clicked(bool checked);

    void on_pushButtonSndMicInput_clicked(bool checked);

    void on_pushButtonVolMic_clicked();

    void on_pushButtonVolCB_clicked();

private:
    Ui::FormAudioRec *ui;
    VideoSynthesizer& m_video;

    void popupRecDevs(const QPoint& pos, SoundDevInfo& dev);
};

#endif // FORMAUDIOREC_H
