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
