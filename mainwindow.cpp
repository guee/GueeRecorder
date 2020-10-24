#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <omp.h>
#include <unistd.h>
#include "GlWidgetPreview.h"

#include "stdio.h"
#include "DialogSetting.h"
#include "FormWaitFinish.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>

QDateTime buildDateTime()
{
    //return QDateTime::fromString(QString("%1 %2").arg(__DATE__, __TIME__), "MMM dd yyyy hh:mm:ss");
    QString date = QString(__DATE__).replace("  ", " 0");
    QString time = QString(__TIME__);
    return QLocale(QLocale::English).toDateTime(QString("%1 %2").arg(date, time), "MMM dd yyyy hh:mm:ss");
//之前遇上一个坑，当__DATE__的月份或日期是单数时，数字前面没有补0, Qt对这种格式居然解析不了。在找到原因之前，就临时写了下面的代码自己解析。
//    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//    QStringList date = QString(__DATE__).split(" ", QString::SkipEmptyParts);
//    QStringList time = QString(__TIME__).split(":", QString::SkipEmptyParts);
//    if (date.size() == 3 && time.size() == 3)
//    {
//        int m = 1;
//        for (; m <= 12 && date[0] != months[m-1] ; ++m);
//        int d = date[1].toInt();
//        int y = date[2].toInt();
//        return QDateTime(QDate(y, m, d), QTime(time[0].toInt(), time[1].toInt(), time[2].toInt()));
//    }
//    return QDateTime(QDate(2020, 10, 6));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_video(VideoSynthesizer::instance())
{
//    Qt::WindowFlags flags = (Qt::Window | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint |
//            Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    Qt::WindowFlags flags = (Qt::Window | Qt::FramelessWindowHint );
    this->setWindowFlags(flags);
    //this->setAttribute(Qt::WA_TranslucentBackground);
    //this->setAttribute(Qt::WA_TransparentForMouseEvents);


    this->setWindowIcon(QIcon(":/GueeRecorder.svg"));

    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setObjectName("fpsTimerView");
    m_fpsTimer->setInterval(200);
    m_fpsTimer->start();

    ui->setupUi(this);
    ui->pushButton_EnablePreview->hide();
//    ui->pushButton_EnablePreview->setParent(ui->widgetTitle);
//    ui->pushButton_EnablePreview->move(8, 60);
//    ui->pushButton_EnablePreview->raise();

    ui->widgetLogo->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->widgetTitleInfo->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->widgetTitle->setMouseTracking(true);
    ui->widgetTitle->installEventFilter(this);
    ui->widgetBottom->setMouseTracking(true);
    ui->widgetBottom->installEventFilter(this);

    ui->widget_AudioRec->installEventFilter(this);

    ui->pushButtonScreenSelect->installEventFilter(this);
    ui->pushButtonCameraSelect->installEventFilter(this);
    ui->pushButtonMediaSelect->installEventFilter(this);
    ui->stackedWidgetAddContents->installEventFilter(this);
    ui->stackedWidgetAddContents->hide();
    ui->stackedWidgetAddContents->m_mainWindow = this;
    ui->widgetLayerTools->hide();

    ui->widgetPreview->setVideoObject(&m_video, ui->widgetLayerTools);
    ui->widgetLayerTools->refreshLayers(&m_video);
    connect(&m_video, &VideoSynthesizer::initDone, this, &MainWindow::on_videoSynthesizer_initDone);
    connect(&m_video, &VideoSynthesizer::frameReady, ui->widgetPreview,
            &GlWidgetPreview::on_videoSynthesizer_frameReady, Qt::QueuedConnection);

    connect(&m_video, &VideoSynthesizer::layerAdded, ui->widgetLayerTools, &FormLayerTools::on_layerAdded);
    connect(&m_video, &VideoSynthesizer::layerRemoved, ui->widgetLayerTools, &FormLayerTools::on_layerRemoved);
    connect(&m_video, &VideoSynthesizer::layerMoved, ui->widgetLayerTools, &FormLayerTools::on_layerMoved);

    //connect(&m_video, &VideoSynthesizer::layerAdded, ui->widgetPreview, &GlWidgetPreview::on_layerAdded);
    connect(&m_video, &VideoSynthesizer::layerRemoved, ui->widgetPreview, &GlWidgetPreview::on_layerRemoved);
    connect(&m_video, &VideoSynthesizer::layerMoved, ui->widgetPreview, &GlWidgetPreview::on_layerMoved);

    connect( ui->widgetLayerTools, &FormLayerTools::selectLayer, ui->widgetPreview, &GlWidgetPreview::on_selectLayer );
    connect( ui->widgetPreview, &GlWidgetPreview::selectLayer, ui->widgetLayerTools, &FormLayerTools::on_selectLayer );

    QList<QPushButton*> buts = ui->widgetTitle->findChildren<QPushButton*>();
    for ( auto but:buts )
    {
        but->setCursor(Qt::ArrowCursor);
    }
    ui->stackedWidgetRecControl->setCursor(Qt::ArrowCursor);

    if (!QApplication::screens().isEmpty())
    {
        QRect screen = QApplication::screens()[0]->geometry();
        QPoint offset((screen.width() - width()) / 2, (screen.height() - height()) / 2);
        move(offset + screen.topLeft());
    }

    DialogSetting::loadProfile(false);

    ui->labelVideoInfo->setText(QString("%1 x %2 @ %3").arg(m_video.width()).arg(m_video.height()).arg(m_video.frameRate()));
    QDateTime tt = buildDateTime();
    qDebug() << tt.toMSecsSinceEpoch();
    QString ver = QString("%1 [%2]").arg(QApplication::applicationVersion(), buildDateTime().toString("yyyy-M-d"));

    ui->labelVersion->setText(ver);
    ui->labelVersion->setToolTip(ver);
    ui->labelLogo->setToolTip(ver);
    ui->widgetLayerTools->setFixedWindow(DialogSetting::userSetting().fixedLayWnd);
    if (DialogSetting::userSetting().fixedLayWnd)
    {
        ui->widgetLayerTools->show();
    }

    initSystemTrayIcon();
}

MainWindow::~MainWindow()
{
    m_video.uninit();
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //static bool mouseOnAddContents = false;
    if ( watched == ui->pushButtonScreenSelect || watched == ui->pushButtonCameraSelect || watched == ui->pushButtonMediaSelect)
    {
        if ( event->type() == QEvent::Enter)
        {
            // if (!mouseOnAddContents)
            {
                ui->pushButtonScreenSelect->setChecked(watched == ui->pushButtonScreenSelect);
                ui->pushButtonCameraSelect->setChecked(watched == ui->pushButtonCameraSelect);
                ui->pushButtonMediaSelect->setChecked(watched == ui->pushButtonMediaSelect);

                static_cast<QPushButton*>(watched)->setChecked(true);
                if (watched == ui->pushButtonScreenSelect)
                    ui->stackedWidgetAddContents->setCurrentIndex(0);
                else if (watched == ui->pushButtonCameraSelect)
                    ui->stackedWidgetAddContents->setCurrentIndex(1);
                else if (watched == ui->pushButtonMediaSelect)
                    ui->stackedWidgetAddContents->setCurrentIndex(2);
                ui->stackedWidgetAddContents->raise();
                ui->stackedWidgetAddContents->setGeometry(0, 0, ui->widgetPreview->width(), ui->stackedWidgetAddContents->height());
                ui->stackedWidgetAddContents->show();
            }
        }
        else if ( event->type() == QEvent::Leave)
        {
            QPoint po = QCursor::pos();
            if (!ui->stackedWidgetAddContents->rect().contains(ui->stackedWidgetAddContents->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonScreenSelect->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonCameraSelect->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonMediaSelect->mapFromGlobal(po)))
            {
                qDebug() << "QEvent::Leave A";
                ui->stackedWidgetAddContents->hide();
            }
            if (!ui->stackedWidgetAddContents->rect().contains(ui->stackedWidgetAddContents->mapFromGlobal(po)) &&
                    !static_cast<QPushButton*>(watched)->rect().contains(static_cast<QPushButton*>(watched)->mapFromGlobal(po)))
            {
                static_cast<QPushButton*>(watched)->setChecked(false);
            }
        }
    }
    else if (watched == ui->stackedWidgetAddContents)
    {
        if ( event->type() == QEvent::Enter)
        {
            //mouseOnAddContents = true;
        }
        else if ( event->type() == QEvent::Leave)
        {
            QPoint po = QCursor::pos();
            if (!ui->stackedWidgetAddContents->rect().contains(ui->stackedWidgetAddContents->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonScreenSelect->mapFromGlobal(po)) &&
                    !ui->pushButtonCameraSelect->rect().contains(ui->pushButtonCameraSelect->mapFromGlobal(po)) &&
                    !ui->pushButtonMediaSelect->rect().contains(ui->pushButtonMediaSelect->mapFromGlobal(po)))
            {
                qDebug() << "QEvent::Leave B";
                //mouseOnAddContents = false;
                ui->pushButtonScreenSelect->setChecked(false);
                ui->pushButtonCameraSelect->setChecked(false);
                ui->pushButtonMediaSelect->setChecked(false);

                ui->stackedWidgetAddContents->hide();
            }
        }
        else if ( event->type() == QEvent::Hide)
        {
            //mouseOnAddContents = false;
        }
    }
    else if (watched == ui->widgetTitle )
    {
        if ( event->type() == QEvent::MouseButtonPress)
        {
            grabMouse();
        }
        else if ( event->type() == QEvent::MouseMove)
        {
            QMouseEvent * movEvt = static_cast<QMouseEvent*>(event);
            Qt::WindowFrameSection hit = Qt::TitleBarArea;
            if ( movEvt->y() <= 4 )
            {
                hit = Qt::TopSection;
            }

            if ( movEvt->x() <= 4 )
            {
                hit = ( hit == Qt::TopSection ) ? Qt::TopLeftSection : Qt::LeftSection;
            }
            else if ( movEvt->x() >= ui->widgetTitle->width() - 5)
            {
                hit = ( hit == Qt::TopSection ) ? Qt::TopRightSection : Qt::RightSection;

            }
            if ( hit != m_hitMain )
            {
                m_hitMain = hit;
                //qDebug() << "Hit:" << m_hitMain;
                setHitCursor(m_hitMain);
            }
        }
        else if ( event->type() == QEvent::Enter)
        {
        }
        else if ( event->type() == QEvent::Leave)
        {
            m_hitMain = Qt::NoSection;
            setCursor(Qt::ArrowCursor);
        }
    }
    else if (watched == ui->widgetBottom )
    {
        if ( event->type() == QEvent::MouseButtonPress)
        {
            grabMouse();
        }
        else if ( event->type() == QEvent::MouseMove)
        {
            QMouseEvent * movEvt = static_cast<QMouseEvent*>(event);
            Qt::WindowFrameSection hit = Qt::BottomSection;
            if ( movEvt->x() <= 4 )
            {
                hit = Qt::BottomLeftSection;
            }
            else if ( movEvt->x() >= ui->widgetTitle->width() - 5)
            {
                hit = Qt::BottomRightSection;
            }
            if ( hit != m_hitMain )
            {
                m_hitMain = hit;
                setHitCursor(m_hitMain);
            }
        }
        else if ( event->type() == QEvent::Enter)
        {
        }
        else if ( event->type() == QEvent::Leave)
        {
            m_hitMain = Qt::NoSection;
            setCursor(Qt::ArrowCursor);
        }
    }
    else
    {
        if ( event->type() == QEvent::Enter)
        {
            m_hitMain = Qt::NoSection;
            setCursor(Qt::ArrowCursor);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MainWindow::setHitCursor(Qt::WindowFrameSection hit)
{
    switch(hit)
    {
    case Qt::LeftSection:
    case Qt::RightSection:
        setCursor(Qt::SizeHorCursor);
        break;
    case Qt::TopSection:
    case Qt::BottomSection:
        setCursor(Qt::SizeVerCursor);
        break;
    case Qt::TopLeftSection:
    case Qt::BottomRightSection:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Qt::TopRightSection:
    case Qt::BottomLeftSection:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Qt::TitleBarArea:
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void MainWindow::initSystemTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return;
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip(QApplication::applicationName());
    m_trayIcon->setIcon(QIcon(":/GueeRecorder.svg"));

    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::on_systemTrayIcon_activated);

}

void MainWindow::showSystemTrayMenu()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return;
    QMenu* menu = new QMenu(this);
    QAction* actShow = menu->addAction( this->isHidden() ? "显示主窗口" : "隐藏主窗口");
    QAction* actRec = menu->addAction(m_video.status() == BaseLayer::NoOpen ? "开始录制" : "停止录制");
    QAction* actPause = nullptr;
    if (m_video.status() > BaseLayer::NoOpen)
    {
        actPause = menu->addAction(m_video.status() == BaseLayer::Paused ? "继续录制" : "暂停录制");
    }
    menu->addSeparator();
    //if (m_video.status() == BaseLayer::NoOpen)
    QAction* actExit = menu->addAction("退出程序");

    QAction* act = menu->exec(QCursor::pos());
    if (act == actShow)
    {
        if (this->isHidden())
            this->show();
        else
            this->hide();
    }
    else if (act == actRec)
    {
        if (m_video.status() == BaseLayer::NoOpen)
            on_pushButtonRecStart_clicked();
        else
            on_pushButtonRecStop_clicked();
    }
    else if (act && act == actPause)
    {
        if (m_video.status() == BaseLayer::Paused)
        {
            ui->pushButtonRecPause->setChecked(false);
            on_pushButtonRecPause_clicked(false);
        }
        else
        {
            ui->pushButtonRecPause->setChecked(true);
            on_pushButtonRecPause_clicked(true);
        }
    }
    else if (act == actExit)
    {
        on_pushButtonClose_clicked();
    }
    delete menu;
}

void MainWindow::on_close_step_progress(void* param)
{
    MainWindow* me = reinterpret_cast<MainWindow*>(param);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::setWaitWidget(const QString &str)
{
    static FormWaitFinish *waitWidget = nullptr;
    if (str.isEmpty())
    {
        if (waitWidget)
        {
            waitWidget->hide();
            delete waitWidget;
            waitWidget = nullptr;
        }
    }
    else
    {
        if (waitWidget == nullptr)
            waitWidget = new FormWaitFinish(this);
        waitWidget->setDispText(str);
        waitWidget->setParent(ui->centralwidget);
        waitWidget->setGeometry(0,0,ui->centralwidget->width(),ui->centralwidget->height());
        waitWidget->raise();
        waitWidget->show();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        m_pressLeftWndOffset = pos() - event->globalPos();

        ui->widgetPreview->setUpdatesEnabled(false);
        m_pressKeyGlobalPos = event->globalPos();
        m_pressKeyGeometry = geometry();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if ( mouseGrabber() == this )
    {
        ui->widgetPreview->setUpdatesEnabled(true);
        releaseMouse();
        ui->widgetPreview->fixOffsetAsScreen();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if ( mouseGrabber() == this && event->buttons() & Qt::LeftButton )
    {
        QPoint offset = event->globalPos() - m_pressKeyGlobalPos;
        int32_t w, h;
        switch(m_hitMain)
        {
        case Qt::TitleBarArea:
            //move(event->globalPos() + m_pressLeftWndOffset);
            move( m_pressKeyGeometry.topLeft() + offset );
            break;
        case Qt::LeftSection:
            w = qMax(m_pressKeyGeometry.width() - offset.x(), minimumWidth());
            h = qMax(m_pressKeyGeometry.height(), minimumHeight());
            setGeometry(m_pressKeyGeometry.x() + m_pressKeyGeometry.width() - w, m_pressKeyGeometry.y(), w, h);
            break;
        case Qt::RightSection:
            w = qMax(m_pressKeyGeometry.width() + offset.x(), minimumWidth());
            resize(w, m_pressKeyGeometry.height());
            break;
        case Qt::TopSection:
            w = qMax(m_pressKeyGeometry.width(), minimumWidth());
            h = qMax(m_pressKeyGeometry.height() - offset.y(), minimumHeight());
            setGeometry(m_pressKeyGeometry.x(), m_pressKeyGeometry.y() + m_pressKeyGeometry.height() - h, w, h);
            break;
        case Qt::BottomSection:
            resize(m_pressKeyGeometry.width(), m_pressKeyGeometry.height() + offset.y());
            break;
        case Qt::TopLeftSection:
            w = qMax(m_pressKeyGeometry.width() - offset.x(), minimumWidth());
            h = qMax(m_pressKeyGeometry.height() - offset.y(), minimumHeight());
            setGeometry(m_pressKeyGeometry.x() + m_pressKeyGeometry.width() - w, m_pressKeyGeometry.y() + m_pressKeyGeometry.height() - h, w, h);
            break;
        case Qt::TopRightSection:
            w = qMax(m_pressKeyGeometry.width() + offset.x(), minimumWidth());
            h = qMax(m_pressKeyGeometry.height() - offset.y(), minimumHeight());
            setGeometry(m_pressKeyGeometry.x(), m_pressKeyGeometry.y() + m_pressKeyGeometry.height() - h, w, h);
            break;
        case Qt::BottomLeftSection:
            w = qMax(m_pressKeyGeometry.width() - offset.x(), minimumWidth());
            h = qMax(m_pressKeyGeometry.height() + offset.y(), minimumHeight());
            setGeometry(m_pressKeyGeometry.x() + m_pressKeyGeometry.width() - w, m_pressKeyGeometry.y(), w, h);
            break;
        case Qt::BottomRightSection:
            w = qMax(m_pressKeyGeometry.width() + offset.x(), minimumWidth());
            h = qMax(m_pressKeyGeometry.height() + offset.y(), minimumHeight());
            resize(w, h);
            break;
        case Qt::NoSection:
            break;
        }
    }
    //fprintf(stderr, "mouseMoveEvent %d,%d\n", event->x(), event->y());
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)
    ui->widgetPreview->fixOffsetAsScreen();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent";
    if (m_video.status() >= BaseLayer::Opened)
    {
        if (this->isHidden())
        {
            show();
        }
        if (QMessageBox::question(this, "退出 Guee 录屏机", "正在录制视频，是否结束录制并退程序？",
                                  QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel)
                == QMessageBox::Yes)
        {
            on_pushButtonRecStop_clicked();
            m_video.uninit();

            event->accept();

        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        m_video.close(nullptr, nullptr);
        m_video.uninit();
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_delayInitAudio)
    {
        killTimer(m_delayInitAudio);
        m_delayInitAudio = 0;
        DialogSetting::loadProfile(true);
        ui->widget_AudioRec->resetAudioRecordUI();
        setWaitWidget();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() != QEvent::WindowStateChange)
    {
        return;
    }
    if (this->windowState()==Qt::WindowMinimized)
    {
        m_video.enablePreview(false);
    }
    else if (this->windowState()==Qt::WindowMaximized)
    {
        m_video.enablePreview(true);
    }
    else if (this->windowState()==Qt::WindowActive)
    {
        m_video.enablePreview(true);
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    m_video.enablePreview(true);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    m_video.enablePreview(false);
}

void MainWindow::on_widgetPreview_initGL()
{
    setWaitWidget("正在初始化，请稍候……");
    m_video.init(ui->widgetPreview->context());
    m_delayInitAudio = startTimer(200);
}

void MainWindow::on_pushButtonRecStart_clicked()
{
    //ScreenLayer::posOnWindow(QPoint(500,500), this->winId());
    //return;

    if (m_video.status() >= BaseLayer::Opened)
    {
        return;
    }
    ui->pushButtonRecStart->setDisabled(true);
    QString path = DialogSetting::userSetting().videoDir;
    if (!path.endsWith("/")) path.append("/");
    path.append(DialogSetting::userSetting().fileName);
    path.append(" (");
    path.append(QDateTime::currentDateTime().toString("yyyy-MM-dd[HH-mm-ss]"));
    path.append(").");
    path.append(DialogSetting::userSetting().fileType);
    if (m_video.open(path))
    {
        ui->pushButtonMenu->setDisabled(true);
        m_video.play();
        ui->pushButtonRecPause->setChecked(false);
        ui->stackedWidgetRecControl->setCurrentIndex(1);
        m_fpsTimer->setInterval(200);

        if (m_trayIcon) m_trayIcon->setIcon(QIcon(":/GueeRecorder_record.svg"));
    }
    else
    {
        if (this->isHidden())
        {
            if (m_trayIcon && QSystemTrayIcon::supportsMessages())
                m_trayIcon->showMessage("Guee 录屏机", "启动录像失败",QIcon(":/GueeRecorder.svg"), 4000);
        }
        else
        {
            QMessageBox::critical(this, "开始录制", "启动录像失败");
        }
        ui->pushButtonMenu->setDisabled(false);
        ui->pushButtonRecStart->setDisabled(false);
    }
}


void MainWindow::on_pushButtonRecStop_clicked()
{
    setWaitWidget("还有一些数据仍在处理，马上完成……");
    ui->stackedWidgetRecControl->setCurrentIndex(0);
    m_video.close(on_close_step_progress, this);
    ui->pushButtonMenu->setDisabled(false);
    ui->pushButtonRecStart->setDisabled(false);
    setWaitWidget();
    if (m_trayIcon)
    {
        m_trayIcon->setIcon(QIcon(":/GueeRecorder.svg"));
        m_trayIcon->setToolTip(QApplication::applicationName());
    }
}

void MainWindow::on_pushButtonRecPause_clicked(bool checked)
{
    if (checked)
    {
        m_video.pause();
        if (m_trayIcon) m_trayIcon->setIcon(QIcon(":/GueeRecorder_pause.svg"));
        m_fpsTimer->setInterval(1000);
    }
    else
    {
        m_video.play();
        if (m_trayIcon) m_trayIcon->setIcon(QIcon(":/GueeRecorder_record.svg"));
        m_fpsTimer->setInterval(200);
    }
}
void MainWindow::on_videoSynthesizer_initDone(bool success)
{
    if ( success )
    {
//        ScreenLayer* scr = static_cast<ScreenLayer*>(m_video.createLayer("screen"));
//        ScreenLayer::Option opt;
//        opt.mode = ScreenLayer::specScreen;
//        opt.screenIndex = 0;

//        scr->setShotOption(opt);
//        scr->open();
//        scr->play();
    }
}

void MainWindow::on_fpsTimerView_timeout()
{
    static int count = 0;
    QString strFps;
    if (m_video.status() >= VideoSynthesizer::Opened)
    {
        int64_t tim = m_video.timestamp() / 1000;
        int64_t h = tim / 3600000;
        tim %= 3600000;
        int64_t m = tim / 60000;
        tim %= 60000;
        int64_t s = tim / 1000;

        static bool sw = false;

        if (m_video.status() == VideoSynthesizer::Paused)
        {
            strFps = QString("输入:%1fps").arg(double(m_video.renderFps()), 0, 'f', 1);
            sw = !sw;
            if (sw)
            {
                ui->labelRecordTime->setText("已暂停");
            }
            else
            {
                ui->labelRecordTime->setText(QString("%1:%2:%3").arg(h,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0')));
            }
            if (m_trayIcon)
            {
                m_trayIcon->setToolTip(QString("已录制 %1:%2:%3 [暂停] %4 (点击鼠标中键继续录制)").arg(h,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0')).arg(strFps));
            }
            count = 0;
        }
        else
        {
            strFps = QString("输入:%1fps  编码:%2fps").arg(double(m_video.renderFps()), 0, 'f', 1).arg(double(m_video.encodeFps()), 0, 'f', 1);
            sw = false;
            ui->labelRecordTime->setText(QString("%1:%2:%3").arg(h,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0')));
            if (m_trayIcon && count == 0)
            {
                m_trayIcon->setToolTip(QString("已录制 %1:%2:%3 %4 (点击鼠标中键暂停录制)").arg(h,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0')).arg(strFps));
            }
            count = (count + 1) % 5;
        }
    }
    else
    {
        strFps = QString("输入:%1fps").arg(double(m_video.renderFps()), 0, 'f', 1);
        if (m_trayIcon && count == 0)
        {
            m_trayIcon->setToolTip(QString("%1 %2 (点击鼠标中键开始录制)").arg(QApplication::applicationName()).arg(strFps));
        }
        count = (count + 1) % 5;
    }
    ui->labelRenderFps->setText(strFps);
}

void MainWindow::on_pushButtonClose_clicked()
{
    close();
}


void MainWindow::on_pushButtonMinimize_clicked()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        hide();
    }
    else
    {
        showMinimized();
    }
}

void MainWindow::on_pushButtonMenu_clicked()
{
    DialogSetting dlg(this);
    dlg.exec();
    dlg.saveProfile();
    ui->labelVideoInfo->setText(QString("%1 x %2 @ %3").arg(m_video.width()).arg(m_video.height()).arg(m_video.frameRate()));
    ui->widget_AudioRec->resetAudioRecordUI();
}

void MainWindow::on_pushButtonScreenSelect_clicked(bool checked)
{
    if (!checked && ui->stackedWidgetAddContents->isVisible() && ui->stackedWidgetAddContents->currentIndex() == 0)
        ui->pushButtonScreenSelect->setChecked(true);
}

void MainWindow::on_pushButtonCameraSelect_clicked(bool checked)
{
    if (!checked && ui->stackedWidgetAddContents->isVisible() && ui->stackedWidgetAddContents->currentIndex() == 1)
        ui->pushButtonCameraSelect->setChecked(true);
}

void MainWindow::on_pushButtonMediaSelect_clicked(bool checked)
{
    if (!checked && ui->stackedWidgetAddContents->isVisible() && ui->stackedWidgetAddContents->currentIndex() == 2)
        ui->pushButtonMediaSelect->setChecked(true);
}

void MainWindow::on_systemTrayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
    //fprintf(stderr, "on_systemTrayIcon_activated %d\n", reason);
    switch(reason)
    {
    case QSystemTrayIcon::Context:  //鼠标右键
        showSystemTrayMenu();
        break;
    case QSystemTrayIcon::DoubleClick:  //在UOS上，没有得到双击消息
        if (this->isHidden())
            this->show();
        else
            this->hide();
        break;
    case QSystemTrayIcon::Trigger:  //鼠标左键
        showSystemTrayMenu();
        break;
    case QSystemTrayIcon::MiddleClick:  //鼠标中键
        if (m_video.status() == BaseLayer::NoOpen)
        {
            on_pushButtonRecStart_clicked();
        }
        else if (m_video.status() > BaseLayer::NoOpen)
        {
            if (m_video.status() == BaseLayer::Paused)
            {
                ui->pushButtonRecPause->setChecked(false);
                on_pushButtonRecPause_clicked(false);
            }
            else
            {
                ui->pushButtonRecPause->setChecked(true);
                on_pushButtonRecPause_clicked(true);
            }
        }
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_EnablePreview_toggled(bool checked)
{
   // m_video.enablePreview(checked);
}
