#ifndef CAMERALAYER_H
#define CAMERALAYER_H

#include "BaseLayer.h"

class QCameraViewfinderSettings;
class CameraSource;
class CameraLayer : public BaseLayer
{
public:
    CameraLayer();
    ~CameraLayer();
    const QString& layerType() const { static QString tn = "camera"; return tn; }
    void setViewSetting(QCameraViewfinderSettings& setting);
    virtual const QString& sourceName();
private:
    CameraSource* m_cameraSour = nullptr;
    QString m_cameraName;
    QCameraViewfinderSettings m_camSetting;
    virtual BaseSource *onCreateSource(const QString &sourceName);
    virtual void onReleaseSource(BaseSource* source);

};

#endif // CAMERALAYER_H
