#include "DialogSelectScreen.h"
#include "ui_DialogSelectScreen.h"
#include <QFileDialog>

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
    ui->widgetInfo->hide();

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
    this->setGeometry(fullScr);
    ui->widget->setGeometry(fullScr);
    ui->widgetTools->setCursor(Qt::ArrowCursor);
    //connect(ui->widget, &GlScreenSelect::selected, this, &DialogSelectScreen::on_widget_selected);
}

DialogSelectScreen::~DialogSelectScreen()
{
    delete ui;
}

ScreenLayer::Option &DialogSelectScreen::option()
{
    return ui->widget->option();
}

void DialogSelectScreen::timerEvent(QTimerEvent *event)
{
    m_infoShowProg -= 10;
    ui->widgetInfo->setStyleSheet(QString("QWidget#widgetInfo{background-color: rgb(0, 0, 0, %1);}").arg(qMin(255,qMax(0,m_infoShowProg * 255 / 100))));
    if (m_infoShowProg <= 0)
    {
        killTimer(event->timerId());
        ui->widgetInfo->hide();
    }
}

void DialogSelectScreen::setInfoShow(const QString &info)
{
    m_infoShowProg = 400;
    ui->labelInfo->setText(info);
    int x = (width() - ui->widgetInfo->width()) / 2;
    int y = (height() - ui->widgetInfo->height()) / 2;
    ui->widgetInfo->move(x, y);
    ui->widgetInfo->setStyleSheet("QWidget#widgetInfo{background-color: rgb(0, 0, 0, 255);}");
    ui->widgetInfo->show();
    startTimer(50);
}

void DialogSelectScreen::on_widget_selected(bool cancel)
{
    fprintf(stderr, "on_widget_selected : %d\n", cancel);
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
        ui->toolButtonSave->setEnabled(true);
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
//    QFileDialog dlg(this);
//    dlg.setAcceptMode(QFileDialog::AcceptSave);
//    dlg.setFileMode(QFileDialog::AnyFile);
//    dlg.setWindowTitle("保存截图文件");
//    dlg.setNameFilter("Picture (*.jpg *.jpeg *.bmp *.png)");
//    static QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//    dlg.setDirectory(dir);
//    Qt::WindowFlags flags = (Qt::Window | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//    //Qt::WindowFlags flags = (Qt::Window | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint );

//    dlg.setWindowFlags(dlg.windowFlags() | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
//    dlg.exec();
    //dlg.setDefaultSuffix(".jpg")
//    QString file = QFileDialog::getSaveFileName(this, "Save", QString(), "Picture (*.jpg *.jpeg *.bmp *.png)");
//    if (!file.isEmpty())
//    {
//        ui->widget->saveSelectToFile(file);
//    }

    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (!path.endsWith('/')) path += "/";

    path.append(QString("截图 %1.png").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd[HH-mm-ss]")));
    if (ui->widget->saveSelectToFile(path))
    {
        ui->toolButtonSave->setEnabled(false);
        setInfoShow("截图已经存储到：" + path);
    }

}
