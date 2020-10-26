#include "ScreenSource.h"
#include "ScreenLayer.h"

#include <QX11Info>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/XShm.h>
#include <X11/Xatom.h>
#include <GL/glx.h>

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include <xcb/xproto.h>

Display* ScreenSource::m_display = nullptr;
int ScreenSource::m_screen = 0;
Window ScreenSource::m_rootWid = 0;
Visual* ScreenSource::m_visual = nullptr;
int ScreenSource::m_depth = 0;
bool ScreenSource::m_useShm = false;
QVector<QRect> ScreenSource::m_screenRects;
//QVector<QRect> ScreenSource::m_screenDeadRects;
QRect ScreenSource::m_screenBound;
bool ScreenSource::m_recordCursor = false;
bool ScreenSource::m_cursorUseable = false;

XFixesCursorImage* ScreenSource::m_cursorImage1 = nullptr;
XFixesCursorImage* ScreenSource::m_cursorImage2 = nullptr;
QOpenGLTexture* ScreenSource::m_cursorTexture1 = nullptr;
QOpenGLTexture* ScreenSource::m_cursorTexture2 = nullptr;

bool ScreenSource::m_xCompcapIsValid = false;
Atom ScreenSource::m_atom_wm_state = 0;
Atom ScreenSource::m_atom_net_wm_state = 0;
Atom ScreenSource::m_atom_net_wm_state_hidden = 0;
PFNGLXBINDTEXIMAGEEXTPROC ScreenSource::glXBindTexImageEXT = nullptr;
PFNGLXRELEASETEXIMAGEEXTPROC ScreenSource::glXReleaseTexImageEXT = nullptr;

static bool *curErrorTarget = nullptr;
static char curErrorText[200];
static int xerrorlock_handler(Display *disp, XErrorEvent *err)
{
    if (curErrorTarget)
        *curErrorTarget = true;

    XGetErrorText(disp, err->error_code, curErrorText, 200);

    return 0;
}
class XErrorLock {
    bool islock;
    bool goterr;
    XErrorHandler prevhandler;

public:
    XErrorLock()
    {
        goterr = false;
        islock = false;
        prevhandler = nullptr;

        lock();
    }
    ~XErrorLock()
    {
        unlock();
    }

    bool isLocked() {return islock;}

    void unlock()
    {
        if (islock) {
            XSync(ScreenSource::xDisplay(), 0);

            curErrorTarget = nullptr;
            XSetErrorHandler(prevhandler);
            prevhandler = nullptr;
            XUnlockDisplay(ScreenSource::xDisplay());
            islock = false;
        }
    }
    void lock()
    {
        if (!islock) {
            XLockDisplay(ScreenSource::xDisplay());
            XSync(ScreenSource::xDisplay(), 0);

            curErrorTarget = &goterr;
            curErrorText[0] = 0;
            prevhandler = XSetErrorHandler(xerrorlock_handler);

            islock = true;
        }
    }

    bool gotError()
    {
        if (!islock)
            return false;

        XSync(ScreenSource::xDisplay(), 0);

        bool res = goterr;
        goterr = false;
        return res;
    }
    std::string getErrorText() {return curErrorText;}
    void resetError()
    {
        if (islock)
            XSync(ScreenSource::xDisplay(), 0);

        goterr = false;
        curErrorText[0] = 0;
    }
};


ScreenSource::ScreenSource(const QString& typeName, const QString &sourceName)
    : BaseSource(typeName, sourceName)
{
    m_isOpaque = true;
    m_img = nullptr;
    memset( &m_shmInfo, 0, sizeof(m_shmInfo) );
    m_shmServerAttached = false;
    qDebug() << "ScreenSource 构造";
}

ScreenSource::~ScreenSource()
{
    onClose();

    qDebug() << "ScreenSource 析构";
}

bool ScreenSource::onOpen()
{
    if (!m_sourceName.isEmpty() && xCompcapIsValid())
    {
        QStringList lst = m_sourceName.split("\r\n");
        m_wid = lst.count() > 0 ? lst[0].toULong() : 0;
        m_windowName = lst.count() > 1 ? lst[1] : "";
        m_windowClass = lst.count() > 2 ? lst[2] : "";
        if (m_wid)
        {
            Window realWnd = findRealWindow(m_wid);
            if (realWnd == 0) return false;
            if (m_windowName.isEmpty())
            {
                getWindowName(realWnd, m_windowName);
            }
            if (m_windowClass.isEmpty())
            {
                getWindowClass(realWnd, m_windowClass);
            }
        }
        else
        {
            if (!m_windowName.isEmpty() || !m_windowClass.isEmpty())
            {
                m_wid = findTopWindow(m_windowName, m_windowClass);
            }
        }

        if (m_wid == 0 && m_windowName.isEmpty() && m_windowClass.isEmpty() )
        {
            return false;
        }
        m_timeCheck.start();
        m_isXCompcapMode = true;
    }
    else
    {
        m_isXCompcapMode = false;
        start();
    }
    return true;
}

bool ScreenSource::isSameSource(const QString &type, const QString &source)
{
    if (BaseSource::isSameSource(type, source))
    {
        return true;
    }
    if (m_typeName == type)
    {
        if (xCompcapIsValid())
        {
            QStringList lst = source.split("\r\n");
            Window wid = lst.count() > 0 ? lst[0].toULong() : 0;
            QString windowName = lst.count() > 1 ? lst[1] : "";
            QString windowClass = lst.count() > 2 ? lst[2] : "";

            bool sameName = !windowName.isEmpty() && windowName == m_windowName;
            if (sameName)
                sameName = windowClass.isEmpty() || windowClass == m_windowClass;

            if (m_wid != 0)
            {
                if (m_wid == wid || sameName)
                {
                    return true;
                }
            }
            else if (sameName)
            {
                 return true;
            }
        }
        else
        {
            return true;
        }
    }
    return false;
}

bool ScreenSource::onClose()
{
    m_semShot.release();
    wait();
    if (m_semShot.available()) m_semShot.acquire(m_semShot.available());
    if (m_wid)
    {
        releaseWindow();
    }
    else
    {
        freeImage();
    }
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
    if(m_display) return true;
    m_display = XOpenDisplay(nullptr); //QX11Info::display();
    if ( m_display == nullptr)
    {
        qCritical() << "XOpenDisplay(nullptr) 调用返回 nullptr!";
        return false;
    }

    m_screen = DefaultScreen(m_display);
    m_rootWid = RootWindow(m_display, m_screen);
    m_visual = DefaultVisual(m_display, m_screen);
    m_depth = DefaultDepth(m_display, m_screen);
    m_useShm = XShmQueryExtension(m_display);
    if(m_useShm)
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
        if(!XFixesQueryExtension(m_display, &event, &error))
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

    m_atom_wm_state = XInternAtom(ScreenSource::xDisplay(), "WM_STATE", true);
    m_atom_net_wm_state = XInternAtom(ScreenSource::xDisplay(), "_NET_WM_STATE", true);
    m_atom_net_wm_state_hidden = XInternAtom(ScreenSource::xDisplay(), "_NET_WM_STATE_HIDDEN", true);

    glXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)glXGetProcAddress((GLubyte*)"glXBindTexImageEXT");
    glXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)glXGetProcAddress((GLubyte*)"glXReleaseTexImageEXT");
    return true;
}

void ScreenSource::static_uninit()
{
    if(m_display != nullptr)
    {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }

}

bool ScreenSource::setRecordCursor(bool record)
{
    m_recordCursor = record;
    return true;
}

bool ScreenSource::isRecordCursor()
{
    return m_recordCursor;
}

bool ScreenSource::readScreenConfig()
{
    m_screenRects.clear();
    int event_base, error_base;
    if(XineramaQueryExtension(m_display, &event_base, &error_base))
    {
        int num_screens;
        XineramaScreenInfo *screens = XineramaQueryScreens(m_display, &num_screens);
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

QImage::Format ScreenSource::checkPixelFormat(const XImage* img)
{
    QImage::Format pixFmt = QImage::Format_Invalid;
    switch(img->bits_per_pixel)
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
        if(img->red_mask == 0xf800 && img->green_mask == 0x07e0 && img->blue_mask == 0x001f)
            pixFmt = QImage::Format_RGB16;
        else if(img->red_mask == 0x7c00 && img->green_mask == 0x03e0 && img->blue_mask == 0x001f)
            pixFmt = QImage::Format_RGB555;
        break;
    case 24:
        if(img->red_mask == 0xff0000 && img->green_mask == 0x00ff00 && img->blue_mask == 0x0000ff)
            pixFmt = QImage::Format_RGB888;
        else if(img->red_mask == 0x0000ff && img->green_mask == 0x00ff00 && img->blue_mask == 0xff0000)
            pixFmt = QImage::Format_RGB888; //??Qt 没有对应的格式。
        break;
    case 32:
        if(img->red_mask == 0x00ff0000 && img->green_mask == 0x0000ff00 && img->blue_mask == 0x000000ff)
            pixFmt = QImage::Format_ARGB32;
        else if(img->red_mask == 0x000000ff && img->green_mask == 0x0000ff00 && img->blue_mask == 0x00ff0000)
            pixFmt = QImage::Format_RGBA8888;
        else
            pixFmt = QImage::Format_ARGB32;
        break;
    default:
        pixFmt = QImage::Format_Invalid;
    }
    return pixFmt;
}

bool ScreenSource::xCompcapIsValid()
{
    if (m_xCompcapIsValid) return true;
    m_xCompcapIsValid = false;
    if (!static_init())
    {
        return false;
    }

    int eventBase, errorBase;
    if (!XCompositeQueryExtension(m_display, &eventBase, &errorBase)) {
        return false;
    }

    int major = 0, minor = 2;
    XCompositeQueryVersion(m_display, &major, &minor);

    if (major == 0 && minor < 2)
    {
        return false;
    }
    m_xCompcapIsValid = true;
    return true;
}

bool ScreenSource::ewmhIsSupported()
{
    if (!static_init()) return false;

    Atom netSupportingWmCheck =
        XInternAtom(m_display, "_NET_SUPPORTING_WM_CHECK", true);
    Atom actualType;
    int format = 0;
    unsigned long num = 0, bytes = 0;
    unsigned char *data = nullptr;
    Window ewmh_window = 0;

    int status = XGetWindowProperty(m_display, m_rootWid,
                    netSupportingWmCheck, 0L, 1L, false,
                    XA_WINDOW, &actualType, &format, &num,
                    &bytes, &data);

    if (status == Success) {
        if (num > 0) {
            ewmh_window = reinterpret_cast<Window*>(data)[0];
        }
        if (data) {
            XFree(data);
            data = nullptr;
        }
    }

    if (ewmh_window) {
        status = XGetWindowProperty(m_display, ewmh_window,
                        netSupportingWmCheck, 0L, 1L, false,
                        XA_WINDOW, &actualType, &format,
                        &num, &bytes, &data);
        if (status != Success || num == 0 ||
            ewmh_window != reinterpret_cast<Window*>(data)[0])
        {
            ewmh_window = 0;
        }
        if (status == Success && data) {
            XFree(data);
        }
    }
    return ewmh_window != 0;
}

bool ScreenSource::windowIsMinimized(Window wid)
{
    Atom actual_type;
    int actual_format;
    unsigned long items, bytes_after;
    Atom *atoms = nullptr;

    int result = XGetWindowProperty(m_display, wid,
                                    m_atom_net_wm_state,
            0, 1024, false, XCB_ATOM_ATOM, &actual_type, &actual_format, &items,
            &bytes_after, reinterpret_cast<unsigned char**>(&atoms));

    if(result == Success)
    {
        for ( int i = 0; i < items; ++i)
        {
            if ( m_atom_net_wm_state_hidden == atoms[i])
            {
                XFree(atoms);
                return true;
            }
        }
    }
    XFree(atoms);
    return false;
}

Window ScreenSource::findTopWindow(const QString &windowName, const QString &windowClass)
{
    if (windowName.isEmpty() && windowClass.isEmpty()) return 0;
    QVector<TopWindowInfo> wnds = getTopLevelWindows();
    for (auto w : wnds)
    {
        int find = 0;
        if (!windowName.isEmpty())
        {
            QString s;
            if (getWindowName(w.widReal, s) && s == windowName)
            {
                ++find;
            }
        }
        else
        {
            ++find;
        }
        if (!windowClass.isEmpty())
        {
            QString s;
            if (getWindowClass(w.widReal, s) && s == windowClass)
            {
                ++find;
            }
        }
        else
        {
            ++find;
        }
        if (find == 2)
        {
            return w.widTop;
        }
    }
    return 0;
}

Window ScreenSource::findRealWindow(Window window)
{
    Atom actual_type;
    int actual_format;
    unsigned long items, bytes_after;
    unsigned char *data = nullptr;
    Window realWindow = None;
    //只有能取得 WM_STATE 属性的，才是与XServer session managers通信的窗口。
    if ( Success == XGetWindowProperty(m_display, window, m_atom_wm_state,
                       0, 0, false, AnyPropertyType, &actual_type, &actual_format, &items, &bytes_after, &data))
    {
        if(data != nullptr)
            XFree(data);
        if(actual_type != None)
            return window;
        Window root, parent, *childs;
        unsigned int childcount;
        //查询子窗口失败，就返回 None
        if(!XQueryTree(ScreenSource::xDisplay(), window, &root, &parent, &childs, &childcount))
        {
            return None;
        }
        //继续枚举子窗口，直到找到第一个与XServer通信的窗口。
        for(unsigned int i = childcount; i > 0; )
        {
            --i;
            Window w = findRealWindow(childs[i]);
            if(w != None)
            {
                realWindow = w;
                break;
            }
        }
        if(childs != nullptr)
            XFree(childs);
    }
    return realWindow;
}

QVector<ScreenSource::TopWindowInfo> ScreenSource::getTopLevelWindows(bool queryTree)
{
    QVector<TopWindowInfo> windows;
    if (!static_init()) return windows;

    if (queryTree)
    {
        Window root, parent;
        Window* children = nullptr;
        unsigned int n;
        if (XQueryTree(m_display, m_rootWid,
                   &root, &parent, &children, &n) )
        {
            for (unsigned int i = 0; i < n; ++i)
            {
                TopWindowInfo wnd;
                wnd.widTop = children[i];
                wnd.widReal = findRealWindow(wnd.widTop);
                if (wnd.widReal)
                {
                    windows.push_back(wnd);
//                    QString name;
//                    getWindowName(wnd.widReal, name);
//                    qDebug() << i << "/" << n << ":" << wnd.widTop << "/" << wnd.widReal << name;
                }
            }
            XFree(children);
            children = nullptr;
        }

    }
    else if (ewmhIsSupported())
    {
        Atom netClList = XInternAtom(m_display, "_NET_CLIENT_LIST", true);
        Atom actualType;
        int format;
        unsigned long num, bytes;
        Window* children = nullptr;

        for (int i = 0; i < ScreenCount(m_display); ++i)
        {
            Window rootWin = RootWindow(m_display, i);

            int status = XGetWindowProperty(m_display, rootWin, netClList, 0L,
                            ~0L, false, AnyPropertyType,
                            &actualType, &format, &num,
                            &bytes, reinterpret_cast<uint8_t **>(&children));

            if (status != Success) continue;
            for (unsigned long i = 0; i < num; ++i)
            {
                TopWindowInfo wids;
                wids.widTop = children[i];
                wids.widReal = children[i];
                windows.push_back(wids);
            }
            XFree(children);
            children = nullptr;
        }
    }
    return windows;
}

bool ScreenSource::getWindowClass(Window wid, QString &windowClass)
{
    return getWindowString(wid, windowClass, "WM_CLASS");
}

bool ScreenSource::getWindowName(Window wid, QString &windowName)
{
    return getWindowString(wid, windowName, "_NET_WM_NAME");
}

bool ScreenSource::allocImage(uint width, uint height)
{
    if(m_shmServerAttached
            && m_img->width == static_cast<int>(width)
            && m_img->height == static_cast<int>(height))
    {
        return true;
    }
    freeImage();
    m_img = XShmCreateImage(m_display, m_visual, static_cast<uint>(m_depth),
                                  ZPixmap, nullptr, &m_shmInfo, width, height);
    if(m_img == nullptr)
    {
        qDebug() << "x11 不能创建共享内存!";
        return false;
    }
    m_pixFormat = checkPixelFormat(m_img);
    m_shmInfo.shmid = shmget(IPC_PRIVATE,
                                  static_cast<size_t>(m_img->bytes_per_line * m_img->height),
                                  IPC_CREAT | 0700);
    if(m_shmInfo.shmid == -1)
    {
        qDebug() << "x11 获取共享内存信息失败";
        return false;
    }
    m_shmInfo.shmaddr = reinterpret_cast<char*>(shmat(m_shmInfo.shmid, nullptr, SHM_RND));
    if(m_shmInfo.shmaddr == reinterpret_cast<char*>(-1))
    {
        qDebug() << "x11 不能附加到共享内存";
        return false;
    }
    m_img->data = m_shmInfo.shmaddr;
    if(!XShmAttach(m_display, &m_shmInfo))
    {
        qDebug() << "x11 不能附加到共享内存";
        return false;
    }
    m_shmServerAttached = true;
    return true;
}

void ScreenSource::freeImage()
{
    if(m_shmServerAttached)
    {
        XShmDetach(m_display, &m_shmInfo);
        m_shmServerAttached = false;
    }
    if(m_shmInfo.shmaddr != reinterpret_cast<char*>(-1))
    {
        shmdt(m_shmInfo.shmaddr);
        m_shmInfo.shmaddr = reinterpret_cast<char*>(-1);
    }
    if(m_shmInfo.shmid != -1)
    {
        shmctl(m_shmInfo.shmid, IPC_RMID, nullptr);
        m_shmInfo.shmid = -1;
    }
    if(m_img != nullptr)
    {
        XDestroyImage(m_img);
        m_img = nullptr;
    }
}

bool ScreenSource::captureWindow()
{
    if (0 == m_wid) return false;

    XSync(m_display, false);
    if (XGetWindowAttributes(m_display, m_wid, &m_attr))
    {
        XErrorLock xlock;
        XCompositeRedirectWindow(m_display, m_wid, CompositeRedirectAutomatic);
        if (xlock.gotError())
        {
            return false;
        }
        XSelectInput(m_display, m_wid,
                 StructureNotifyMask | ExposureMask |
                     VisibilityChangeMask);
        const int config_attrs[] = {GLX_BIND_TO_TEXTURE_RGBA_EXT,
                        GL_TRUE,
                        GLX_DRAWABLE_TYPE,
                        GLX_PIXMAP_BIT,
                        GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                        GLX_TEXTURE_2D_BIT_EXT,
                        GLX_DOUBLEBUFFER,
                        GL_FALSE,
                        None};
        int nelem = 0;
        GLXFBConfig *configs = glXChooseFBConfig(m_display, m_screen, config_attrs, &nelem);
        bool found = false;
        GLXFBConfig config;
        for (int i = 0; i < nelem; i++) {
            config = configs[i];
            XVisualInfo *visual = glXGetVisualFromFBConfig(m_display, config);
            if (!visual)
                continue;

            if (m_attr.depth != visual->depth) {
                XFree(visual);
                continue;
            }
            XFree(visual);
            found = true;
            break;
        }
        if (!found) {
            XFree(configs);
            onClose();
            return false;
        }
        bool draw_opaque = true;
        int inverted;
        glXGetFBConfigAttrib(m_display, config, GLX_Y_INVERTED_EXT, &inverted);

        xlock.resetError();
        m_pix = XCompositeNameWindowPixmap(m_display, m_wid);
        if (xlock.gotError())
        {
            m_pix = 0;
            XFree(configs);
            return false;
        }
        const int pixmap_attrs[] = {GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
                        GLX_TEXTURE_FORMAT_EXT,
                        GLX_TEXTURE_FORMAT_RGBA_EXT, None};
        m_glxPix = glXCreatePixmap(m_display, config, m_pix, pixmap_attrs);
        if (xlock.gotError())
        {
            XFreePixmap(m_display, m_pix);
            XFree(configs);
            m_pix = 0;
            m_glxPix = 0;
            return false;
        }
        XFree(configs);
        m_width = m_attr.width;
        m_height = m_attr.height;
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        if(!m_texture->create())
        {
            onClose();
            return false;
        }
        m_texture->bind();
        m_texture->setSize(m_width, m_height);
        m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        m_texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        glXBindTexImageEXT(m_display, m_glxPix, GLX_FRONT_LEFT_EXT, NULL);
        if (xlock.gotError())
        {
            XFreePixmap(m_display, m_pix);
            delete m_texture;
            m_texture = nullptr;
            m_pix = 0;
            m_glxPix = 0;
            return false;
        }
        for (auto it:m_layers)
        {
            ScreenLayer* scr = static_cast<ScreenLayer*>(it);
            if (scr->m_shotOption.mode == ScreenLayer::specWindow
                    || scr->m_shotOption.mode == ScreenLayer::clientOfWindow)
            {
                scr->m_shotOnScreen = QRect(m_attr.x, m_attr.y, m_attr.width, m_attr.height) + scr->m_shotOption.margins;
                //scr->m_shotOnScreen = m_screenBound.intersected(scr->m_shotOnScreen);
                scr->setRectOnSource(QRect(0, 0, m_width, m_height) + scr->m_shotOption.margins);
            }
        }
        return true;
    }
    return false;
}

void ScreenSource::releaseWindow()
{
    if (m_glxPix)
    {
        glXReleaseTexImageEXT(m_display, m_glxPix, GLX_FRONT_LEFT_EXT);
        glXDestroyPixmap(m_display, m_glxPix);
        m_glxPix = 0;
    }
    if (m_pix)
    {
        XFreePixmap(m_display, m_pix);
        m_pix = 0;
    }
    if (m_wid)
    {
        XCompositeUnredirectWindow(m_display, m_wid, CompositeRedirectAutomatic);
        XSelectInput(m_display, m_wid, 0);
        m_wid = 0;
    }
    if (m_texture)
    {
        delete m_texture;
        m_texture = nullptr;
    }
    XSync(m_display, false);
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
    //int64_t tim = QDateTime::currentMSecsSinceEpoch();
   // XLockDisplay(ScreenSource::xDisplay());
   // XSync(ScreenSource::xDisplay(), 0);

    if(m_useShm)
    {
        if ( !allocImage(uint32_t(m_width), uint32_t(m_height)) )
        {
            m_imageLock.unlock();
            return false;
        }

        if(!XShmGetImage(m_display, m_rootWid, m_img,
                         m_shotRect.x(), m_shotRect.y(), AllPlanes))
        {
            qWarning() << "截取x11屏幕失败。";
            m_imageLock.unlock();
            return false;
        }
    }
    else
    {
        if(m_img != nullptr)
        {
            XDestroyImage(m_img);
            m_img = nullptr;
        }
        m_img = XGetImage(m_display, m_rootWid,
                                m_shotRect.x(), m_shotRect.y(),
                                uint32_t(m_width), uint32_t(m_height), AllPlanes, ZPixmap);
        if(m_img == nullptr)
        {
            qWarning() << "截取x11屏幕失败。";
            m_imageLock.unlock();
            return false;
        }
        m_pixFormat = checkPixelFormat(m_img);
    }
//    XUnlockDisplay(ScreenSource::xDisplay());
//    tim = QDateTime::currentMSecsSinceEpoch() - tim;
//    fprintf(stderr, "TIME:%d\n", int(tim));

    m_imageBuffer   = reinterpret_cast<uint8_t*>(m_img->data);
    m_stride = m_img->bytes_per_line;
    m_imageLock.unlock();

    for (auto it:m_layers)
    {
        ScreenLayer* scr = static_cast<ScreenLayer*>(it);
        QRect rtOnSour = scr->m_shotOnScreen.translated(m_screenBound.left() - m_shotRect.left(), m_screenBound.top() - m_shotRect.top());
        if (sizeChanged || rtOnSour != scr->m_rectOnSource)
        {
            scr->setRectOnSource(rtOnSour);
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
            if (scr->m_shotOption.screenIndex < m_screenRects.count())
            {
                m_hasImage = true;
                scr->m_shotOnScreen = m_screenRects[scr->m_shotOption.screenIndex];
                bound |= scr->m_shotOnScreen;
            }
            else
            {
                m_hasImage = false;
            }
            break;
        case ScreenLayer::fullScreen:
            m_hasImage = true;
            scr->m_shotOnScreen = m_screenBound;
            bound |= scr->m_shotOnScreen;
            break;
        case ScreenLayer::rectOfScreen:
            scr->m_shotOnScreen = m_screenBound.intersected(scr->m_shotOption.geometry);
            if (scr->m_shotOnScreen.isEmpty())
            {
                m_hasImage = false;
            }
            else
            {
                m_hasImage = true;
                bound |= scr->m_shotOnScreen;
            }
            break;
        case ScreenLayer::clientOfWindow:
            if (windowIsMinimized(scr->m_shotOption.widTop))
            {
                m_hasImage = false;
                break;
            }
            m_hasImage = true;
            if (scr->m_shotOption.widReal)
            {
                XWindowAttributes attributes;
                if ( XGetWindowAttributes(m_display, scr->m_shotOption.widReal, &attributes) )
                {
                    int offsetX, offsetY;
                    Window child;
                    if (XTranslateCoordinates(m_display, scr->m_shotOption.widReal, m_rootWid, 0, 0, &offsetX, &offsetY, &child))
                    {
                        scr->m_shotOnScreen = QRect(offsetX, offsetY, attributes.width, attributes.height);
                        scr->m_shotOnScreen = m_screenBound.intersected(scr->m_shotOnScreen);
                        bound |= scr->m_shotOnScreen;
                        break;
                    }
                }
            }
            //此处不 break，如果失败则用带框架的窗口。
        case ScreenLayer::specWindow:
            if (windowIsMinimized(scr->m_shotOption.widTop))
            {
                m_hasImage = false;
                break;
            }
            m_hasImage = true;
            if (scr->m_shotOption.widTop)
            {
                XWindowAttributes attributes;
                if ( XGetWindowAttributes(m_display, scr->m_shotOption.widTop, &attributes) )
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

void ScreenSource::readyNextImage(int64_t next_timestamp)
{
    m_requestTimestamp = next_timestamp;
    if (m_sourceName.isEmpty())
    {
        m_semShot.release();
        //qDebug() << "updateToTexture()" << next_timestamp;
    }
    else
    {
        static QSet<Window> changedWindows;
        XLockDisplay(m_display);
        while (XEventsQueued(m_display, QueuedAfterReading) > 0)
        {
            XEvent ev;
            XNextEvent(m_display, &ev);
            if (ev.type == ConfigureNotify)
                changedWindows.insert(ev.xconfigure.event);
            else if (ev.type == MapNotify)
                changedWindows.insert(ev.xmap.event);
            else if (ev.type == Expose)
                changedWindows.insert(ev.xexpose.window);
            else if (ev.type == VisibilityNotify)
                changedWindows.insert(ev.xvisibility.window);
            else if (ev.type == DestroyNotify)
                changedWindows.insert(ev.xdestroywindow.event);
        }
        XUnlockDisplay(m_display);

        if (m_wid == 0)
        {
            if (m_timeCheck.elapsed() > 500)
            {
                m_wid = findTopWindow(m_windowName, m_windowClass);
                if (m_wid)
                {
                    m_sourceName = QString::number(m_wid);
                }
                m_timeCheck.start();
            }
        }


        if (m_wid)
        {
            auto it = changedWindows.find(m_wid);
            //如果在窗口被重置的列表中找到了窗口，就尝试重新初始化窗口纹理绑定
            if (it != changedWindows.end())
            {
                changedWindows.erase(it);
                if ( !windowIsMinimized(m_wid) )
                {
                    releaseWindow();
                    m_wid = m_sourceName.toULong();
                    if (!captureWindow())
                    {
                        releaseWindow();
                        m_timeCheck.start();
                        m_hasImage = false;
                    }
                    else
                    {
                        m_hasImage = true;
                        m_imageChanged = true;
                    }
                }
            }
            else if(m_glxPix == 0)
            {
                m_wid = m_sourceName.toULong();
                if (!captureWindow())
                {
                    releaseWindow();
                    m_timeCheck.start();
                    m_hasImage = false;
                }
                else
                {
                    m_imageChanged = true;
                    m_hasImage = true;
                }
            }
            if (m_wid && m_timeCheck.elapsed() > 1000)
            {
                Window realWnd = findRealWindow(m_wid);
                getWindowName(realWnd, m_windowName);
                getWindowClass(realWnd, m_windowClass);
                m_timeCheck.start();
            }
        }
    }
}

void ScreenSource::updateCursorTexture()
{
    if ( nullptr == m_display || !m_cursorUseable || !m_recordCursor )
    {
        releaseCursorTexture();
        return;
    }

    if (m_cursorImage1)
    {
        XFree(m_cursorImage1);
        m_cursorImage1 = nullptr;
    }
    m_cursorImage1 = XFixesGetCursorImage(m_display);
    if (m_cursorImage1 == nullptr) return;

    if (nullptr == m_cursorTexture1 || m_cursorTexture1->width() != m_cursorImage1->width || m_cursorTexture1->height() != m_cursorImage1->height)
    {
        if (m_cursorTexture1)
            delete m_cursorTexture1;
        m_cursorTexture1 = new QOpenGLTexture(QOpenGLTexture::Target2D);
        m_cursorTexture1->create();
        m_cursorTexture1->bind();
        m_cursorTexture1->setSize(m_cursorImage1->width, m_cursorImage1->height);
        m_cursorTexture1->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        m_cursorTexture1->setFormat(QOpenGLTexture::RGBA8_UNorm);
        m_cursorTexture1->allocateStorage(QOpenGLTexture::BGRA, QOpenGLTexture::UInt8);
    }
    else
    {
        m_cursorTexture1->bind();
    }
    int pixCount = m_cursorImage1->height * m_cursorImage1->width;
    ulong* orgPix = m_cursorImage1->pixels;
    uint* newPix = reinterpret_cast<uint*>(orgPix);
    for ( int i = 0; i < pixCount; ++i )
    {
        newPix[i] = orgPix[i] & 0xFFFFFFFF;
    }
    m_cursorTexture1->setData(QOpenGLTexture::BGRA, QOpenGLTexture::UInt8, static_cast<const void*>(newPix), nullptr);

    qSwap(m_cursorImage1, m_cursorImage2);
    qSwap(m_cursorTexture1, m_cursorTexture2);
}

void ScreenSource::releaseCursorTexture()
{
    if (m_cursorImage1)
    {
        XFree(m_cursorImage1);
        m_cursorImage1 = nullptr;
    }
    if (m_cursorImage2)
    {
        XFree(m_cursorImage2);
        m_cursorImage2 = nullptr;
    }
    if (m_cursorTexture1)
    {
        delete m_cursorTexture1;
        m_cursorTexture1 = nullptr;
    }
    if (m_cursorTexture2)
    {
        delete m_cursorTexture2;
        m_cursorTexture2 = nullptr;
    }
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
            }
        }
    }

}

void ScreenSource::drawCursor()
{
    XFixesCursorImage * ci = XFixesGetCursorImage(m_display);
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
            + (ovrRect.left() - m_shotRect.left()) * m_img->bits_per_pixel / 8;

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

bool ScreenSource::getWindowString(Window wid, QString &str, const char *atom)
{
    if (!static_init())
    {
        return false;
    }
    Atom netWmName = XInternAtom(m_display, atom, false);
    int count = 0;
    char **list = nullptr;
    XTextProperty text;

    XGetTextProperty(m_display, wid, &text, netWmName);

    if (!text.nitems)
    {
        if (!XGetWMName(m_display, wid, &text))
        {
            return false;
        }
    }

    if (!text.nitems)
        return false;

    if (text.encoding == XA_STRING)
    {
        str = QString::fromUtf8(reinterpret_cast<char*>(text.value));
    }
    else
    {
        int ret = XmbTextPropertyToTextList(m_display, &text, &list, &count);
        if (ret >= Success && count > 0 && *list)
        {
            str = QString::fromUtf8(*list);
            XFreeStringList(list);
        }
    }
    XFree(text.value);
    return true;

}


