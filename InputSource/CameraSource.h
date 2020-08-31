#ifndef CAMERASOURCE_H
#define CAMERASOURCE_H

#include "BaseSource.h"
#include <QImage>
#include <QVideoProbe>
#include <QCameraInfo>

class CameraSource : public BaseSource
{
    Q_OBJECT
public:
    CameraSource(const QString& typeName, const QString &sourceName = QString());
    void setViewSetting(QCameraViewfinderSettings& setting);
    virtual ~CameraSource();
protected:
    virtual bool onOpen();
    virtual bool onClose();
    virtual bool onPlay();
    virtual bool onPause();
private:
    CameraSource* m_camSour = nullptr;
    QCamera* m_camera = nullptr;
    QVideoProbe* m_vidProbe = nullptr;
    QCameraViewfinderSettings m_camSetting;
private slots:
    void updateCameraState(QCamera::State);
    void processFrame(const QVideoFrame& frame);
};


#endif // CAMERASOURCE_H
