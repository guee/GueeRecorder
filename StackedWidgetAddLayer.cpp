#include "StackedWidgetAddLayer.h"
#include "ui_StackedWidgetAddLayer.h"
#include <QFileDialog>
#include <QCameraInfo>
#include <QPainter>
#include <QOpenGLContext>
#include "DialogSelectScreen.h"
#include "VdeoSynthesizer.h"
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
    Q_UNUSED(event);
    on_StackedWidgetAddLayer_currentChanged(currentIndex());
}

void StackedWidgetAddLayer::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    closeCameraPreview();
}

void StackedWidgetAddLayer::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void StackedWidgetAddLayer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void StackedWidgetAddLayer::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void StackedWidgetAddLayer::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void StackedWidgetAddLayer::timerEvent(QTimerEvent *event)
{
    killTimer(event->timerId());
    on_comboBoxCameras_currentIndexChanged(ui->comboBoxCameras->currentIndex());
}

void StackedWidgetAddLayer::showScreens()
{
    QSize iconSize = rescaleButIcon(ui->toolButtonScreenArea, ":/windowAndAreaIcon.png");
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
                scrName = QString("screen %1").arg(i + 1);
             }
            else
            {
                pnt.drawImage(QRect(xs, ys, pix.width() - xs * 2, pix.height() - ys * 2), scr, ScreenLayer::screenBound() );
                objName = QString("toolButton_Screen_All");
                scrName = "All screen";
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
    static QList<QCameraInfo> camsLast;
    QList<QCameraInfo> cams = QCameraInfo::availableCameras();
    if (nullptr == m_selectedCamera)
    {
        ui->comboBoxCameras->setDisabled(true);
        ui->comboBoxSizeFps->setDisabled(true);
        ui->pushButtonAddCamera->setDisabled(true);
    }
    if (cams.count())
    {
        if (camsLast != cams)
        {
            ui->comboBoxCameras->clear();
            ui->comboBoxSizeFps->clear();
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
        ui->comboBoxSizeFps->clear();
    }


}

QSize StackedWidgetAddLayer::rescaleButIcon(QToolButton* but, const QString &path)
{
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
        DialogSelectScreen *dlg = new DialogSelectScreen(m_mainWindow);
        if ( QDialog::Accepted == dlg->exec() )
        {
            scrOpt = dlg->option();
        }
        delete dlg;
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
        layer->setShotOption(scrOpt);
        layer->open();
        layer->play();
        //emit selectedScreen(scrOpt);
    }
   //     m_selArea->startSelect(mainWndow);
//     setMouseTracking(false);
//     releaseMouse();
//     releaseKeyboard();
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
    ui->comboBoxSizeFps->clear();
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
            ui->comboBoxSizeFps->setDisabled(true);
            ui->comboBoxSizeFps->clear();
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
            m_selectedCamera->load();
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
        ui->comboBoxSizeFps->setEnabled(true);
        ui->pushButtonAddCamera->setEnabled(true);

        QCameraViewfinderSettings s = m_selectedCamera->viewfinderSettings();
        QString fmtName = pixNames[(s.pixelFormat() < 0 || s.pixelFormat() >= pixNames.count()) ? 0 : s.pixelFormat()];
        fmtName = QString("[%4] %1 x %2 @ %3fps")
                .arg(s.resolution().width()).arg(s.resolution().height())
                .arg(s.maximumFrameRate()).arg(fmtName);
        qDebug() << "Current:" << fmtName;
    }
    else if (status == QCamera::LoadedStatus)
    {
        if (ui->comboBoxSizeFps->count() == 0)
        {
            ui->comboBoxSizeFps->setDisabled(true);
            ui->comboBoxSizeFps->clear();
            QList<QCameraViewfinderSettings> setList = m_selectedCamera->supportedViewfinderSettings();
            qreal minDiff = -1.0;
            int selIndex = 0;
            for (int i = 0; i < setList.count(); ++i)
            {
                auto s = setList[i];
                int diffW = m_camSetting.resolution().width() - s.resolution().width();
                int diffH = m_camSetting.resolution().height() - s.resolution().height();
                qreal diff = m_camSetting.maximumFrameRate() - s.maximumFrameRate();
                diff = diffW * diffW + diffH * diffH + diff * diff;
                if (m_camSetting.pixelFormat() != s.pixelFormat()) diff *= 2.0;
                if (minDiff < 0.0 || diff < minDiff)
                {
                    minDiff = diff;
                    selIndex = i;
                }

                QString fmtName = pixNames[(s.pixelFormat() < 0 || s.pixelFormat() >= pixNames.count()) ? 0 : s.pixelFormat()];
                fmtName = QString("[%1] %2 x %3 @ %4 fps")
                        .arg(fmtName)
                        .arg(s.resolution().width()).arg(s.resolution().height())
                        .arg(s.maximumFrameRate());
                ui->comboBoxSizeFps->addItem(fmtName);
                qDebug() << fmtName;
            }
            ui->comboBoxSizeFps->setCurrentIndex(selIndex);
            //ui->comboBoxSizeFps->setDisabled(false);

            m_camSetting = setList[selIndex];
            QSize siPreview = m_camSetting.resolution().scaled(ui->widgetCameraFrame->size(), Qt::KeepAspectRatio);
            ui->widgetCamera->setMaximumSize(siPreview);
            ui->widgetCamera->setMinimumSize(siPreview);
            m_selectedCamera->setViewfinderSettings(m_camSetting);
        }
        m_selectedCamera->start();
    }
    else if (status == QCamera::UnloadedStatus)
    {

    }
}

void StackedWidgetAddLayer::on_comboBoxSizeFps_currentIndexChanged(int index)
{
    if (index < 0 || nullptr == m_selectedCamera || !ui->comboBoxSizeFps->isEnabled())
        return;
    qDebug() << "on_comboBoxSizeFps:" << index;
    QList<QCameraViewfinderSettings> setList = m_selectedCamera->supportedViewfinderSettings();
    if (index >= setList.count() || m_camSetting == setList[index])
    {
        return;
    }
    ui->comboBoxSizeFps->setDisabled(true);
    m_camSetting = setList[index];
    m_selectedCamera->setViewfinderSettings(setList[index]);
}

void StackedWidgetAddLayer::closeCameraPreview()
{
    if (m_selectedCamera)
    {
        ui->comboBoxSizeFps->setDisabled(true);
        ui->comboBoxSizeFps->clear();

        m_selectedCamera->stop();
        m_selectedCamera->unload();
        delete m_selectedCamera;
        m_selectedCamera = nullptr;
    }
}

void StackedWidgetAddLayer::on_StackedWidgetAddLayer_currentChanged(int arg1)
{
    if ( arg1 == 0 )
    {
        int32_t ms = QTime::currentTime().msecsSinceStartOfDay();
        if ( m_prevMS + 500 < ms || ms < m_prevMS )
        {
            m_prevMS = ms;
            fprintf(stderr, "%d, showEvent\n", m_prevMS);
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
