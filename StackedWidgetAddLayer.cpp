#include "StackedWidgetAddLayer.h"
#include "ui_StackedWidgetAddLayer.h"
#undef None
#include <QFileDialog>
#include <QCameraInfo>
#include <QPainter>
#include <QOpenGLContext>
#include <QAction>
#include <QActionGroup>
#include <QVariant>
#include "DialogSelectScreen.h"
#include "VideoSynthesizer.h"
#include "./InputSource/ScreenLayer.h"
#include "./InputSource/CameraLayer.h"
#include "./InputSource/PictureLayer.h"



StackedWidgetAddLayer::StackedWidgetAddLayer(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::StackedWidgetAddLayer)
{
    ui->setupUi(this);
    rescaleButIcon(ui->toolButtonAddPicture, ":/pictureIcon.png");
    m_camSetting.setResolution(640, 480);
    m_camSetting.setPixelFormat(QVideoFrame::Format_YUYV);
    m_camSetting.setMaximumFrameRate(30.0);
}

StackedWidgetAddLayer::~StackedWidgetAddLayer()
{
    delete ui;
}

void StackedWidgetAddLayer::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    on_StackedWidgetAddLayer_currentChanged(currentIndex());
}

void StackedWidgetAddLayer::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    closeCameraPreview();
}

void StackedWidgetAddLayer::timerEvent(QTimerEvent *event)
{
    killTimer(event->timerId());
    on_comboBoxCameras_currentIndexChanged(ui->comboBoxCameras->currentIndex());
}

bool StackedWidgetAddLayer::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip
            || event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonRelease
            || event->type() == QEvent::Enter
            || event->type() == QEvent::Leave)
    {
        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void StackedWidgetAddLayer::showScreens()
{
    static QSize iconSize;
    if (iconSize != rescaleButIcon(ui->toolButtonScreenArea, ""))
    {
        iconSize = rescaleButIcon(ui->toolButtonScreenArea, ":/windowAndAreaIcon.png");
    }
    QImage scr;
    if (ScreenLayer::fullScreenImage(scr))
    {
        //这里有个奇怪的问题，Format_ARGB32的格式不能用QPainter正确地绘制到其它的QImage或QPixmap。
        scr = scr.convertToFormat(QImage::Format_RGB888);
        int c = ScreenLayer::screenCount();
        QToolButton* but = nullptr;
        QString objName;
        do
        {
             objName = QString("toolButton_Screen_%1").arg(c);
             but = ui->pageScreen->findChild<QToolButton*>(objName);
             if (but)
             {
                 ui->horizontalLayoutScreen->removeWidget(but);
                 delete but;
                 ++c;
             }
        }while(but);
        c = ScreenLayer::screenCount();
        if (ScreenLayer::screenCount() > 1)
        {
            ++c;
        }
        else
        {
            objName = QString("toolButton_Screen_All");
            but = ui->pageScreen->findChild<QToolButton*>(objName);
            if (but)
            {
                ui->horizontalLayoutScreen->removeWidget(but);
                delete but;
            }
        }

        for ( int i = 0; i < c; ++i )
        {
            QPixmap pix(iconSize);
            pix.fill(Qt::transparent);
            QSize scrSize = i < ScreenLayer::screenCount() ? ScreenLayer::screenRect(i).size() : ScreenLayer::screenBound().size();
            QSizeF siDraw = scrSize;
            siDraw.scale(pix.size(), Qt::KeepAspectRatio);
            int32_t xs = qFloor((pix.width() - siDraw.width()) * 0.5);
            int32_t ys = qFloor((pix.height() - siDraw.height()) * 0.5);

            QPainter pnt(&pix);
            QRect drawRect(xs, ys, pix.width() - xs * 2, pix.height() - ys * 2);
            pnt.setRenderHint(QPainter::SmoothPixmapTransform);
            pnt.fillRect(drawRect, QColor(070,0,0,255));
            QString objName;
            QString scrName;
            if (i < ScreenLayer::screenCount())
            {
                QRect sourRect = ScreenLayer::screenRect(i);
                pnt.drawImage(drawRect, scr, sourRect );
                objName = QString("toolButton_Screen_%1").arg(i);
                scrName = QString("屏幕 %1").arg(i + 1);
             }
            else
            {
                pnt.drawImage(QRect(xs, ys, pix.width() - xs * 2, pix.height() - ys * 2), scr, ScreenLayer::screenBound() );
                objName = QString("toolButton_Screen_All");
                scrName = "所有屏幕";
            }
            QRect draw(0, pix.height() - 18, pix.width(), 17);
            pnt.fillRect(draw, QColor(0, 0, 0, 128));
            pnt.setPen(QColor(200, 200, 200, 255));
            pnt.drawText(draw, Qt::AlignCenter, QString("%1 x %2").arg(scrSize.width()).arg(scrSize.height()));

            QIcon ico(pix);
            QToolButton* toolButton = ui->pageScreen->findChild<QToolButton*>(objName);
            if ( nullptr == toolButton )
            {
                toolButton = new QToolButton(ui->pageScreen);
                toolButton->setObjectName(objName);
                toolButton->setEnabled(true);
                toolButton->setMinimumSize(ui->toolButtonScreenArea->minimumSize());
                toolButton->setIconSize(ui->toolButtonScreenArea->iconSize());
                //toolButton->setCheckable(true);
                //toolButton->setChecked(false);
                //toolButton->setAutoExclusive(true);
                toolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                ui->horizontalLayoutScreen->addWidget(toolButton);
                connect( toolButton, &QToolButton::clicked, this, &StackedWidgetAddLayer::on_toolButtonScreenArea_clicked);
            }
            toolButton->setText(scrName);
            toolButton->setIcon(ico);
            toolButton->setChecked(i == 0);
        }
    }
}

void StackedWidgetAddLayer::showCameras()
{
    if (m_selectedCamera) return;
    static QList<QCameraInfo> camsLast;
    QList<QCameraInfo> cams = QCameraInfo::availableCameras();
    if (nullptr == m_selectedCamera)
    {
        ui->comboBoxCameras->setDisabled(true);
        makeCameraSizeMenu(nullptr);
    }
    if (cams.count())
    {
        if (camsLast != cams)
        {
            ui->comboBoxCameras->clear();
            makeCameraSizeMenu(nullptr);
            camsLast = cams;
            for (const QCameraInfo& info : cams)
            {
                ui->comboBoxCameras->addItem(info.description(), info.deviceName().toUtf8());
                if (info == QCameraInfo::defaultCamera())
                {
                    ui->comboBoxCameras->setCurrentIndex(ui->comboBoxCameras->count() - 1);
                }
            }
        }
        if (nullptr == m_selectedCamera)
        {
            startTimer(100);
        }
        ui->comboBoxCameras->setEnabled(true);
    }
    else if (nullptr == m_selectedCamera)
    {
        camsLast.clear();
        ui->comboBoxCameras->clear();
        makeCameraSizeMenu(nullptr);
    }


}

QSize StackedWidgetAddLayer::rescaleButIcon(QToolButton* but, const QString &path)
{
    if (path.isEmpty())
    {
        qreal scale = static_cast<QGuiApplication*>(QGuiApplication::instance())->devicePixelRatio();
        return QSize(qRound(but->iconSize().width() * scale), qRound(but->iconSize().height() * scale));
    }

    QImage scr(path);
    qreal scale = static_cast<QGuiApplication*>(QGuiApplication::instance())->devicePixelRatio();
    QSize iconSize(qRound(but->iconSize().width() * scale), qRound(but->iconSize().height() * scale));
    QPixmap pix(iconSize);
    QSizeF siDraw = scr.size().scaled(pix.size(), Qt::KeepAspectRatio);
    int32_t xs = qFloor((pix.width() - siDraw.width()) * 0.5);
    int32_t ys = qFloor((pix.height() - siDraw.height()) * 0.5);
    pix.fill(Qt::transparent);
    QPainter pnt(&pix);
    pnt.setRenderHint(QPainter::SmoothPixmapTransform);
    pnt.drawImage(QRect(xs, ys, pix.width() - xs * 2, pix.height() - ys * 2), scr, scr.rect() );
    but->setIcon(QIcon(pix));
    return iconSize;
}

void StackedWidgetAddLayer::on_toolButtonScreenArea_clicked()
{
    hide();
    ScreenLayer::Option scrOpt;

    QToolButton* but = qobject_cast<QToolButton*>(sender());

    if ( but == ui->toolButtonScreenArea )
    {
        QOpenGLContext* currContext = QOpenGLContext::currentContext();
        QSurface* mainSur = currContext ? currContext->surface() : nullptr;
        //隐藏主界面之后，才能对屏幕截图，以便选择窗口。
        m_mainWindow->setUpdatesEnabled(false);
        m_mainWindow->hide();
        //等待一段时间后对桌面进行截图，因为隐藏窗口的过程可能是动画，需要一定的时间。
        QTime tim;
        tim.start();
        do
        {
            qApp->processEvents();
            QThread::msleep(50);
        }while(tim.elapsed() < 200);

        DialogSelectScreen *dlg = new DialogSelectScreen(m_mainWindow);
        if ( QDialog::Accepted == dlg->exec() )
        {
            scrOpt = dlg->option();

            fprintf(stderr, "Selected window=%p\n",scrOpt.widTop );
        }

        delete dlg;

        m_mainWindow->show();
        m_mainWindow->setUpdatesEnabled(true);

        if ( mainSur )
        {
            currContext->makeCurrent(mainSur);
        }

    }
    else if ( but->objectName() == "toolButton_Screen_All")
    {
        scrOpt.mode = ScreenLayer::fullScreen;
    }
    else if ( but->objectName().indexOf("toolButton_Screen_") >= 0 )
    {
        scrOpt.mode = ScreenLayer::specScreen;
        scrOpt.screenIndex = but->objectName().mid(18).toInt();

    }

    if ( scrOpt.mode != ScreenLayer::unspecified )
    {
        ScreenLayer* layer = static_cast<ScreenLayer*>(VideoSynthesizer::instance().createLayer("screen"));
        layer->setAspectRatioMode(Qt::KeepAspectRatio);
        if (layer->open(scrOpt))
        {
            layer->play();
        }
        else
        {   delete layer;
        }
    }
}

void StackedWidgetAddLayer::on_pushButtonAddCamera_clicked()
{
    hide();
    BaseLayer* layer = VideoSynthesizer::instance().createLayer("camera");
    layer->setAspectRatioMode(Qt::KeepAspectRatio);
    layer->open(ui->comboBoxCameras->itemData(ui->comboBoxCameras->currentIndex()).toString());
    static_cast<CameraLayer*>(layer)->setViewSetting(m_camSetting);
    layer->play();
}

void StackedWidgetAddLayer::on_toolButtonAddPicture_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Open", QString(), "Pictures (*.jpg *.jpeg *.bmp *.png)");
    if (!file.isEmpty())
    {
        PictureLayer* layer = static_cast<PictureLayer*>(VideoSynthesizer::instance().createLayer("picture"));
        layer->setAspectRatioMode(Qt::KeepAspectRatio);
        layer->open( file );
        layer->play();
    }
    hide();
}

void StackedWidgetAddLayer::on_comboBoxCameras_currentIndexChanged(int index)
{
    if (!ui->comboBoxCameras->isEnabled()) return;

    qDebug() << "on_comboBoxCameras:" << index;
    makeCameraSizeMenu(nullptr);
    if (index < 0)
    {
        closeCameraPreview();
        return;
    }
    else
    {
        if (m_selectedCamera)
        {
            if (ui->comboBoxCameras->itemData(index).toString() != m_selectedCamera->objectName())
            {
                closeCameraPreview();
            }
            else
            {
                return;
            }
        }
        if (m_selectedCamera == nullptr)
        {
            makeCameraSizeMenu(nullptr);
            QByteArray dev = ui->comboBoxCameras->itemData(index).toByteArray();
            m_selectedCamera = new QCamera(dev, this);
            connect(m_selectedCamera, &QCamera::statusChanged,
                    this, &StackedWidgetAddLayer::on_camera_statusChanged);
            if (m_selectedCamera->isCaptureModeSupported(QCamera::CaptureVideo))
            {
                m_selectedCamera->setCaptureMode(QCamera::CaptureVideo);
            }
            else if (m_selectedCamera->isCaptureModeSupported(QCamera::CaptureStillImage))
            {
                m_selectedCamera->setCaptureMode(QCamera::CaptureStillImage);
            }
            else
            {
                closeCameraPreview();
                return;
            }
            m_selectedCamera->setViewfinder(ui->widgetCamera);
            m_selectedCamera->start();
        }

//        QList<QCameraViewfinderSettings> setList = m_selectedCamera->supportedViewfinderSettings();
//        for (auto s : setList)
//        {
//            QString fmtName = pixNames[(s.pixelFormat() < 0 || s.pixelFormat() >= pixNames.count()) ? 0 : s.pixelFormat()];
//            ui->comboBoxSizeFps->addItem(QString("%1 x %2 @ %3 [%4]")
//                                         .arg(s.resolution().width()).arg(s.resolution().height())
//                                         .arg(s.maximumFrameRate()).arg(fmtName));
//        }

    }
    //showCameraPreview(true);
}

void StackedWidgetAddLayer::on_camera_statusChanged(QCamera::Status status)
{
    QStringList pixNames = {"Invalid", "ARGB32", "ARGB32_Premultiplied", "RGB32", "RGB24", "RGB565", "RGB555", "ARGB8565_Premultiplied", "BGRA32", "BGRA32_Premultiplied",
                           "BGR32", "BGR24", "BGR565", "BGR555", "BGRA5658_Premultiplied", "AYUV444", "AYUV444_Premultiplied", "YUV444", "YUV420P", "YV12",
                           "UYVY", "YUYV", "NV12", "NV21", "IMC1", "IMC2", "IMC3", "IMC4", "Y8", "Y16",
                           "Jpeg", "CameraRaw", "AdobeDng", "ABGR32"};
    if (status == QCamera::ActiveStatus)
    {
        ui->pushButtonCameraSizeFps->setEnabled(true);
        ui->pushButtonAddCamera->setEnabled(true);

        QCameraViewfinderSettings s = m_selectedCamera->viewfinderSettings();
        QString fmtName = pixNames[(int(s.pixelFormat()) < 0 || s.pixelFormat() >= pixNames.count()) ? 0 : s.pixelFormat()];
        fmtName = QString("[%4] %1 x %2 @ %3fps")
                .arg(s.resolution().width()).arg(s.resolution().height())
                .arg(s.maximumFrameRate()).arg(fmtName);
        qDebug() << "Current:" << fmtName;
    }
    else if (status == QCamera::LoadedStatus)
    {
        makeCameraSizeMenu(m_selectedCamera);
    }
    else if (status == QCamera::UnloadedStatus)
    {

    }
}

void StackedWidgetAddLayer::closeCameraPreview()
{
    if (m_selectedCamera)
    {
        makeCameraSizeMenu(nullptr);

        m_selectedCamera->stop();
        m_selectedCamera->unload();
        delete m_selectedCamera;
        m_selectedCamera = nullptr;
    }
}

void StackedWidgetAddLayer::makeCameraSizeMenu(QCamera *cam)
{
    QStringList pixNames = {"Invalid", "ARGB32", "ARGB32_Premultiplied", "RGB32", "RGB24", "RGB565", "RGB555", "ARGB8565_Premultiplied", "BGRA32", "BGRA32_Premultiplied",
                           "BGR32", "BGR24", "BGR565", "BGR555", "BGRA5658_Premultiplied", "AYUV444", "AYUV444_Premultiplied", "YUV444", "YUV420P", "YV12",
                           "UYVY", "YUYV", "NV12", "NV21", "IMC1", "IMC2", "IMC3", "IMC4", "Y8", "Y16",
                           "Jpeg", "CameraRaw", "AdobeDng", "ABGR32"};
    if ( cam == nullptr )
    {
        if (m_camSizeMenu)
        {
            delete m_camSizeMenu;
            m_camSizeMenu = nullptr;
        }
        ui->pushButtonCameraSizeFps->setDisabled(true);
        ui->pushButtonCameraSizeFps->setText("----");
        ui->pushButtonAddCamera->setDisabled(true);
        return;
    }

    if (m_camSizeMenu == nullptr)
    {
        QList<QCameraViewfinderSettings> setList = m_selectedCamera->supportedViewfinderSettings();
        QList<QSize> sizes = m_selectedCamera->supportedViewfinderResolutions();
        m_camSizeMenu = new QMenu(ui->pushButtonCameraSizeFps);
        QActionGroup* grpSize = new QActionGroup(m_camSizeMenu);
        QActionGroup* grpFps = new QActionGroup(m_camSizeMenu);
        grpSize->setExclusive(true);
        grpFps->setExclusive(true);
        qreal minDiffSize = 65536.0 * 65536.0;
        qreal minDiffFps = 65536.0;
        QMenu* seledSizeMenu = nullptr;
        QAction* selecFpsAction = nullptr;
        for (auto s:sizes)
        {
            QMenu* menuSize = m_camSizeMenu->addMenu(QString("%1 x %2").arg(s.width()).arg(s.height()));
            menuSize->menuAction()->setCheckable(true);
            menuSize->menuAction()->setData(s);
            grpSize->addAction(menuSize->menuAction());

            int diffW = m_camSetting.resolution().width() - s.width();
            int diffH = m_camSetting.resolution().height() - s.height();
            if (diffW * diffW + diffH * diffH < minDiffSize)
            {
                minDiffSize = diffW * diffW + diffH * diffH;
                seledSizeMenu = menuSize;
                selecFpsAction = nullptr;
                minDiffFps = 65536.0;
            }
            for (int i = 0; i < setList.count(); ++i)
            {
                auto& vf = setList[i];
                if (vf.resolution() != s) continue;
                QString fmtName = pixNames[(int(vf.pixelFormat()) < 0 || vf.pixelFormat() >= pixNames.count()) ? 0 : vf.pixelFormat()];
                QAction* act = menuSize->addAction(QString("%1 fps\t[%2]").arg(vf.maximumFrameRate()).arg(fmtName));

                act->setData(i);
                act->setCheckable(true);
                grpFps->addAction(act);
                if ( qAbs(vf.maximumFrameRate() - m_camSetting.maximumFrameRate()) < minDiffFps )
                {
                    minDiffFps = qAbs(vf.maximumFrameRate() - m_camSetting.maximumFrameRate());
                    selecFpsAction = act;
                }
            }
        }
        if (seledSizeMenu)
        {
            seledSizeMenu->menuAction()->setChecked(true);
        }
        if (selecFpsAction)
        {
            selecFpsAction->setChecked(true);
            m_camSetting = setList[selecFpsAction->data().toInt()];
        }

        m_selectedCamera->setViewfinderSettings(m_camSetting);
    }
    QSize siPreview = m_camSetting.resolution().scaled(ui->widgetCameraFrame->size(), Qt::KeepAspectRatio);
    ui->widgetCamera->setMaximumSize(siPreview);
    ui->widgetCamera->setMinimumSize(siPreview);

    QString fmtName = pixNames[(int(m_camSetting.pixelFormat()) < 0 || m_camSetting.pixelFormat() >= pixNames.count()) ? 0 : m_camSetting.pixelFormat()];
    QString sizName = QString("%1 x %2 @ %3 \t[%4]")
        .arg(m_camSetting.resolution().width())
        .arg(m_camSetting.resolution().height())
        .arg(m_camSetting.maximumFrameRate())
        .arg(fmtName);
    ui->pushButtonCameraSizeFps->setText(sizName);
}

void StackedWidgetAddLayer::on_StackedWidgetAddLayer_currentChanged(int arg1)
{
    if ( arg1 == 0 )
    {
        if (ui->pageScreen->isHidden()) return;
        int32_t ms = QTime::currentTime().msecsSinceStartOfDay();
        if ( m_prevMS + 500 < ms || ms < m_prevMS )
        {
            m_prevMS = ms;

            showScreens();
        }
    }
    else if ( currentIndex() == 1 )
    {
        showCameras();
    }
    else if ( currentIndex() == 2 )
    {
    }
}

void StackedWidgetAddLayer::on_pushButtonCameraSizeFps_clicked()
{
    if (m_camSizeMenu)
    {
        QPoint pos(0, ui->pushButtonCameraSizeFps->height());
        pos = ui->pushButtonCameraSizeFps->mapToGlobal(pos);
        QAction* act = m_camSizeMenu->exec(pos);
        if (act)
        {
            QList<QCameraViewfinderSettings> setList = m_selectedCamera->supportedViewfinderSettings();
            if (act->data().toInt() >= setList.count() || m_camSetting == setList[act->data().toInt()])
            {
                return;
            }
            //ui->pushButtonCameraSizeFps->setDisabled(true);
            m_camSetting = setList[act->data().toInt()];
            m_selectedCamera->setViewfinderSettings(m_camSetting);
            qobject_cast<QMenu*>(act->parent())->menuAction()->setChecked(true);
        }
    }
}
