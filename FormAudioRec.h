#ifndef FORMAUDIOREC_H
#define FORMAUDIOREC_H

#include <QWidget>
#include <QAudioInput>
#include "./InputSource/SoundRecorder.h"

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
private slots:
    void on_pushButtonSndCallback_clicked(bool checked);

    void on_pushButtonSndMicInput_clicked(bool checked);

private:
    Ui::FormAudioRec *ui;
    SoundRecorder   m_rec;
};

#endif // FORMAUDIOREC_H
