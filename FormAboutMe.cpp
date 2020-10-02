#include "FormAboutMe.h"
#include "ui_FormAboutMe.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QLocale>

static QDateTime buildDateTime()
{
    QString dt;
    dt += __DATE__;
    dt += __TIME__;
    return QLocale(QLocale::English).toDateTime(dt, "MMM dd yyyyhh:mm:ss");
}

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

void FormAboutMe::on_label_26_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}
