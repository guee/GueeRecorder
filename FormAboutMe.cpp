#include "FormAboutMe.h"
#include "ui_FormAboutMe.h"
#include <QDesktopServices>
#include <QUrl>

FormAboutMe::FormAboutMe(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAboutMe)
{
    ui->setupUi(this);

    ui->labelVerDate->setText(QApplication::applicationVersion());
}

FormAboutMe::~FormAboutMe()
{
    delete ui;
}

void FormAboutMe::on_label_Help_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void FormAboutMe::on_label_Diary_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void FormAboutMe::on_label_18_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void FormAboutMe::on_label_20_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void FormAboutMe::on_label_21_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void FormAboutMe::on_label_Source_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}
