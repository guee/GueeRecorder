#include "FormVolumeAction.h"
#include "ui_FormVolumeAction.h"

FormVolumeAction::FormVolumeAction(SoundDevInfo& dev, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormVolumeAction),
    m_dev(dev)
{
    ui->setupUi(this);
    ui->horizontalSlider->setValue(int(ui->horizontalSlider->maximum() * m_dev.volume()));
    ui->label_Volume->setText(QString("%1%").arg(int(m_dev.volume() * 100.0)));
}

FormVolumeAction::~FormVolumeAction()
{
    delete ui;
}

void FormVolumeAction::on_horizontalSlider_valueChanged(int value)
{
    m_dev.setVolume(qreal(value) / ui->horizontalSlider->maximum());
    ui->label_Volume->setText(QString("%1%").arg(int(m_dev.volume() * 100.0)));
}
