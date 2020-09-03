#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <omp.h>
#include <unistd.h>
#include "GlWidgetPreview.h"

#include "stdio.h"
//#include "./GueeGL/GLShaderProgram.h"

#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_video(VideoSynthesizer::instance())
{
//    Qt::WindowFlags flags = (Qt::Window | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint |
//            Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    Qt::WindowFlags flags = (Qt::Window | Qt::FramelessWindowHint );
    this->setWindowFlags(flags);

    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setObjectName("fpsTimerView");
    m_fpsTimer->setInterval(100);
    m_fpsTimer->start();

    ui->setupUi(this);

    ui->widgetLogo->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->widgetTitle->setMouseTracking(true);
    ui->widgetTitle->installEventFilter(this);
    ui->widgetBottom->setMouseTracking(true);
    ui->widgetBottom->installEventFilter(this);
    ui->pushButtonScreenSelect->installEventFilter(this);
    ui->pushButtonCameraSelect->installEventFilter(this);
    ui->pushButtonMediaSelect->installEventFilter(this);
    ui->stackedWidgetAddContents->installEventFilter(this);
    ui->stackedWidgetAddContents->hide();
    ui->stackedWidgetAddContents->m_mainWindow = this;

    ui->widgetPreview->setVideoObject(&m_video);
    connect(&m_video, &VideoSynthesizer::initDone, this, &MainWindow::on_videoSynthesizer_initDone);
    connect(&m_video, &VideoSynthesizer::frameReady, ui->widgetPreview, &GlWidgetPreview::on_videoSynthesizer_frameReady);

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

}

MainWindow::~MainWindow()
{
    m_video.uninit();
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static bool mouseOnAddContents = false;
    if ( watched == ui->pushButtonScreenSelect || watched == ui->pushButtonCameraSelect || watched == ui->pushButtonMediaSelect)
    {
        if ( event->type() == QEvent::Enter)
        {
           // if (!mouseOnAddContents)
            {
                static_cast<QPushButton*>(watched)->setChecked(true);
                if (watched == ui->pushButtonScreenSelect)
                    ui->stackedWidgetAddContents->setCurrentIndex(0);
                else if (watched == ui->pushButtonCameraSelect)
                    ui->stackedWidgetAddContents->setCurrentIndex(1);
                else if (watched == ui->pushButtonMediaSelect)
                    ui->stackedWidgetAddContents->setCurrentIndex(2);
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
            mouseOnAddContents = true;
        }
        else if ( event->type() == QEvent::Leave)
        {
            QPoint po = QCursor::pos();
            if (!ui->stackedWidgetAddContents->rect().contains(ui->stackedWidgetAddContents->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonScreenSelect->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonCameraSelect->mapFromGlobal(po)) &&
                    !ui->pushButtonScreenSelect->rect().contains(ui->pushButtonMediaSelect->mapFromGlobal(po)))
            {
                qDebug() << "QEvent::Leave B";
                mouseOnAddContents = false;
                ui->pushButtonScreenSelect->setChecked(false);
                ui->pushButtonCameraSelect->setChecked(false);
                ui->pushButtonMediaSelect->setChecked(false);

                ui->stackedWidgetAddContents->hide();
            }
        }
        else if ( event->type() == QEvent::Hide)
        {
            mouseOnAddContents = false;
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

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        m_pressLeftWndOffset = pos() - event->globalPos();

        //setUpdatesEnabled(false);
        m_pressKeyGlobalPos = event->globalPos();
        m_pressKeyGeometry = geometry();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if ( mouseGrabber() == this )
    {
        //setUpdatesEnabled(true);
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
            w = qMax(m_pressKeyGeometry.width() - offset.x(), sizeHint().width());
            h = qMax(m_pressKeyGeometry.height(), sizeHint().height());
            setGeometry(m_pressKeyGeometry.x() + m_pressKeyGeometry.width() - w, m_pressKeyGeometry.y(), w, h);
            break;
        case Qt::RightSection:
            resize(m_pressKeyGeometry.width() + offset.x(), m_pressKeyGeometry.height());
            break;
        case Qt::TopSection:
            w = qMax(m_pressKeyGeometry.width(), sizeHint().width());
            h = qMax(m_pressKeyGeometry.height() - offset.y(), sizeHint().height());
            setGeometry(m_pressKeyGeometry.x(), m_pressKeyGeometry.y() + m_pressKeyGeometry.height() - h, w, h);
            break;
        case Qt::BottomSection:
            resize(m_pressKeyGeometry.width(), m_pressKeyGeometry.height() + offset.y());
            break;
        case Qt::TopLeftSection:
            w = qMax(m_pressKeyGeometry.width() - offset.x(), sizeHint().width());
            h = qMax(m_pressKeyGeometry.height() - offset.y(), sizeHint().height());
            setGeometry(m_pressKeyGeometry.x() + m_pressKeyGeometry.width() - w, m_pressKeyGeometry.y() + m_pressKeyGeometry.height() - h, w, h);
            break;
        case Qt::TopRightSection:
            w = qMax(m_pressKeyGeometry.width() + offset.x(), sizeHint().width());
            h = qMax(m_pressKeyGeometry.height() - offset.y(), sizeHint().height());
            setGeometry(m_pressKeyGeometry.x(), m_pressKeyGeometry.y() + m_pressKeyGeometry.height() - h, w, h);
            break;
        case Qt::BottomLeftSection:
            w = qMax(m_pressKeyGeometry.width() - offset.x(), sizeHint().width());
            h = qMax(m_pressKeyGeometry.height() + offset.y(), sizeHint().height());
            setGeometry(m_pressKeyGeometry.x() + m_pressKeyGeometry.width() - w, m_pressKeyGeometry.y(), w, h);
            break;
        case Qt::BottomRightSection:
            resize(m_pressKeyGeometry.width() + offset.x(), m_pressKeyGeometry.height() + offset.y());
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
    //m_video.uninit();
    event->accept();
}

void MainWindow::on_widgetPreview_initGL()
{
    m_video.init(ui->widgetPreview->context());
    m_video.setFrameRate(15.0f);
    m_video.setSize(1920, 1080);

 //       ScreenLayer* scr = static_cast<ScreenLayer*>(m_video.createLayer("screen"));

//        ScreenLayer::Option opt;
//        opt.mode = ScreenLayer::specScreen;
//        opt.screenIndex = 0;

//        scr->setShotOption(opt);
//        scr->open();
//        scr->play();
//        Guee::GLShaderProgram   prg;
//        prg.addShaderFromSourceFile(Guee::GlShader::Fragment, ":/Shaders/Test.frag");
//        prg.addShaderFromSourceFile(Guee::GlShader::Vertex, ":/Shaders/Test.vert");
//        prg.link();
}

void MainWindow::on_pushButton_clicked()
{
  //  ScreenLayer* scr = static_cast<ScreenLayer*>(m_video.childLayer(0));
  //  scr->setRect(QRectF(0.3333, 0.3333, 0.5555, 0.5555));

        m_video.uninit();
}

void MainWindow::on_pushButtonRecStart_clicked()
{
    ui->stackedWidgetRecControl->setCurrentIndex(1);
    if (m_video.open("/home/guee/Videos/test.flv"))
    {
        m_video.play();
    }
}

void MainWindow::on_pushButtonRecStop_clicked()
{
    ui->stackedWidgetRecControl->setCurrentIndex(0);
    m_video.close();
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
    ui->labelRenderFps->setText(QString::number(m_video.renderFps()));
}

void MainWindow::on_pushButtonClose_clicked()
{
    close();
}

void MainWindow::on_pushButtonMinimize_clicked()
{
    showMinimized();
}

void MainWindow::on_stackedWidgetAddContents_selectedScreen(ScreenLayer::Option scrOpt)
{
    ScreenLayer* scr = static_cast<ScreenLayer*>(m_video.createLayer("screen"));

    scr->setShotOption(scrOpt);
    scr->open();
    scr->play();
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
