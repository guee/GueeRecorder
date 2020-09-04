#include "ScreenSource.h"
#include "ScreenLayer.h"

Display* ScreenSource::m_x11_Display = nullptr;
int ScreenSource::m_x11_Screen = 0;
Window ScreenSource::m_x11_RootWindow = 0;
Visual* ScreenSource::m_x11_Visual = nullptr;
int ScreenSource::m_x11_Depth = 0;
bool ScreenSource::m_x11_UseShm = false;
QVector<QRect> ScreenSource::m_screenRects;
QVector<QRect> ScreenSource::m_screenDeadRects;
QRect ScreenSource::m_screenBound;
bool ScreenSource::m_recordCursor = false;

ScreenSource::ScreenSource(const QString& typeName, const QString &sourceName)
    : BaseSource(typeName, sourceName)
{
    m_recordCursor = false;
    m_x11_image = nullptr;
    memset( &m_x11_shm_info, 0, sizeof(m_x11_shm_info) );
    m_x11_shm_server_attached = false;
}

ScreenSource::~ScreenSource()
{
    onClose();
    freeImage();
}

bool ScreenSource::onOpen()
{
    m_frameSync.init(m_neededFps);
    m_thread = std::thread(&ScreenSource::shotThread, this);
    return true;
}

bool ScreenSource::onClose()
{
    m_frameSync.stop();
    if(m_thread.joinable())
    {
        m_thread.join();
    }
    return true;
}

bool ScreenSource::onPlay()
{
    m_frameSync.start();
    m_timestamp = m_frameSync.elapsed();
    return true;
}

bool ScreenSource::onPause()
{
    m_frameSync.pause();
    return true;
}

bool ScreenSource::static_init()
{
    if(m_x11_Display) return true;
    m_x11_Display = XOpenDisplay(nullptr); //QX11Info::display();
    if ( m_x11_Display == nullptr)
    {
        qDebug() << "can't open X display!";
        return false;
    }

    m_x11_Screen = DefaultScreen(m_x11_Display); //QX11Info::appScreen();
    m_x11_RootWindow = RootWindow(m_x11_Display, m_x11_Screen); //QX11Info::appRootWindow(m_x11_Screen);
    m_x11_Visual = DefaultVisual(m_x11_Display, m_x11_Screen); //(Visual*) QX11Info::appVisual(m_x11_Screen);
    m_x11_Depth = DefaultDepth(m_x11_Display, m_x11_Screen); //QX11Info::appDepth(m_x11_Screen);
    m_x11_UseShm = XShmQueryExtension(m_x11_Display);
    if(m_x11_UseShm)
    {
        fprintf(stderr, "[X11Input::Init] Using X11 shared memory.");
    }
    else
    {
        fprintf(stderr, "[X11Input::Init] Not using X11 shared memory.");
    }
    if(m_recordCursor)
    {
        int event, error;
        if(!XFixesQueryExtension(m_x11_Display, &event, &error))
        {
            fprintf(stderr, "[X11Input::Init] Warning: XFixes is not supported by X server, the cursor has been hidden. Don't translate 'XFixes'");
            m_recordCursor = false;
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
        fprintf(stderr, "[X11Input::Init] Warning: Xinerama is not supported by X server, multi-monitor support may not work properly. Don't translate 'Xinerama'");
        return true;
    }

    // make sure that we have at least one monitor
    if(m_screenRects.size() == 0) {
        fprintf(stderr, "[X11Input::Init] Warning: No monitors detected, multi-monitor support may not work properly.");
        return true;
    }

    // calculate bounding box
    m_screenBound = m_screenRects[0];
    for(int i = 1; i < m_screenRects.size(); ++i)
    {
        m_screenBound |= m_screenRects[i];
    }
    if ( m_screenBound.isEmpty())
    {
        fprintf(stderr, "[X11Input::UpdateScreenConfiguration] Error: Invalid screen bounding box!\n");
        return false;
    }
    // calculate dead space
    m_screenDeadRects = {m_screenBound};
    for(int i = 0; i < m_screenRects.size(); ++i)
    {
        QRect &subtract = m_screenRects[i];
        QVector<QRect> result;
        for(QRect &rect : m_screenDeadRects)
        {
            if (rect.intersects(subtract))
            {
                if (rect.y() < subtract.y())
                {
                    result.push_back(QRect(rect.x(), rect.y(), rect.width(), subtract.y() - rect.y()));
                }
                if (rect.x() < subtract.x())
                {
                    result.push_back(QRect(rect.x(), std::max(rect.y(), subtract.y()), subtract.x() - rect.x(), subtract.height()));
                }
                if (subtract.right() < rect.right())
                {
                    result.push_back(QRect(subtract.right() + 1, std::max(rect.y(), subtract.y()), rect.right() - subtract.right(), subtract.height()));
                }
                if (subtract.bottom() < rect.bottom())
                {
                    result.push_back(QRect(rect.x(), subtract.bottom() + 1, rect.width(), rect.bottom() - subtract.bottom()));
                }
            }
            else
            {
                result.push_back(rect);
            }
        }
        m_screenDeadRects = std::move(result);
    }
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
            pixFmt = QImage::Format_RGB888;
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
        return true; // reuse existing image
    }
    freeImage();
    m_x11_image = XShmCreateImage(m_x11_Display, m_x11_Visual, static_cast<uint>(m_x11_Depth),
                                  ZPixmap, nullptr, &m_x11_shm_info, width, height);
    if(m_x11_image == nullptr)
    {
        qDebug() << "[X11Input::Init] Error: Can't create shared image!";
        return false;
    }
    m_pixFormat = checkPixelFormat(m_x11_image);
    m_x11_shm_info.shmid = shmget(IPC_PRIVATE,
                                  static_cast<size_t>(m_x11_image->bytes_per_line * m_x11_image->height),
                                  IPC_CREAT | 0700);
    if(m_x11_shm_info.shmid == -1)
    {
        qDebug() << "[X11Input::Init] Error: Can't get shared memory!";
        return false;
    }
    m_x11_shm_info.shmaddr = reinterpret_cast<char*>(shmat(m_x11_shm_info.shmid, nullptr, SHM_RND));
    if(m_x11_shm_info.shmaddr == reinterpret_cast<char*>(-1))
    {
        qDebug() << "[X11Input::Init] Error: Can't attach to shared memory!";
        return false;
    }
    m_x11_image->data = m_x11_shm_info.shmaddr;
    if(!XShmAttach(m_x11_Display, &m_x11_shm_info))
    {
        qDebug() << "[X11Input::Init] Error: Can't attach server to shared memory!";
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

void ScreenSource::shotThread()
{
//    m_frameSync.start();
//    m_frameSync.pause();
//    m_frameRate.start();
 //   m_timestamp = m_frameRate.elapsed();
    while(m_status != BaseLayer::NoOpen)
    {
        if (m_timestamp < 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else if (m_status == BaseLayer::Palying)
        {
            if (shotScreen())
            {
                m_imageChanged = true;
                //qDebug() << "shot screen m_timestamp:" << m_timestamp / 1000000.0;
            }
        }
        m_timestamp = m_frameSync.waitNextFrame();
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
        if ( !allocImage(static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)) )
        {
            m_imageLock.unlock();
            return false;
        }

        if(!XShmGetImage(m_x11_Display, m_x11_RootWindow, m_x11_image,
                         m_shotRect.x(), m_shotRect.y(), AllPlanes))
        {
            fprintf(stderr, "[X11Input::InputThread] Error: Can't get image (using shared memory)!\n"
                             "    Usually this means the recording area is not completely inside the screen. Or did you change the screen resolution?");
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
                                static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), AllPlanes, ZPixmap);
        if(m_x11_image == nullptr)
        {
            fprintf(stderr, "[X11Input::InputThread] Error: Can't get image (not using shared memory)!\n"
                             "    Usually this means the recording area is not completely inside the screen. Or did you change the screen resolution?");
            m_imageLock.unlock();
            return false;
        }
        m_pixFormat = checkPixelFormat(m_x11_image);
    }
    m_imageBuffer   = reinterpret_cast<uint8_t*>(m_x11_image->data);
    m_stride = m_x11_image->bytes_per_line;

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
            scr->m_shotOnScreen = m_screenRects[static_cast<size_t>(scr->m_shotOption.screenIndex)];
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
