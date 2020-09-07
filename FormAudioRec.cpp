#include "FormAudioRec.h"
#include "ui_FormAudioRec.h"
#include <QMenu>
#include <QDebug>

FormAudioRec::FormAudioRec(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAudioRec)
{
    ui->setupUi(this);
    ui->labelVolCallback->installEventFilter(this);
    ui->labelVolMicInput->installEventFilter(this);
}

FormAudioRec::~FormAudioRec()
{
    delete ui;
}

void FormAudioRec::on_pushButtonSndCallback_clicked(bool checked)
{

}

void FormAudioRec::on_pushButtonSndMicInput_clicked(bool checked)
{

}

bool FormAudioRec::eventFilter(QObject *watched, QEvent *event)
{
    if ( event->type() == QEvent::MouseButtonRelease)
    {
        QLabel* lab = qobject_cast<QLabel*>(watched);
        QAction* refAction = nullptr;
        QAction* clkAction = nullptr;
        SoundDevInfo& dev = ( watched == ui->labelVolCallback) ? m_rec.callbackDev() : m_rec.micInputDev();
        QMenu menu(this);
        do
        {
            QString devDef = dev.defaultDev();
            QString devSel = dev.currentDev();
            menu.clear();
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
            QPoint pos(0, lab->height());
            pos = lab->mapToGlobal(pos);
            clkAction = menu.exec(pos);
        }while(clkAction == refAction);
        if (clkAction && (refAction != clkAction))
        {
            dev.selectDev(clkAction->data().toString());
        }
    }
}
