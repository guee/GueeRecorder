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
    if (m_cameraSour)
    {
        m_cameraSour->setViewSetting(setting);
    }
}

BaseSource *CameraLayer::onCreateSource(const QString &sourceName)
{
    m_cameraSour = new CameraSource(layerType(), sourceName);
    return m_cameraSour;
}

void CameraLayer::onReleaseSource(BaseSource *source)
{
    if ( source != nullptr )
    {
        delete source;
    }
}
