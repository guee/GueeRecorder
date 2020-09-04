#include "ScreenLayer.h"

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
    return static_cast<int32_t>(ScreenSource::m_screenRects.size());
}

QRect ScreenLayer::screenRect(int32_t i)
{
    ScreenSource::static_init();
    if(i >= 0 && i < ScreenSource::m_screenRects.size())
    {
        return ScreenSource::m_screenRects[i];
    }
    return QRect();
}

QRect ScreenLayer::screenBound()
{
    ScreenSource::static_init();
    return ScreenSource::m_screenBound;
}

bool ScreenLayer::fullScreenImage(QImage& img)
{
    ScreenSource::static_init();
    ScreenSource    sour("");
    if ( sour.shotScreen(&ScreenSource::m_screenBound) )
    {
        img = QImage(sour.m_width, sour.m_height, sour.m_pixFormat);
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

ScreenLayer::Option ScreenLayer::posOnWindow(const QPoint& pos)
{
    Option opt;
    ScreenSource::static_init();
    int32_t cx, cy;
    if(XTranslateCoordinates(ScreenSource::m_x11_Display, ScreenSource::m_x11_RootWindow, ScreenSource::m_x11_RootWindow, pos.x(), pos.y(), &cx, &cy, &opt.windowId))
    {
        XWindowAttributes attributes;
        if(opt.windowId != None && XGetWindowAttributes(ScreenSource::m_x11_Display, opt.windowId, &attributes))
        {
            opt.geometry = QRect(attributes.x, attributes.y, attributes.width, attributes.height);
            opt.margins = QMargins(attributes.border_width, attributes.border_width, attributes.border_width, attributes.border_width);
            auto scrIt = std::find(ScreenSource::m_screenRects.begin(), ScreenSource::m_screenRects.end(), opt.geometry);
            if ( opt.margins.isNull() && scrIt != ScreenSource::m_screenRects.end() )
            {
                Window root, parent, *childs;
                unsigned int childcount;
                if(XQueryTree(ScreenSource::m_x11_Display, opt.windowId, &root, &parent, &childs, &childcount))
                {
                    if (root == parent)
                    {
                        opt.mode = specScreen;
                        opt.screenIndex = static_cast<int32_t>(scrIt - ScreenSource::m_screenRects.begin());
                        return opt;
                    }
                }
            }
            QRect outer = opt.geometry;
            QRect inner = opt.geometry - opt.margins;
            Window realWindow = findRealWindow(opt.windowId);
            if(realWindow != None)
            {
                Window child;
                if(XTranslateCoordinates(ScreenSource::m_x11_Display, realWindow, ScreenSource::m_x11_RootWindow, 0, 0, &cx, &cy, &child)
                         && XGetWindowAttributes(ScreenSource::m_x11_Display, realWindow, &attributes))
                {
                    inner = QRect(cx, cy, attributes.width, attributes.height);
                }

//                Atom actual_type;
//                int actual_format;
//                unsigned long items, bytes_left;
//                int32_t *data = nullptr;
//                int result = XGetWindowProperty(m_x11_Display, realWindow,
//                                                XInternAtom(m_x11_Display, "_NET_FRAME_EXTENTS", true),
//                                                0, 4, false, AnyPropertyType, &actual_type, &actual_format, &items,
//                                                &bytes_left, reinterpret_cast<unsigned char**>(&data));
//                if(result == Success)
//                {
//                    if(actual_type == XCB_ATOM_CARDINAL && items == 4 && bytes_left == 0 && actual_format == 32)
//                    {
//                        Window child;
//                        QMargins margins(data[0], data[2], data[1], data[3]);
//                        if(XTranslateCoordinates(m_x11_Display, realWindow, m_x11_RootWindow, 0, 0, &cx, &cy, &child)
//                                 && XGetWindowAttributes(m_x11_Display, realWindow, &attributes))
//                        {
//                            opt.geometry = QRect(cx, cy, attributes.width, attributes.height);
//                            inner = opt.geometry;
//                            outer = opt.geometry + margins;
//                            opt.windowId = realWindow;
//                        }
//                        else
//                        {
//                            inner = outer - margins;
//                        }
//                    }
//                }
//                if(data != nullptr)
//                {
//                    XFree(data);
//                }
            }


            if ( inner.contains(pos) )
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
        }
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

// Tries to find the real window that corresponds to a top-level window (the actual window without window manager decorations).
// Returns None if it can't find the window (probably because the window is not handled by the window manager).
// Based on the xprop source code (http://cgit.freedesktop.org/xorg/app/xprop/tree/clientwin.c).
Window ScreenLayer::findRealWindow(Window window)
{
    // is this the real window?
    Atom actual_type;
    int actual_format;
    unsigned long items, bytes_left;
    unsigned char *data = nullptr;
    XGetWindowProperty(ScreenSource::m_x11_Display, window, XInternAtom(ScreenSource::m_x11_Display, "WM_STATE", true),
                       0, 0, false, AnyPropertyType, &actual_type, &actual_format, &items, &bytes_left, &data);
    if(data != nullptr)
        XFree(data);
    if(actual_type != None)
        return window;

    // get the child windows
    Window root, parent, *childs;
    unsigned int childcount;
    if(!XQueryTree(ScreenSource::m_x11_Display, window, &root, &parent, &childs, &childcount)) {
        return None;
    }

    // recursively call this function for all childs
    Window real_window = None;
    for(unsigned int i = childcount; i > 0; ) {
        --i;
        Window w = findRealWindow(childs[i]);
        if(w != None) {
            real_window = w;
            break;
        }
    }

    // free child window list
    if(childs != nullptr)
        XFree(childs);

    return real_window;

}
