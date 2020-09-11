#ifndef BASESOURCE_H
#define BASESOURCE_H

#include "BaseLayer.h"
#include <QVideoFrame>
#include <QOpenGLTexture>

class BaseSource : public QObject
{

public:
    BaseSource(const QString& typeName, const QString &sourceName = QString());
    virtual ~BaseSource();
    //打开数据源
    //返回值表示源是否打开成功。成功则返回 true，并添加 layer 的引用。失败返回 false，不添加引用。
    bool sourceOpen(BaseLayer* layer);
    //关闭数据源
    //关闭数据源时会移除 layer 的引用，当所有引用都移出后，引用计数为0时返回 true，表示本对象可以被删除了。
    bool sourceClose(BaseLayer* layer);

    bool sourcePlay(BaseLayer* layer);
    bool sourcePause(BaseLayer* layer);

    virtual bool isSameSource(const QString& type, const QString& source);

    void setSourceFps(float fps);
    bool updateToTexture();
    void setImage(const QImage& image);
    void setFrame(const QVideoFrame &frame);
    int width() const { return m_width; }
    int height() const { return m_height; }
    friend BaseLayer;

    QVector<BaseLayer*> m_layers;
    const uint8_t* m_imageBuffer = nullptr;
    int32_t m_stride = 0;
    int32_t m_width = 0;
    int32_t m_height = 0;
    int64_t m_lastTimestamp = -1;  //如果视频帧的时间戳要以此源的时间戳为准，就设置为大于等于0的毫秒值
    float m_neededFps = 0.0f;
    int   m_intputYuvFormat = 0;

    QImage::Format m_pixFormat = QImage::Format_Invalid;
    QOpenGLTexture::PixelFormat m_pixFormatGL = QOpenGLTexture::NoSourceFormat;
    QMutex m_imageLock;
    BaseLayer::EStatus m_status = BaseLayer::NoOpen;

   // static QOpenGLContext* m_context;
    //static QOffscreenSurface* m_surface;
    QOpenGLTexture* m_texture = nullptr;
    bool m_imageChanged = false;
    static int pixelBits(QImage::Format fmt);
protected:
    virtual bool onOpen() = 0;
    virtual bool onClose() = 0;
    virtual bool onPlay() = 0;
    virtual bool onPause() = 0;

    QString m_typeName;
    QString m_sourceName;
};

#endif // BASESOURCE_H
