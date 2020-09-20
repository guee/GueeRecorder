#include "ScreenSource.h"
#include "ScreenLayer.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

Display* ScreenSource::m_x11_Display = nullptr;
int ScreenSource::m_x11_Screen = 0;
Window ScreenSource::m_x11_RootWindow = 0;
Visual* ScreenSource::m_x11_Visual = nullptr;
int ScreenSource::m_x11_Depth = 0;
bool ScreenSource::m_x11_UseShm = false;
QVector<QRect> ScreenSource::m_screenRects;
//QVector<QRect> ScreenSource::m_screenDeadRects;
QRect ScreenSource::m_screenBound;
bool ScreenSource::m_recordCursor = false;
bool ScreenSource::m_cursorUseable = false;

ScreenSource::ScreenSource(const QString& typeName, const QString &sourceName)
    : BaseSource(typeName, sourceName)
{
    m_x11_image = nullptr;
    memset( &m_x11_shm_info, 0, sizeof(m_x11_shm_info) );
    m_x11_shm_server_attached = false;
    qDebug() << "ScreenSource 构造";
}

ScreenSource::~ScreenSource()
{
    onClose();
    freeImage();
    qDebug() << "ScreenSource 析构";
}

bool ScreenSource::onOpen()
{
    start();
    return true;
}

bool ScreenSource::onClose()
{
    m_semShot.release();
    wait();
    if (m_semShot.available()) m_semShot.acquire(m_semShot.available());
    return true;
}

bool ScreenSource::onPlay()
{
    return true;
}

bool ScreenSource::onPause()
{
    return true;
}

bool ScreenSource::static_init()
{
    if(m_x11_Display) return true;
    m_x11_Display = XOpenDisplay(nullptr); //QX11Info::display();
    if ( m_x11_Display == nullptr)
    {
        qCritical() << "XOpenDisplay(nullptr) 调用返回 nullptr!";
        return false;
    }

    m_x11_Screen = DefaultScreen(m_x11_Display); //QX11Info::appScreen();
    m_x11_RootWindow = RootWindow(m_x11_Display, m_x11_Screen); //QX11Info::appRootWindow(m_x11_Screen);
    m_x11_Visual = DefaultVisual(m_x11_Display, m_x11_Screen); //(Visual*) QX11Info::appVisual(m_x11_Screen);
    m_x11_Depth = DefaultDepth(m_x11_Display, m_x11_Screen); //QX11Info::appDepth(m_x11_Screen);
    m_x11_UseShm = XShmQueryExtension(m_x11_Display);
    if(m_x11_UseShm)
    {
        qDebug() << "X11 截屏可以使用共享内存。";
    }
    else
    {
        qWarning() << "X11 截屏不能使用共享内存。";
    }
    if(!m_cursorUseable)
    {
        int event, error;
        if(!XFixesQueryExtension(m_x11_Display, &event, &error))
        {
            qWarning() << "初始化鼠标录制失败，不能录制鼠标。";
            m_cursorUseable = false;
        }
        else
        {
            m_cursorUseable = true;
        }
    }
    readScreenConfig();
    return true;
}

void ScreenSource::static_uninit()
{
    if(m_x11_Display != nullptr) {
        XCloseDisplay(m_x11_Display);
        m_x11_Display = nullptr;

    }
}

bool ScreenSource::setRecordCursor(bool record)
{
    if (m_cursorUseable)
    {
        m_recordCursor = record;
        return true;
    }
    return false;
}

bool ScreenSource::isRecordCursor()
{
    return m_cursorUseable && m_recordCursor;
}

bool ScreenSource::readScreenConfig()
{
    m_screenRects.clear();
    int event_base, error_base;
    if(XineramaQueryExtension(m_x11_Display, &event_base, &error_base))
    {
        int num_screens;
        XineramaScreenInfo *screens = XineramaQueryScreens(m_x11_Display, &num_screens);
        for(int i = 0; i < num_screens; ++i) {
            m_screenRects.push_back(QRect(screens[i].x_org, screens[i].y_org, screens[i].width, screens[i].height));
        }
        XFree(screens);
    }
    else
    {
        qWarning() << "Xinerama 不能工作,不能支持多显示器";
        return true;
    }

    if(m_screenRects.size() == 0)
    {
        qWarning() << "没有获取到显示器信息";
        return true;
    }

    // 计算所有显示器合并后的矩形区域
    m_screenBound = m_screenRects[0];
    for(int i = 1; i < m_screenRects.size(); ++i)
    {
        m_screenBound |= m_screenRects[i];
    }
    if ( m_screenBound.isEmpty())
    {
        qWarning() << "计算出的所有屏幕矩形区域是空的";
        return false;
    }
    // calculate dead space
//    m_screenDeadRects = {m_screenBound};
//    for(int i = 0; i < m_screenRects.size(); ++i)
//    {
//        QRect &subtract = m_screenRects[i];
//        QVector<QRect> result;
//        for(QRect &rect : m_screenDeadRects)
//        {
//            if (rect.intersects(subtract))
//            {
//                if (rect.y() < subtract.y())
//                {
//                    result.push_back(QRect(rect.x(), rect.y(), rect.width(), subtract.y() - rect.y()));
//                }
//                if (rect.x() < subtract.x())
//                {
//                    result.push_back(QRect(rect.x(), std::max(rect.y(), subtract.y()), subtract.x() - rect.x(), subtract.height()));
//                }
//                if (subtract.right() < rect.right())
//                {
//                    result.push_back(QRect(subtract.right() + 1, std::max(rect.y(), subtract.y()), rect.right() - subtract.right(), subtract.height()));
//                }
//                if (subtract.bottom() < rect.bottom())
//                {
//                    result.push_back(QRect(rect.x(), subtract.bottom() + 1, rect.width(), rect.bottom() - subtract.bottom()));
//                }
//            }
//            else
//            {
//                result.push_back(rect);
//            }
//        }
//        m_screenDeadRects = std::move(result);
//    }
    return true;
}

QImage::Format ScreenSource::checkPixelFormat(XImage* image)
{
    QImage::Format pixFmt = QImage::Format_Invalid;
    switch(image->bits_per_pixel)
    {
    case 1:
        pixFmt = QImage::Format_Mono;
        break;
    case 4:
        pixFmt = QImage::Format_Indexed8;
        break;
    case 8:
        pixFmt = QImage::Format_Indexed8;
        break;
    case 16:
        if(image->red_mask == 0xf800 && image->green_mask == 0x07e0 && image->blue_mask == 0x001f)
            pixFmt = QImage::Format_RGB16;
        else if(image->red_mask == 0x7c00 && image->green_mask == 0x03e0 && image->blue_mask == 0x001f)
            pixFmt = QImage::Format_RGB555;
        break;
    case 24:
        if(image->red_mask == 0xff0000 && image->green_mask == 0x00ff00 && image->blue_mask == 0x0000ff)
            pixFmt = QImage::Format_RGB888;
        else if(image->red_mask == 0x0000ff && image->green_mask == 0x00ff00 && image->blue_mask == 0xff0000)
            pixFmt = QImage::Format_RGB888; //??Qt 没有对应的格式。
        break;
    case 32:
        if(image->red_mask == 0x00ff0000 && image->green_mask == 0x0000ff00 && image->blue_mask == 0x000000ff)
            pixFmt = QImage::Format_ARGB32;
        else if(image->red_mask == 0x000000ff && image->green_mask == 0x0000ff00 && image->blue_mask == 0x00ff0000)
            pixFmt = QImage::Format_RGBA8888;
        break;
    default:
        pixFmt = QImage::Format_Invalid;
    }
    return pixFmt;
}

bool ScreenSource::allocImage(uint width, uint height)
{
    if(m_x11_shm_server_attached
            && m_x11_image->width == static_cast<int>(width)
            && m_x11_image->height == static_cast<int>(height))
    {
        return true;
    }
    freeImage();
    m_x11_image = XShmCreateImage(m_x11_Display, m_x11_Visual, static_cast<uint>(m_x11_Depth),
                                  ZPixmap, nullptr, &m_x11_shm_info, width, height);
    if(m_x11_image == nullptr)
    {
        qDebug() << "x11 不能创建共享内存!";
        return false;
    }
    m_pixFormat = checkPixelFormat(m_x11_image);
    m_x11_shm_info.shmid = shmget(IPC_PRIVATE,
                                  static_cast<size_t>(m_x11_image->bytes_per_line * m_x11_image->height),
                                  IPC_CREAT | 0700);
    if(m_x11_shm_info.shmid == -1)
    {
        qDebug() << "x11 获取共享内存信息失败";
        return false;
    }
    m_x11_shm_info.shmaddr = reinterpret_cast<char*>(shmat(m_x11_shm_info.shmid, nullptr, SHM_RND));
    if(m_x11_shm_info.shmaddr == reinterpret_cast<char*>(-1))
    {
        qDebug() << "x11 不能附加到共享内存";
        return false;
    }
    m_x11_image->data = m_x11_shm_info.shmaddr;
    if(!XShmAttach(m_x11_Display, &m_x11_shm_info))
    {
        qDebug() << "x11 不能附加到共享内存";
        return false;
    }
    m_x11_shm_server_attached = true;
    return true;
}

void ScreenSource::freeImage()
{
    if(m_x11_shm_server_attached)
    {
        XShmDetach(m_x11_Display, &m_x11_shm_info);
        m_x11_shm_server_attached = false;
    }
    if(m_x11_shm_info.shmaddr != reinterpret_cast<char*>(-1))
    {
        shmdt(m_x11_shm_info.shmaddr);
        m_x11_shm_info.shmaddr = reinterpret_cast<char*>(-1);
    }
    if(m_x11_shm_info.shmid != -1)
    {
        shmctl(m_x11_shm_info.shmid, IPC_RMID, nullptr);
        m_x11_shm_info.shmid = -1;
    }
    if(m_x11_image != nullptr)
    {
        XDestroyImage(m_x11_image);
        m_x11_image = nullptr;
    }
}

bool ScreenSource::shotScreen(const QRect* rect)
{
    m_imageLock.lock();
    if ( rect )
    {
        m_shotRect = *rect;

    }
    else
    {
        m_shotRect = calcShotRect();
    }
    bool sizeChanged = ( m_width != m_shotRect.width() || m_height != m_shotRect.height() );
    m_width = m_shotRect.width();
    m_height = m_shotRect.height();
    if (m_height <= 0 || m_width <= 0 )
    {
        m_imageLock.unlock();
        return false;
    }

    if(m_x11_UseShm)
    {
        if ( !allocImage(uint32_t(m_width), uint32_t(m_height)) )
        {
            m_imageLock.unlock();
            return false;
        }

        if(!XShmGetImage(m_x11_Display, m_x11_RootWindow, m_x11_image,
                         m_shotRect.x(), m_shotRect.y(), AllPlanes))
        {
            qWarning() << "截取x11屏幕失败。";
            m_imageLock.unlock();
            return false;
        }
    }
    else
    {
        if(m_x11_image != nullptr)
        {
            XDestroyImage(m_x11_image);
            m_x11_image = nullptr;
        }
        m_x11_image = XGetImage(m_x11_Display, m_x11_RootWindow,
                                m_shotRect.x(), m_shotRect.y(),
                                uint32_t(m_width), uint32_t(m_height), AllPlanes, ZPixmap);
        if(m_x11_image == nullptr)
        {
            qWarning() << "截取x11屏幕失败。";
            m_imageLock.unlock();
            return false;
        }
        m_pixFormat = checkPixelFormat(m_x11_image);
    }

    m_imageBuffer   = reinterpret_cast<uint8_t*>(m_x11_image->data);
    m_stride = m_x11_image->bytes_per_line;
    if (m_recordCursor) drawCursor();
    m_imageLock.unlock();

    for (auto it:m_layers)
    {
        ScreenLayer* scr = static_cast<ScreenLayer*>(it);
        scr->m_shotOnScreen.translate(m_screenBound.left() - m_shotRect.left(), m_screenBound.top() - m_shotRect.top());
        if (sizeChanged || scr->m_shotOnScreen != scr->m_rectOnSource)
        {
            scr->setRectOnSource(scr->m_shotOnScreen);
        }
    }


//    QImage img((const uchar*)m_imageBuffer, m_width, m_height, m_stride, QImage::Format_ARGB32);
//    QString fn = QString("/home/guee/Pictures/Temp/[%1x%2] %3.jpg").arg(img.width()).arg(img.height()).arg(m_timestamp / 1000000.0);
//    img.save(fn, "jpg");

    return true;
}

QRect ScreenSource::calcShotRect()
{
    QRect   bound;
    for (auto it:m_layers)
    {
        ScreenLayer* scr = static_cast<ScreenLayer*>(it);//dynamic_cast<ScreenLayer*>(it);
        switch(scr->m_shotOption.mode)
        {
        case ScreenLayer::specScreen:
            scr->m_shotOnScreen = m_screenRects[scr->m_shotOption.screenIndex];
            bound |= scr->m_shotOnScreen;
            break;
        case ScreenLayer::fullScreen:
            scr->m_shotOnScreen = m_screenBound;
            bound |= scr->m_shotOnScreen;
            break;
        case ScreenLayer::rectOfScreen:
            scr->m_shotOnScreen = m_screenBound.intersected(scr->m_shotOption.geometry);
            bound |= scr->m_shotOnScreen;
            break;
        case ScreenLayer::specWindow:
        case ScreenLayer::clientOfWindow:
            if (scr->m_shotOption.windowId)
            {
                XWindowAttributes attributes;
                if ( XGetWindowAttributes(m_x11_Display, scr->m_shotOption.windowId, &attributes) )
                {
                    scr->m_shotOnScreen = QRect(attributes.x, attributes.y, attributes.width, attributes.height) + scr->m_shotOption.margins;
                    scr->m_shotOnScreen = m_screenBound.intersected(scr->m_shotOnScreen);
                    bound |= scr->m_shotOnScreen;
                }
            }
            break;
        default:
            break;
        }


    }
    return bound;
}

void ScreenSource::requestTimestamp(int64_t timestamp)
{
    BaseSource::requestTimestamp(timestamp);
    m_semShot.release();
}

void ScreenSource::run()
{
    while(m_status != BaseLayer::NoOpen)
    {
        m_semShot.acquire();
        if (m_status == BaseLayer::NoOpen)
        {
            break;
        }
        else if (m_status != BaseLayer::Paused)
        {
            if (shotScreen())
            {
                m_imageChanged = true;
                //qDebug() << "shot screen timestamp:" << timestamp / 1000000.0;
            }
        }
    }

}

void ScreenSource::drawCursor()
{
    XFixesCursorImage * ci = XFixesGetCursorImage(m_x11_Display);
    if ( nullptr == ci ) return;
    int bytePerPix = 0, ro, go, bo;
    switch(m_pixFormat)
    {
    case QImage::Format_RGB888:
        bytePerPix = 3;
        ro = 2; go = 1; bo = 0;
        break;
    case QImage::Format_ARGB32:
        bytePerPix = 4;
        ro = 2; go = 1; bo = 0;
        break;
    case QImage::Format_RGBA8888:
        bytePerPix = 4;
        ro = 0; go = 1; bo = 2;
        break;
    default:
        return;
    }


    QRect curRect(ci->x - ci->xhot, ci->y - ci->yhot, ci->width, ci->height);
    QRect ovrRect = m_shotRect.intersected(curRect);

    ulong* curRow = ci->pixels + ci->width * (ovrRect.top() - curRect.top())
            + (ovrRect.left() - curRect.left());
    uint8_t* scrRow = const_cast<uint8_t*>(m_imageBuffer) + m_stride * (ovrRect.top() - m_shotRect.top())
            + (ovrRect.left() - m_shotRect.left()) * m_x11_image->bits_per_pixel / 8;

    for (int y = 0; y < ovrRect.height(); ++y)
    {
        ulong* curPix = curRow;
        uint8_t* scrPix = scrRow;
        for (int x = 0; x < ovrRect.width(); ++x)
        {
            uint8_t ca = uint8_t(*curPix >> 24);
            uint8_t cr = uint8_t(*curPix >> 16);
            uint8_t cg = uint8_t(*curPix >> 8);
            uint8_t cb = uint8_t(*curPix);
            if (ca == 255)
            {
                scrPix[ro] = cr;
                scrPix[go] = cg;
                scrPix[bo] = cb;
            }
            else
            {
                scrPix[ro] = (scrPix[ro] * (255 - ca) + 127) / 255 + cr;
                scrPix[go] = (scrPix[go] * (255 - ca) + 127) / 255 + cg;
                scrPix[bo] = (scrPix[bo] * (255 - ca) + 127) / 255 + cb;
            }
            scrPix += bytePerPix;
            ++curPix;
        }
        curRow += ci->width;
        scrRow += m_stride;
    }
    XFree(ci);
}


