#ifndef FORMAUDIOREC_H
#define FORMAUDIOREC_H

#include <QWidget>

namespace Ui {
class FormAudioRec;
}

class FormAudioRec : public QWidget
{
    Q_OBJECT

public:
    explicit FormAudioRec(QWidget *parent = nullptr);
    ~FormAudioRec();

private slots:
    void on_pushButtonSndCallback_clicked(bool checked);

    void on_pushButtonSndInput_clicked(bool checked);

private:
    Ui::FormAudioRec *ui;
};

#endif // FORMAUDIOREC_H
