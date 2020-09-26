#include "ScreenLayer.h"
#include <X11/extensions/shape.h>
//#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/Xlib.h>

ScreenLayer::ScreenLayer()
{
    memset(static_cast<void*>(&m_shotOption), 0, sizeof(m_shotOption));
    ScreenSource::static_init();
}

ScreenLayer::~ScreenLayer()
{
    close();
}

int32_t ScreenLayer::screenCount()
{
    ScreenSource::static_init();
    return static_cast<int32_t>(ScreenSource::screenRects().size());
}

QRect ScreenLayer::screenRect(int32_t i)
{
    ScreenSource::static_init();
    if(i >= 0 && i < ScreenSource::screenRects().size())
    {
        return ScreenSource::screenRects()[i];
    }
    return QRect();
}

QRect ScreenLayer::screenBound()
{
    ScreenSource::static_init();
    return ScreenSource::screenBound();
}

bool ScreenLayer::fullScreenImage(QImage& img)
{
    ScreenSource::static_init();
    ScreenSource    sour("");
    if ( sour.shotScreen(&ScreenSource::screenBound()) )
    {
        if (img.width() != sour.m_width || img.height() != sour.m_height || img.format() != sour.m_pixFormat)
        {
            img.detach();
            img = QImage(sour.m_width, sour.m_height, sour.m_pixFormat);
        }
        int line = std::min(sour.m_stride, img.bytesPerLine());
        for ( int i = 0; i < sour.m_height; ++i)
        {
            memcpy(img.scanLine(i), sour.m_imageBuffer + sour.m_stride * i, line);
        }
        return true;
    }
    return false;
}

QPoint ScreenLayer::mousePhysicalCoordinates()
{
    Window root, child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask_return;

    XQueryPointer(QX11Info::display(), QX11Info::appRootWindow(), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask_return);
    return QPoint(root_x, root_y);
}

QString ScreenLayer::windowName(Window wid)
{
    QString title;
    XTextProperty text;
    Window realWnd = findRealWindow(wid);
    if (realWnd) wid = realWnd;
    if (XGetWMName(ScreenSource::xDisplay(), wid, &text) && text.value)
    {
        int count = 0;
        char **list = nullptr;

        if (Success == XmbTextPropertyToTextList(ScreenSource::xDisplay(), &text, &list, &count))
        {
            for ( int i=0; i < count; ++i)
            {
                if (list[i] && list[i][0])
                {
                    title = QString::fromUtf8(list[i]);
                    break;
                }
            }
            XFreeStringList(list);
        }
        XFree(text.value);
    }

    if (title.isEmpty())
    {
        if (XGetWMIconName(ScreenSource::xDisplay(), wid, &text) && text.value)
        {
            int count = 0;
            char **list = nullptr;

            if (Success == XmbTextPropertyToTextList(ScreenSource::xDisplay(), &text, &list, &count))
            {
                for ( int i=0; i < count; ++i)
                {
                    if (list[i] && list[i][0])
                    {
                        title = QString::fromUtf8(list[i]);
                        break;
                    }
                }
                XFreeStringList(list);
            }
            XFree(text.value);
        }
        //fprintf(stderr, "XGetWMIconName=%s\n", title.toUtf8().data());
    }

    return title;
}

bool ScreenLayer::windowIsMinimized(Window wid)
{
    Atom actual_type;
    int actual_format;
    unsigned long items, bytes_after;
    Atom *atoms = nullptr;

    int result = XGetWindowProperty(ScreenSource::xDisplay(), wid,
                                    ScreenSource::atom_net_wm_state(),
            0, 1024, false, XCB_ATOM_ATOM, &actual_type, &actual_format, &items,
            &bytes_after, reinterpret_cast<unsigned char**>(&atoms));

    if(result == Success)
    {
        Atom atomHide = ScreenSource::atom_net_wm_state_hidden();
        for ( int i = 0; i < items; ++i)
        {
            if ( atomHide == atoms[i])
            {
                XFree(atoms);
                return true;
            }
        }
    }
    XFree(atoms);
    return false;
}

void ScreenLayer::enum_window(Display*display, Window window, int depth)
{
    XTextProperty text;

    if ( nullptr == display ) display = ScreenSource::xDisplay();
    if ( 0 ==window)  window = ScreenSource::xRootWindow();
    XGetWMName(display, window, &text);

    Window realWindow = ScreenLayer::findRealWindow(window);

    XWindowAttributes attributes;
    if(XGetWindowAttributes(display, window, &attributes))
    {
        if (attributes.map_state == IsViewable)
        {
            for ( int i=0; i < depth; ++i)
                fprintf(stderr,"  ");
            fprintf(stderr,"[%d]id=0x%x / %x, Visual:0x%x, [%s]\n", depth, window, realWindow,
                    attributes.visual, ScreenLayer::windowName(window).toUtf8().data());

            int32_t dx, dy;
            Window atWid;
            if(XTranslateCoordinates(ScreenSource::xDisplay(), window, ScreenSource::xRootWindow(),
                                     0, 0, &dx, &dy, &atWid))
            {
                for ( int i=0; i <= depth; ++i)
                    fprintf(stderr,"  ");
                fprintf(stderr,"%s (%d[%d],%d[%d])-(%d,%d) border_width=%d\n",
                        ScreenLayer::windowIsMinimized(window) ? "Minimize" : "", dx, attributes.x,
                        dy,attributes.y, attributes.width, attributes.height, attributes.border_width);
            }
        }
    }
        Window root, parent;
        Window* children;
        unsigned int n;
        XQueryTree(display, window, &root, &parent, &children, &n);
        if (children)
        {
            for (int i = 0; i < n; ++i)
            {
                enum_window(display, children[i], depth + 1);
            }
            XFree(children);
        }

}

bool ScreenLayer::windowImage(Window wid)
{
    Display* xdisp = XOpenDisplay(nullptr);
    //重定向窗口绘制，这个是必须的。
    XCompositeRedirectWindow(xdisp, wid, CompositeRedirectAutomatic);
    XWindowAttributes attr;
    XGetWindowAttributes(xdisp, wid, &attr);
    int64_t tim = QDateTime::currentMSecsSinceEpoch();
    Pixmap pixmap = XCompositeNameWindowPixmap(xdisp, wid);
    XImage* image = XGetImage(xdisp, pixmap, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);

    tim = QDateTime::currentMSecsSinceEpoch() - tim;
    fprintf(stderr, "TIME:%d\n", int(tim));
    if (image)
    {
        QImage img;
        if (image->bits_per_pixel == 24)
            img = QImage((uchar*)image->data, attr.width, attr.height, image->bytes_per_line, QImage::Format_RGB888);
        else
            img = QImage((uchar*)image->data, attr.width, attr.height, image->bytes_per_line, QImage::Format_RGB32);
        img.save(QString("/home/guee/Pictures/%1.png").arg(QDateTime::currentMSecsSinceEpoch()));
        XDestroyImage(image);
    }
    XFreePixmap(xdisp, pixmap);

    XCompositeUnredirectWindow(ScreenSource::xDisplay(), wid, CompositeRedirectAutomatic);
}

ScreenLayer::Option ScreenLayer::posOnWindow(const QPoint &pos, Window exclude)
{
    Option opt;
    Window root, parent;
    Window* children;
    unsigned int n;
    QVector<Window> reals;
    opt.mode = unspecified;
    opt.windowId = 0;
    if (XQueryTree(ScreenSource::xDisplay(), ScreenSource::xRootWindow(),
               &root, &parent, &children, &n) )
    {
        for (int i = 0; i < int(n); ++i)
        {
            if (windowIsMinimized(children[i]))
            {
                --n;
                memmove(children + i, children + i + 1, sizeof(Window) * (n - uint32_t(i)));
                --i;
            }
            else
            {
                Window realWindow = findRealWindow(children[i]);
                if (None == realWindow || windowIsMinimized(realWindow))
                {
                    --n;
                    memmove(children + i, children + i + 1, sizeof(Window) * (n - uint32_t(i)));
                    --i;
                }
                else
                {
                    reals.append(realWindow);
                }
            }
        }
        for (int i = int(n-1); i >= 0; --i)
        {
            if (children[i] == exclude || reals[i] == exclude) continue;
            XWindowAttributes attributes;
            if(!XGetWindowAttributes(ScreenSource::xDisplay(), children[i], &attributes))
                continue;
            if (attributes.map_state != IsViewable)
                continue;
            opt.windowId = children[i];
            opt.margins = QMargins(attributes.border_width, attributes.border_width, attributes.border_width, attributes.border_width);
            opt.geometry = QRect(attributes.x, attributes.y, attributes.width, attributes.height);
            QRect outer = opt.geometry + opt.margins;
            QRect inner = opt.geometry;
            if (!outer.contains(pos))
                continue;
            if (ScreenSource::screenRects().size() > i && outer == inner)
            {
                auto scrIt = std::find(ScreenSource::screenRects().begin(), ScreenSource::screenRects().end(), outer);
                if ( opt.margins.isNull() && scrIt != ScreenSource::screenRects().end() )
                {
                    opt.mode = specScreen;
                    opt.screenIndex = static_cast<int32_t>(scrIt - ScreenSource::screenRects().begin());
                    break;
                }
            }
            if (reals[i] != opt.windowId)
            {
                //opt.windowId = realWindow;
                int offsetX, offsetY;
                Window child;
                if(XTranslateCoordinates(ScreenSource::xDisplay(), reals[i], ScreenSource::xRootWindow(), 0, 0, &offsetX, &offsetY, &child)
                         && XGetWindowAttributes(ScreenSource::xDisplay(), reals[i], &attributes))
                {
                    inner = QRect(offsetX, offsetY, attributes.width, attributes.height);
                }
            }
            if ( inner != outer && inner.contains(pos) )
            {
                opt.mode = clientOfWindow;
                opt.margins.setLeft(opt.geometry.left() - inner.left());
                opt.margins.setTop(opt.geometry.top() - inner.top());
                opt.margins.setRight(inner.right() - opt.geometry.right());
                opt.margins.setBottom(inner.bottom() - opt.geometry.bottom());
            }
            else
            {
                opt.mode = specWindow;
                opt.margins.setLeft(opt.geometry.left() - outer.left());
                opt.margins.setTop(opt.geometry.top() - outer.top());
                opt.margins.setRight(outer.right() - opt.geometry.right());
                opt.margins.setBottom(outer.bottom() - opt.geometry.bottom());
            }
            break;
        }
        XFree(children);
    }
    return opt;
}

//QRect ScreenLayer::mapToLogicaRect(const QRect &rect)
//{
//    for(QScreen *screen :  QApplication::screens()) {
//        QRect geometry = screen->geometry();
//        qreal ratio = screen->devicePixelRatio();
//        QRect physical_geometry(geometry.x(), geometry.y(),
//                                static_cast<int32_t>(lrint(geometry.width() * ratio)),
//                                static_cast<int32_t>(lrint(geometry.height() * ratio)));
//        if(physical_geometry.contains(rect.center()))
//        {
//            return QRect(
//                        static_cast<int32_t>(lrint((geometry.x() + (rect.x() - physical_geometry.x()) / ratio))),
//                        static_cast<int32_t>(lrint((geometry.y() + (rect.y() - physical_geometry.y()) / ratio))),
//                        static_cast<int32_t>(lrint(rect.width() / ratio)),
//                        static_cast<int32_t>(lrint(rect.height() / ratio)));
//        }
//    }
//    return QRect();
//}

bool ScreenLayer::setShotOption(const ScreenLayer::Option &opt)
{
    m_shotOption = opt;
    return true;
}

BaseSource *ScreenLayer::onCreateSource(const QString &sourceName)
{
    Q_UNUSED(sourceName)
    return new ScreenSource(layerType());

}

void ScreenLayer::onReleaseSource(BaseSource* source)
{
    if ( source != nullptr )
    {
        delete source;
    }
}

Window ScreenLayer::findRealWindow(Window window)
{
    Atom actual_type;
    int actual_format;
    unsigned long items, bytes_after;
    unsigned char *data = nullptr;
    Window realWindow = None;
    //只有能取得 WM_STATE 属性的，才是与XServer session managers通信的窗口。
    if ( Success == XGetWindowProperty(ScreenSource::xDisplay(), window, ScreenSource::atom_wm_state(),
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

        for(unsigned int i = childcount; i > 0; ) {
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
