#ifndef PICTURESOURCE_H
#define PICTURESOURCE_H

#include "BaseSource.h"
class PictureSource : public BaseSource
{
public:
    PictureSource(const QString& typeName, const QString &sourceName);
    virtual ~PictureSource();
protected:
    virtual bool onOpen();
    virtual bool onClose();
    virtual bool onPlay();
    virtual bool onPause();
private:
    QImage* m_image = nullptr;
};

#endif // PICTURESOURCE_H
