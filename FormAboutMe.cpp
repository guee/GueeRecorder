#include "FormAboutMe.h"
#include "ui_FormAboutMe.h"

FormAboutMe::FormAboutMe(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAboutMe)
{
    ui->setupUi(this);
}

FormAboutMe::~FormAboutMe()
{
    delete ui;
}
