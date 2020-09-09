#include "FormAudioRec.h"
#include "ui_FormAudioRec.h"
#include <QMenu>
#include <QDebug>
#include <QWidgetAction>

FormAudioRec::FormAudioRec(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAudioRec),
    m_video(VideoSynthesizer::instance())
{
    ui->setupUi(this);
    //ui->labelVolCallback->installEventFilter(this);
    //ui->labelVolMicInput->installEventFilter(this);

    startTimer(100);
}

FormAudioRec::~FormAudioRec()
{
    delete ui;
}

void FormAudioRec::on_pushButtonSndCallback_clicked(bool checked)
{
    m_video.audCallbackDev().setEnable(checked);
}

void FormAudioRec::on_pushButtonSndMicInput_clicked(bool checked)
{
    m_video.audMicInputDev().setEnable(checked);
}

bool FormAudioRec::eventFilter(QObject *watched, QEvent *event)
{
//    if ( event->type() == QEvent::MouseButtonRelease)
//    {
//        QLabel* lab = qobject_cast<QLabel*>(watched);
//        QAction* refAction = nullptr;
//        QAction* clkAction = nullptr;
//        SoundDevInfo& dev = ( watched == ui->labelVolCallback) ? m_rec.callbackDev() : m_rec.micInputDev();
//        QMenu menu(this);
//        do
//        {
//            QString devDef = dev.defaultDev();
//            QString devSel = dev.currentDev();
//            menu.clear();
//            for (auto &n:dev.availableDev(clkAction && (refAction == clkAction)))
//            {
//                QAction* act = menu.addAction(devDef == n ? QString("[默认] %1").arg(n) : n);
//                act->setData(n);
//                if (devSel == n)
//                {
//                    act->setCheckable(true);
//                    act->setChecked(true);
//                }
//            }
//            menu.addSeparator();
//            refAction = menu.addAction("刷新列表");
//            QPoint pos(0, lab->height());
//            pos = lab->mapToGlobal(pos);
//            clkAction = menu.exec(pos);
//        }while(clkAction == refAction);
//        if (clkAction && (refAction != clkAction))
//        {
//            dev.selectDev(clkAction->data().toString());
//        }
//    }
}

void FormAudioRec::timerEvent(QTimerEvent *event)
{
    ui->pushButtonVolMic->updateAmplitude(m_video.audMicInputDev().amplitude());
    ui->pushButtonVolCB->updateAmplitude(m_video.audCallbackDev().amplitude());
    // qDebug() << m_video.audCallbackDev().amplitude() << "," << m_video.audMicInputDev().amplitude();
}

void FormAudioRec::resetAudioRecordUI()
{
    setEnabled(m_video.audioIsEnabled());
    SoundDevInfo& cbDev = m_video.audCallbackDev();
    SoundDevInfo& micDev = m_video.audMicInputDev();
    ui->pushButtonSndCallback->setChecked(cbDev.isEnabled());
    ui->pushButtonSndMicInput->setChecked(micDev.isEnabled());
    if (m_video.audioIsEnabled())
    {
        //m_video
    }
}

void FormAudioRec::on_pushButtonVolMic_clicked()
{
    QPoint pos(0, ui->pushButtonVolMic->height());
    pos = ui->pushButtonVolMic->mapToGlobal(pos);
    popupRecDevs(pos, m_video.audMicInputDev());
}

void FormAudioRec::on_pushButtonVolCB_clicked()
{
    QPoint pos(0, ui->pushButtonVolCB->height());
    pos = ui->pushButtonVolCB->mapToGlobal(pos);
    popupRecDevs(pos, m_video.audCallbackDev());
}

void FormAudioRec::popupRecDevs(const QPoint& pos, SoundDevInfo &dev)
{
    QAction* refAction = nullptr;
    QAction* clkAction = nullptr;
    QMenu menu(this);
    do
    {
        QString devDef = dev.defaultDev();
        QString devSel = dev.currentDev();
        menu.clear();
        QWidgetAction* actVolume = new QWidgetAction(&menu);
        QSlider* sliVolume = new QSlider(&menu);
        actVolume->setDefaultWidget(new QSlider)
        menu.addAction()
        for (auto &n:dev.availableDev(clkAction && (refAction == clkAction)))
        {
            QAction* act = menu.addAction(devDef == n ? QString("[默认] %1").arg(n) : n);
            act->setData(n);
            if (devSel == n)
            {
                act->setCheckable(true);
                act->setChecked(true);
            }
        }
        menu.addSeparator();
        refAction = menu.addAction("刷新列表");
        clkAction = menu.exec(pos);
    }while(clkAction == refAction);
    if (clkAction && (refAction != clkAction))
    {
        dev.selectDev(clkAction->data().toString());
    }

}
