#include "PictureLayer.h"
#include "PictureSource.h"

PictureLayer::PictureLayer()
{

}

BaseSource *PictureLayer::onCreateSource(const QString &sourceName)
{

    return new PictureSource(layerType(), sourceName);
}

void PictureLayer::onReleaseSource(BaseSource *source)
{
    if ( source != nullptr )
    {
        delete source;
    }
}
