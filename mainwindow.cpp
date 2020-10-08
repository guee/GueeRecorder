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
    //return QLocale(QLocale::English).toDateTime(QString("%1 %2").arg(__DATE__, __TIME__), "MMM dd yyyy hh:mm:ss");
    //UOS v20 升级到 1022 版之后，上面两种写法神奇地失效了，于是只好自己解析了。
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    QStringList date = QString(__DATE__).split(" ", QString::SkipEmptyParts);
    QStringList time = QString(__TIME__).split(":", QString::SkipEmptyParts);
    if (date.size() == 3 && time.size() == 3)
    {
        int m = 1;
        for (; m <= 12 && date[0] != months[m-1] ; ++m);
        int d = date[1].toInt();
        int y = date[2].toInt();
        return QDateTime(QDate(y, m, d), QTime(time[0].toInt(), time[1].toInt(), time[2].toInt()));
    }
    return QDateTime(QDate(2020, 10, 6));
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

    this->setWindowIcon(QIcon(":/gueeRecorder.ico"));
    //this->setWindowTitle(QApplication::applicationDisplayName());

    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setObjectName("fpsTimerView");
    m_fpsTimer->setInterval(200);
    m_fpsTimer->start();

    ui->setupUi(this);

    ui->widgetLogo->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->widgetTitleInfo->setAttribute(Qt::WA_TransparentForMouseEvents);
   // ui->labelVideoInfo->setAttribute(Qt::WA_TransparentForMouseEvents);

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

    connect(&m_video, &VideoSynthesizer::layerAdded, ui->widgetPreview, &GlWidgetPreview::on_layerAdded);
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

    //setMouseTracking(true);
    if (!QApplication::screens().isEmpty())
    {
        QRect screen = QApplication::screens()[0]->geometry();
        QPoint offset((screen.width() - width()) / 2, (screen.height() - height()) / 2);
        move(offset + screen.topLeft());
    }

    DialogSetting::loadProfile();
    ui->widget_AudioRec->resetAudioRecordUI();

    ui->labelVideoInfo->setText(QString("%1 x %2 @ %3").arg(m_video.width()).arg(m_video.height()).arg(m_video.frameRate()));
    QDateTime tt = buildDateTime();
    qDebug() << tt.toMSecsSinceEpoch();
    QString ver = QString("%1 [%2]").arg(QApplication::applicationVersion(), buildDateTime().toString("yyyy-M-d"));

    ui->labelVersion->setText(ver);
    ui->labelVersion->setToolTip(ver);
    ui->labelLogo->setToolTip(ver);

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

void MainWindow::initMenu()
{
//    QList<int>
//    QAction ss;
    //    ss.setData()
}

void MainWindow::on_close_step_progress(void* param)
{
    MainWindow* me = reinterpret_cast<MainWindow*>(param);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
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
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent";
    if (m_video.status() >= BaseLayer::Opened)
    {
        if (QMessageBox::question(this, "Guee 录屏机", "正在录制视频，是否结束录制并退程序？",
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

void MainWindow::on_widgetPreview_initGL()
{
    m_video.init(ui->widgetPreview->context());
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
        ui->stackedWidgetRecControl->setCurrentIndex(1);
        m_fpsTimer->setInterval(200);
    }
    else
    {
        QMessageBox::critical(this, "开始录制", "启动录像失败");
        ui->pushButtonMenu->setDisabled(false);
        ui->pushButtonRecStart->setDisabled(false);
    }
}


void MainWindow::on_pushButtonRecStop_clicked()
{
    FormWaitFinish wait(this);
    wait.setParent(ui->centralwidget);
    wait.setGeometry(0,0,ui->centralwidget->width(),ui->centralwidget->height());
    wait.raise();
    wait.show();
    ui->stackedWidgetRecControl->setCurrentIndex(0);
    m_video.close(on_close_step_progress, this);
    ui->pushButtonMenu->setDisabled(false);
    ui->pushButtonRecStart->setDisabled(false);
}

void MainWindow::on_pushButtonRecPause_clicked(bool checked)
{
    if (checked)
    {
        m_video.pause();
        m_fpsTimer->setInterval(1000);
    }
    else
    {
        m_video.play();
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
    QString str;
    if (m_video.status() >= VideoSynthesizer::Opened)
    {
        str = QString("渲染fps:%1  编码fps：%2").arg(double(m_video.renderFps()), 0, 'f', 1).arg(double(m_video.encodeFps()), 0, 'f', 1);
        int64_t tim = m_video.timestamp() / 1000;
        int64_t h = tim / 3600000;
        tim %= 3600000;
        int64_t m = tim / 60000;
        tim %= 60000;
        int64_t s = tim / 1000;

        static bool sw = false;
        if (m_video.status() == VideoSynthesizer::Paused)
        {
            sw = !sw;
            if (sw)
            {
                ui->labelRecordTime->setText("已暂停");
            }
        }
        else
        {
            sw = false;
        }
        if (!sw)
            ui->labelRecordTime->setText(QString("%1:%2:%3").arg(h,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0')));
    }
    else
    {
        str = QString("渲染fps:%1").arg(double(m_video.renderFps()), 0, 'f', 1);
    }
    ui->labelRenderFps->setText(str);
}

void MainWindow::on_pushButtonClose_clicked()
{
    close();
}


void MainWindow::on_pushButtonMinimize_clicked()
{
    showMinimized();
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

void MainWindow::on_pushButtonMenu_clicked()
{
    //hide();
    //return;
    DialogSetting dlg(this);
    dlg.exec();
    dlg.saveProfile();
    ui->labelVideoInfo->setText(QString("%1 x %2 @ %3").arg(m_video.width()).arg(m_video.height()).arg(m_video.frameRate()));
    ui->widget_AudioRec->resetAudioRecordUI();
}
