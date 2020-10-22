#include "CameraSource.h"

CameraSource::CameraSource(const QString &typeName, const QString &sourceName)
    : BaseSource(typeName, sourceName)
{
    m_isOpaque = true;
    m_vidProbe = new QVideoProbe();
    connect(m_vidProbe, &QVideoProbe::videoFrameProbed,
            this, &CameraSource::processFrame);
}

void CameraSource::setViewSetting(QCameraViewfinderSettings &setting)
{
    if (m_camSetting != setting)
    {
        m_camSetting = setting;
        if (m_camera && m_camSetting.resolution().width() > 80 &&
                m_camSetting.resolution().height() > 60 &&
                m_camSetting.maximumFrameRate() >= 1.0)
        {
            m_camera->setViewfinderSettings(m_camSetting);
        }
    }
}
CameraSource::~CameraSource()
{
    onClose();
}
bool CameraSource::onOpen()
{
    onClose();
    m_camera = new QCamera( m_sourceName.toUtf8(), nullptr );
    connect(m_camera, &QCamera::stateChanged,
            this, &CameraSource::updateCameraState);
    m_vidProbe->setSource(m_camera);
    if (m_camera->isCaptureModeSupported(QCamera::CaptureVideo))
    {
        m_camera->setCaptureMode(QCamera::CaptureVideo);
    }
    else if (m_camera->isCaptureModeSupported(QCamera::CaptureStillImage))
    {
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
    }
    else
    {
        onClose();
        return false;
    }
    if (m_camSetting.resolution().width() > 80 &&
            m_camSetting.resolution().height() > 60 &&
            m_camSetting.maximumFrameRate() >= 1.0)
    {
        m_camera->setViewfinderSettings(m_camSetting);
    }
    m_camera->load();

    return true;
}

bool CameraSource::onClose()
{
    if (m_camera)
    {
        m_camera->stop();
        m_camera->unload();
        m_camera = nullptr;
    }
    return true;
}

bool CameraSource::onPlay()
{
    if (m_camera)
    {
        m_camera->start();
        return true;
    }
    return false;
}

bool CameraSource::onPause()
{
    return (m_camera != nullptr);
}


void CameraSource::updateCameraState(QCamera::State)
{
//    switch (state) {
//    case QCamera::ActiveState:
//        ui->actionStartCamera->setEnabled(false);
//        ui->actionStopCamera->setEnabled(true);
//        ui->captureWidget->setEnabled(true);
//        ui->actionSettings->setEnabled(true);
//        break;
//    case QCamera::UnloadedState:
//    case QCamera::LoadedState:
//        ui->actionStartCamera->setEnabled(true);
//        ui->actionStopCamera->setEnabled(false);
//        ui->captureWidget->setEnabled(false);
//        ui->actionSettings->setEnabled(false);
//    }
}


void CameraSource::processFrame(const QVideoFrame& frame)
{

    if (frame.handleType() == QAbstractVideoBuffer::NoHandle)
    {
        setFrame(frame);
    }

//    Q_UNUSED(requestId);
//    if (m_camImage.format() != img.format() ||
//            m_camImage.size() != img.size())
//    {
//        m_camImage = img;
//    }
//    else if (m_camImage.bytesPerLine() == img.bytesPerLine())
//    {
//        memcpy(m_camImage.bits(), img.bits(), img.byteCount());
//    }
//    else
//    {
//        int cpySize = qMin(m_camImage.bytesPerLine(), img.bytesPerLine());
//        for (int i = 0;i < m_camImage.height(); ++i)
//        {
//            memcpy(m_camImage.scanLine(i), img.scanLine(i), cpySize);
//        }
//    }
//    m_imageBuffer = m_camImage.bits();
//    m_stride = m_camImage.bytesPerLine();
//    m_width = m_camImage.width();
//    m_height = m_camImage.height();
//    m_pixFormat = m_camImage.format();

}
