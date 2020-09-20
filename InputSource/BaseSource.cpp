#include "BaseSource.h"

BaseSource::BaseSource()
{

}

BaseSource::BaseSource(const QString& typeName, const QString &sourceName)
    : m_typeName(typeName)
    , m_sourceName(sourceName)
{

}

BaseSource::~BaseSource()
{

}

bool BaseSource::sourceOpen(BaseLayer *layer)
{
    if (layer == nullptr) return false;
    if (m_status == BaseLayer::NoOpen)
    {
        m_status = BaseLayer::Opening;
        m_imageChanged = false;
        if ( onOpen() )
        {
            m_status = BaseLayer::Opened;
        }
        else
        {
            m_status = BaseLayer::NoOpen;
        }
    }
    m_imageLock.lock();
    if (m_status >= BaseLayer::Opened)
    {
        if (!m_layers.contains(layer))
        {
            m_layers.push_back(layer);
        }
    }
    m_imageLock.unlock();
    layer->onSizeChanged();
    return m_status >= BaseLayer::Opened;
}

bool BaseSource::sourceClose(BaseLayer *layer)
{
    if(layer == nullptr) return false;
    m_imageLock.lock();
    int i = m_layers.indexOf(layer);
    if ( i < 0 )
    {
        //如果找不到对 layer 的引用，则返回false。
        m_imageLock.unlock();
        return false;
    }
    //移除引用
    m_layers.remove(i);
    //先标记当前是否已经打开，如果已经没有引用了，就设置状态为 isOpened
    bool isOpened = m_status >= BaseLayer::Opened;
    if ( m_layers.empty())
    {
        m_status = BaseLayer::NoOpen;
    }
    m_imageLock.unlock();
    //如果标记为了 NoOpen，表示已经移除了所有引用。已经打开的就必須关闭。
    if ( m_status == BaseLayer::NoOpen && isOpened)
    {
        onClose();
    }
    return m_status == BaseLayer::NoOpen;
}

bool BaseSource::sourcePlay(BaseLayer *layer)
{
    if (layer == nullptr || m_status == BaseLayer::NoOpen) return false;
    if (!m_layers.contains(layer)) return false;
    if (m_status == BaseLayer::Palying) return true;
    if ( (m_status == BaseLayer::Opened || m_status == BaseLayer::Paused) && onPlay() )
    {
        m_status = BaseLayer::Palying;
        return true;
    }
    return true;
}

bool BaseSource::sourcePause(BaseLayer *layer)
{
    if (layer == nullptr || m_status == BaseLayer::NoOpen) return false;
    if (!m_layers.contains(layer)) return false;
    if(m_status == BaseLayer::Paused) return true;
    if ( ( m_status == BaseLayer::Palying || m_status == BaseLayer::Opened ) )
    {
        bool needPause = true;
        for ( auto it:m_layers )
        {
            if ( it != layer && it->status() == BaseLayer::Palying )
            {
                //如果其它的 layer 正在播放，就不能暂停源。
                needPause = false;
                break;
            }
        }
        if ( needPause )
        {
            if (onPause())
            {
                m_status = BaseLayer::Paused;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

bool BaseSource::isSameSource(const QString &type, const QString &source)
{
    m_imageLock.lock();
    if (m_status == BaseLayer::NoOpen)
    {
        m_imageLock.unlock();
        return false;
    }
    m_imageLock.unlock();
    if ( m_typeName == type && m_sourceName == source )
        return true;
    return false;
}

void BaseSource::setSourceFps(float fps)
{
    m_neededFps = fps;
}

void BaseSource::requestTimestamp(int64_t timestamp)
{
    m_requestTimestamp = timestamp;
}

void BaseSource::setImage(const QImage& image)
{
    m_imageBuffer = image.bits();
    m_stride = image.bytesPerLine();
    m_width = image.width();
    m_height = image.height();
    m_pixFormat = image.format();
    m_imageChanged = true;
    updateToTexture();
}

void BaseSource::setFrame(const QVideoFrame &frame)
{
    QVideoFrame cloneFrame(frame);
    if (cloneFrame.map(QAbstractVideoBuffer::ReadOnly))
    {
        m_imageLock.lock();
        QOpenGLTexture::PixelFormat     pixFormat = QOpenGLTexture::NoSourceFormat;
        QOpenGLTexture::PixelType       pixType = QOpenGLTexture::UInt8;
        QOpenGLTexture::TextureFormat   texFormat = QOpenGLTexture::RGBA8_UNorm;
        QSize textureSize = frame.size();
        switch (frame.pixelFormat())
        {
        case QVideoFrame::Format_AYUV444:
            pixFormat = QOpenGLTexture::RGBA;
            texFormat = QOpenGLTexture::RGBA8_UNorm;
            m_intputYuvFormat = 1;
            break;
        case QVideoFrame::Format_YUV444:
            pixFormat = QOpenGLTexture::RGB;
            texFormat = QOpenGLTexture::RGB8_UNorm;
            m_intputYuvFormat = 2;
            break;
        case QVideoFrame::Format_YUV420P:       //YYYY YYYY UU VV
            pixFormat = QOpenGLTexture::Luminance;
            texFormat = QOpenGLTexture::LuminanceFormat;
            m_intputYuvFormat = 3;
            break;
        case QVideoFrame::Format_YV12:          //YYYY YYYY VV UU
            pixFormat = QOpenGLTexture::Luminance;
            texFormat = QOpenGLTexture::LuminanceFormat;
            textureSize.rheight() += textureSize.height() / 2;
            m_intputYuvFormat = 5;
            break;
        case QVideoFrame::Format_UYVY:
            pixFormat = QOpenGLTexture::LuminanceAlpha;
            texFormat = QOpenGLTexture::LuminanceAlphaFormat;
            m_intputYuvFormat = 11;
            break;
        case QVideoFrame::Format_YUYV:
            pixFormat = QOpenGLTexture::LuminanceAlpha;
            texFormat = QOpenGLTexture::LuminanceAlphaFormat;
            m_intputYuvFormat = 12;
            break;
        case QVideoFrame::Format_NV12:          //YYYY YYYY UVUV
            pixFormat = QOpenGLTexture::Luminance;
            texFormat = QOpenGLTexture::LuminanceFormat;
            textureSize.rheight() += textureSize.height() / 2;
            m_intputYuvFormat = 6;
            break;
        case QVideoFrame::Format_NV21:          //YYYY YYYY VUVU
            pixFormat = QOpenGLTexture::Luminance;
            texFormat = QOpenGLTexture::LuminanceFormat;
            textureSize.rheight() += textureSize.height() / 2;
            m_intputYuvFormat = 7;
            break;
        case QVideoFrame::Format_Y8:
            pixFormat = QOpenGLTexture::Luminance;
            texFormat = QOpenGLTexture::LuminanceFormat;
            m_intputYuvFormat = 13;
            break;
        case QVideoFrame::Format_Y16:
            pixFormat = QOpenGLTexture::Luminance;
            texFormat = QOpenGLTexture::LuminanceFormat;
            pixType = QOpenGLTexture::UInt16;
            m_intputYuvFormat = 13;
            break;
        case QVideoFrame::Format_Jpeg:
            m_intputYuvFormat = 0;
            setImage(QImage::fromData(cloneFrame.bits(), cloneFrame.mappedBytes()));
            cloneFrame.unmap();
            return;
        default:
            {
                m_intputYuvFormat = 0;
                QImage::Format fmt = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
                if (fmt)
                {
                    setImage(QImage(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(), cloneFrame.bytesPerLine(), fmt ));
                }
                cloneFrame.unmap();
                return;
            }
        }
        if (m_texture &&
                (m_texture->width() != textureSize.width() || m_texture->height() != textureSize.height() || m_texture->format() != texFormat))
        {
            delete m_texture;
            m_texture = nullptr;
        }
        if (m_texture == nullptr)
        {
            qDebug() << frame.startTime() << "HandType:" << frame.handleType() << ",Format:" << frame.pixelFormat() << ",Size(" << frame.width() << "," << frame.height()<<")";
            m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
            m_texture->create();
            m_texture->bind();
            m_texture->setSize(textureSize.width(), textureSize.height());
            m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
            m_texture->setFormat(texFormat);
            m_texture->allocateStorage(pixFormat, pixType);

            m_imageBuffer = nullptr;
            m_stride = 0;
            m_width = frame.width();
            m_height = frame.height();
            m_pixFormat = QImage::Format_Invalid;
            for (auto it:m_layers)
            {
                it->onSizeChanged();
            }
        }
        else
        {
            m_texture->bind();
        }
        m_imageChanged = true;
        m_texture->setData(pixFormat, pixType, (const uint8_t*)cloneFrame.bits(), nullptr);
        m_imageLock.unlock();
        cloneFrame.unmap();
    }
}

bool BaseSource::updateToTexture()
{
    m_imageLock.lock();
    if (m_imageChanged == false)
    {
        m_imageLock.unlock();
        return false;
    }
    if (m_imageBuffer == nullptr)
    {
        m_imageLock.unlock();
        return true;
    }
    m_imageChanged = false;
    if (nullptr == m_texture || m_texture->width() != m_width || m_texture->height() != m_height)
    {
        if (m_texture)
            delete m_texture;
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        m_texture->create();
        m_texture->bind();
        m_texture->setSize(m_width, m_height);
        m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        QOpenGLTexture::TextureFormat texFmt = QOpenGLTexture::NoFormat;
        switch(m_pixFormat)
        {
        case QImage::Format_RGB32:
            m_pixFormatGL = QOpenGLTexture::BGRA;
            texFmt = QOpenGLTexture::RGBA8_UNorm;
            break;
        case QImage::Format_ARGB32:
            m_pixFormatGL = QOpenGLTexture::BGRA;
            texFmt = QOpenGLTexture::RGBA8_UNorm;
            break;
        case QImage::Format_RGB888:
            m_pixFormatGL = QOpenGLTexture::RGB;
            texFmt = QOpenGLTexture::RGB8_UNorm;
            break;
        case QImage::Format_RGBX8888:
            m_pixFormatGL = QOpenGLTexture::RGBA;
            texFmt = QOpenGLTexture::RGBA8_UNorm;
            break;
        case QImage::Format_RGBA8888:
            m_pixFormatGL = QOpenGLTexture::RGBA;
            texFmt = QOpenGLTexture::RGBA8_UNorm;
            break;
        default:
            m_pixFormatGL = QOpenGLTexture::BGR;
            texFmt = QOpenGLTexture::RGB8_UNorm;
            break;
        }
        m_texture->setFormat(texFmt);
        m_texture->allocateStorage(m_pixFormatGL, QOpenGLTexture::UInt8);
    }
    else
    {
        m_texture->bind();
    }
    m_intputYuvFormat = 0;
    m_texture->setData(m_pixFormatGL, QOpenGLTexture::UInt8, static_cast<const void*>(m_imageBuffer), nullptr);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, m_imageBuffer);
    m_imageLock.unlock();
    return true;
}

int BaseSource::pixelBits(QImage::Format fmt)
{
    switch(fmt)
    {
    case QImage::Format_Invalid:
        return 0;
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        return 1;
    case QImage::Format_Indexed8:
        return 8;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        return 32;
    case QImage::Format_RGB16:
        return 16;
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_RGB666:
    case QImage::Format_ARGB6666_Premultiplied:
        return 24;
    case QImage::Format_RGB555:
        return 16;
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_RGB888:
        return 24;
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
        return 16;
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_BGR30:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_RGB30:
    case QImage::Format_A2RGB30_Premultiplied:
        return 32;
    case QImage::Format_Alpha8:
    case QImage::Format_Grayscale8:
        return 8;
    default:
        return 0;
    }
    return 0;
}

void BaseSource::run()
{
    return;
}

