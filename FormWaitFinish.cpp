#include "FormWaitFinish.h"
#include "ui_FormWaitFinish.h"

FormWaitFinish::FormWaitFinish(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormWaitFinish)
{
    ui->setupUi(this);
}

FormWaitFinish::~FormWaitFinish()
{
    delete ui;
}

void FormWaitFinish::setDispText(const QString &str)
{
    ui->label->setText(str);
}
