#include "PictureSource.h"

PictureSource::PictureSource(const QString& typeName, const QString &sourceName)
    : BaseSource(typeName, sourceName)
{

}

PictureSource::~PictureSource()
{
   onClose();
}

bool PictureSource::onOpen()
{
    onClose();
    m_image = new QImage(m_sourceName);
    if (m_image->isNull())
    {
        delete m_image;
        m_image = nullptr;
        return false;
    }
    *m_image = m_image->convertToFormat(QImage::Format_ARGB32);
    m_imageBuffer = m_image->bits();
    m_stride = m_image->bytesPerLine();
    m_width = m_image->width();
    m_height = m_image->height();
    m_pixFormat = m_image->format();
    m_imageChanged = true;
    for (auto it:m_layers)
    {
        it->setRectOnSource(QRect(0, 0, m_width, m_height));
    }
    return true;
}

bool PictureSource::onClose()
{
    if (m_image)
    {
        delete m_image;
        m_image = nullptr;
        m_imageBuffer = nullptr;
        m_imageChanged = false;
        return true;
    }
    return false;
}

bool PictureSource::onPlay()
{
    return m_image ? true : false;
}

bool PictureSource::onPause()
{
    return m_image ? true : false;
}
