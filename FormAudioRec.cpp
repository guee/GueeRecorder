#include "FormAudioRec.h"
#include "ui_FormAudioRec.h"
#include <QMenu>
#include <QDebug>
#include <QWidgetAction>
#include <QSlider>
#include "FormVolumeAction.h"
#include "DialogSetting.h"

FormAudioRec::FormAudioRec(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAudioRec),
    m_video(VideoSynthesizer::instance())
{
    ui->setupUi(this);

    startTimer(100);
}

FormAudioRec::~FormAudioRec()
{
    delete ui;
}

void FormAudioRec::on_pushButtonSndCallback_clicked(bool checked)
{
    m_video.audCallbackDev().setEnable(checked);
    QSettings ini(DialogSetting::userSetting().profile, QSettings::IniFormat);
    ini.beginGroup("Audio");
    ini.setValue("cbEnabled", m_video.audCallbackDev().isEnabled());
    ini.endGroup();
}

void FormAudioRec::on_pushButtonSndMicInput_clicked(bool checked)
{
    m_video.audMicInputDev().setEnable(checked);
    QSettings ini(DialogSetting::userSetting().profile, QSettings::IniFormat);
    ini.beginGroup("Audio");
    ini.setValue("micEnabled", m_video.audMicInputDev().isEnabled());
    ini.endGroup();
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
    ui->pushButtonVolMic->updateVolume(m_video.audMicInputDev().volume());
    ui->pushButtonVolCB->updateVolume(m_video.audCallbackDev().volume());

    if (m_video.audioIsEnabled())
    {
        //m_video
    }
}

void FormAudioRec::on_pushButtonVolMic_clicked()
{
    QPoint pos(0, ui->pushButtonVolMic->height());
    pos = ui->pushButtonVolMic->mapToGlobal(pos);
    popupRecDevs(pos, false);
}

void FormAudioRec::on_pushButtonVolCB_clicked()
{
    QPoint pos(0, ui->pushButtonVolCB->height());
    pos = ui->pushButtonVolCB->mapToGlobal(pos);
    popupRecDevs(pos, true);
}

void FormAudioRec::popupRecDevs(const QPoint& pos, bool isCallbackDev)
{
    QAction* refAction = nullptr;
    QAction* clkAction = nullptr;
    SoundDevInfo& dev = isCallbackDev ? m_video.audCallbackDev() : m_video.audMicInputDev();
    QMenu menu(this);
    do
    {
        QString devDef = dev.defaultDev();
        QString devSel = dev.currentDev();
        menu.clear();
        QWidgetAction* actVolume = new QWidgetAction(&menu);
        FormVolumeAction* sliVolume = new FormVolumeAction(dev, &menu);

        actVolume->setDefaultWidget(sliVolume);
        menu.addAction(actVolume);
        menu.addSeparator();
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
    QSettings ini(DialogSetting::userSetting().profile, QSettings::IniFormat);
    ini.beginGroup("Audio");
    if (isCallbackDev)
    {
        ini.setValue("cbDev", dev.currentDev());
        //ini.setValue("cbEnabled", dev.isEnabled());
        ini.setValue("cbVolume", dev.volume());
        ui->pushButtonVolCB->updateVolume(m_video.audCallbackDev().volume());
    }
    else
    {
        ini.setValue("micDev", dev.currentDev());
        //ini.setValue("micEnabled", dev.isEnabled());
        ini.setValue("micVolume", dev.volume());
        ui->pushButtonVolMic->updateVolume(m_video.audMicInputDev().volume());
    }
    ini.endGroup();
}