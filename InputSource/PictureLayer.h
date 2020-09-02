#ifndef PICTURELAYER_H
#define PICTURELAYER_H

#include "BaseLayer.h"

class PictureLayer : public BaseLayer
{
public:
    PictureLayer();
    const QString& layerType() const { static QString tn = "picture"; return tn; }
private:
    virtual BaseSource *onCreateSource(const QString &sourceName);
    virtual void onReleaseSource(BaseSource* source);
};

#endif // PICTURELAYER_H
