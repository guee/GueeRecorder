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
private:
    CameraSource* m_cameraSour = nullptr;
    virtual BaseSource *onCreateSource(const QString &sourceName);
    virtual void onReleaseSource(BaseSource* source);

};

#endif // CAMERALAYER_H
