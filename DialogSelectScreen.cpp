#include "DialogSelectScreen.h"
#include "ui_DialogSelectScreen.h"

DialogSelectScreen::DialogSelectScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectScreen)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = (Qt::Window | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    //Qt::WindowFlags flags = (Qt::Window | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint );

    this->setWindowFlags(flags);
    setMouseTracking(true);
    ui->widgetTools->hide();

    if (parent)
    {
        m_mainOldGeometry = parent->geometry();
        parent->hide();
        //由于隐藏窗口时的渐隐动画时间可能较长，因此换一种方式使主窗口基本不可见，耗时较短。
        parent->setMask(QRegion(0,0,1,1));
        if (parent->pos() == QPoint(0,0))
        {
            parent->move(1,1);
        }
        else
        {
            parent->move(0, 0);
        }
        //等待一段时间后对桌面进行截图。
        QThread::msleep(50);
        parent->setUpdatesEnabled(false);
        parent->hide();
    }
    this->activateWindow();

    QRect fullScr = ScreenLayer::screenBound();
    for(QScreen *screen :  QApplication::screens())
    {
        QRect geometry = screen->geometry();
        qreal ratio = screen->devicePixelRatio();
        QRect physical_geometry(geometry.x(), geometry.y(),
                                static_cast<int32_t>(lrint(geometry.width() * ratio)),
                                static_cast<int32_t>(lrint(geometry.height() * ratio)));
        if(physical_geometry.contains(fullScr.center()))
        {
            ui->widget->setMainScale(ratio);
            fullScr = QRect(
                        static_cast<int32_t>(lrint((fullScr.x()) / ratio)),
                        static_cast<int32_t>(lrint((fullScr.y()) / ratio)),
                        static_cast<int32_t>(lrint(fullScr.width() / ratio)),
                        static_cast<int32_t>(lrint(fullScr.height() / ratio)));
            break;
        }
    }
    //fullScr.setWidth(2000);
    //fullScr.setHeight(1000);
    this->setGeometry(fullScr);
    ui->widget->setGeometry(fullScr);
}

DialogSelectScreen::~DialogSelectScreen()
{
    delete ui;
    if (parentWidget())
    {
        parentWidget()->show();
        parentWidget()->setMask(QRegion(0, 0, m_mainOldGeometry.width(), m_mainOldGeometry.height()));
        parentWidget()->setGeometry(m_mainOldGeometry);
        parentWidget()->setUpdatesEnabled(true);
    }
}

ScreenLayer::Option &DialogSelectScreen::option()
{
    return ui->widget->option();
}

void DialogSelectScreen::on_widget_selected(bool cancel)
{
    if ( cancel )
        this->reject();
    else
        this->accept();
}

void DialogSelectScreen::on_widget_editing(bool ready, const QRect& box)
{
    if (ready)
    {
        ScreenLayer::Option& opt = ui->widget->option();
        switch(opt.mode)
        {
        case ScreenLayer::unspecified:
            ui->labelType->setText("无");
            ui->labelType->setToolTip("");
            break;
        case ScreenLayer::specScreen:
            ui->labelType->setPixmap(QPixmap(":/typeIconScreen.png"));
            ui->labelType->setToolTip("指定屏幕");
            break;
        case ScreenLayer::fullScreen:
            ui->labelType->setPixmap(QPixmap(":/typeIconAllScreen.png"));
            ui->labelType->setToolTip("所有屏幕");
            break;
        case ScreenLayer::rectOfScreen:
            ui->labelType->setPixmap(QPixmap(":/typeIconScreenArea.png"));
            ui->labelType->setToolTip("屏幕区域");
            break;
        case ScreenLayer::specWindow:
            ui->labelType->setPixmap(QPixmap(":/typeIconWindow.png"));
            ui->labelType->setToolTip("指定窗口");
            break;
        case ScreenLayer::clientOfWindow:
            ui->labelType->setPixmap(QPixmap(":/typeIconWndClient.png"));
            ui->labelType->setToolTip("窗口内容");
            break;

        }
        ui->labelSize->setText(QString("%1 x %2").arg(ui->widget->box().width()).arg(ui->widget->box().height()));

        //显示工具栏的位置，准备8个位置，按顺序检查在哪个位置能显示最多的内容，就显示在哪个位置。
        QRect tool(0, 0, ui->widgetTools->width(), ui->widgetTools->height());
        QPoint pos[8];
        int32_t offset = static_cast<int32_t>(4 * ui->widget->mainScale());
        pos[0].setX(box.right() - tool.width()); pos[0].setY(box.bottom() + offset);
        pos[1].setX(box.right() - tool.width() - offset); pos[1].setY(box.bottom() - offset - tool.height());
        pos[2].setX(box.right() - tool.width()); pos[2].setY(box.top() - offset - tool.height());
        pos[3].setX(box.right() - tool.width() - offset); pos[3].setY(box.top() + offset);

        pos[4].setX(box.left()); pos[4].setY(box.bottom() + offset);
        pos[5].setX(box.left() + offset); pos[5].setY(box.bottom() - offset - tool.height());
        pos[6].setX(box.left()); pos[6].setY(box.top() - offset - tool.height());
        pos[7].setX(box.left() + offset); pos[7].setY(box.top() + offset);

        int32_t maxArea = 0;
        QPoint selPos = pos[0];
        for ( int32_t i = 0; i < 8; ++i )
        {
            QRect sect;
            tool.moveTo(pos[i]);
            for(QScreen *screen : QApplication::screens())
            {
                QRect geometry = screen->geometry();
                geometry.moveLeft(static_cast<int32_t>(lrint(geometry.left() / ui->widget->mainScale())));
                geometry.moveTop(static_cast<int32_t>(lrint(geometry.top() / ui->widget->mainScale())));
                sect = geometry.intersected(tool);
                if (sect.width() * sect.height() > maxArea)
                {
                    maxArea = sect.width() * sect.height();
                    selPos = pos[i];
                    if ( sect.width() == tool.width() && sect.height() == tool.height() ) break;
                }
            }
            if ( sect.width() == tool.width() && sect.height() == tool.height() ) break;
            if ( box.height() - offset * 2 < tool.height() ) ++i;
        }
        ui->widgetTools->move(selPos);
        ui->widgetTools->show();
        ui->widget->setToolsBox(ui->widgetTools->geometry());
    }
    else
    {
        ui->widgetTools->hide();
    }
}

void DialogSelectScreen::on_toolButtonOK_clicked()
{
    this->accept();
}

void DialogSelectScreen::on_toolButtonCancel_clicked()
{
    this->reject();
}

void DialogSelectScreen::on_toolButtonSave_clicked()
{

}
