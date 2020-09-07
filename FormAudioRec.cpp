#include "FormAudioRec.h"
#include "ui_FormAudioRec.h"

FormAudioRec::FormAudioRec(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAudioRec)
{
    ui->setupUi(this);
}

FormAudioRec::~FormAudioRec()
{
    delete ui;
}

void FormAudioRec::on_pushButtonSndCallback_clicked(bool checked)
{

}

void FormAudioRec::on_pushButtonSndInput_clicked(bool checked)
{

}
