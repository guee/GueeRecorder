#include "CameraLayer.h"
#include "CameraSource.h"
#include <QCameraInfo>

CameraLayer::CameraLayer()
{

}

CameraLayer::~CameraLayer()
{

}

void CameraLayer::setViewSetting(QCameraViewfinderSettings &setting)
{
    if (m_camSetting != setting)
    {
        m_camSetting = setting;
        if (m_cameraSour)
        {
            m_cameraSour->setViewSetting(setting);
        }
    }
}

const QString &CameraLayer::sourceName()
{
    return m_cameraName.isEmpty() ? BaseLayer::sourceName() : m_cameraName;
}

BaseSource *CameraLayer::onCreateSource(const QString &sourceName)
{
    m_cameraSour = new CameraSource(layerType(), sourceName);
    m_cameraSour->setViewSetting(m_camSetting);
    m_cameraName = QCameraInfo(sourceName.toUtf8()).description();
    return m_cameraSour;
}

void CameraLayer::onReleaseSource(BaseSource *source)
{
    if ( source != nullptr )
    {
        if (m_cameraSour == source )
        {
            delete source;
            m_cameraSour = nullptr;
            m_cameraName.clear();
        }
    }
}
