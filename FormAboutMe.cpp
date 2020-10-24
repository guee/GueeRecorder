#include "FormAboutMe.h"
#include "ui_FormAboutMe.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QLocale>



QDateTime buildDateTime();

FormAboutMe::FormAboutMe(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAboutMe)
{
    ui->setupUi(this);
    ui->label_Name->setText(QString("%1 %2").arg(QApplication::applicationDisplayName(),QApplication::applicationVersion()));
    ui->labelVerDate->setText(buildDateTime().toString("yyyy-M-d"));

}

FormAboutMe::~FormAboutMe()
{
    delete ui;
}

