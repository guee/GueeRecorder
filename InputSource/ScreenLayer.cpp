#include "ScreenLayer.h"


#include <X11/Xlib.h>
#include <QX11Info>

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
    ScreenSource::getWindowName(wid, title);
    return title;
}

ScreenLayer::Option ScreenLayer::posOnWindow(const QPoint &pos, Window exclude)
{
    Option opt;
    int    visibledCount = 0;
    auto wnds = ScreenSource::getTopLevelWindows();
    for (int i = wnds.size() - 1; i >= 0; --i)
    {
        auto wnd = wnds[i];
        //qDebug() << i << "/" << wnds.size() << ":" << wnd.widTop << wnd.widReal << windowName(wnd.widReal);
        if (wnd.widTop == exclude || wnd.widReal == exclude) continue;
        if (ScreenSource::windowIsMinimized(wnd.widTop)) continue;
        XWindowAttributes attributes;
        if(!XGetWindowAttributes(ScreenSource::xDisplay(), wnd.widTop, &attributes))
            continue;
        if (attributes.map_state != IsViewable)
            continue;
        opt.widTop = wnd.widTop;
        opt.widReal = wnd.widReal;
        opt.margins = QMargins(attributes.border_width, attributes.border_width, attributes.border_width, attributes.border_width);
        opt.geometry = QRect(attributes.x, attributes.y, attributes.width, attributes.height);
        QRect outer = opt.geometry + opt.margins;
        QRect inner = opt.geometry;
        if (!outer.contains(pos))
            continue;
        if (ScreenSource::screenRects().size() > visibledCount && outer == inner)
        {
            auto scrIt = std::find(ScreenSource::screenRects().begin(), ScreenSource::screenRects().end(), outer);
            if ( opt.margins.isNull() && scrIt != ScreenSource::screenRects().end() )
            {
                opt.mode = specScreen;
                opt.screenIndex = static_cast<int32_t>(scrIt - ScreenSource::screenRects().begin());
                break;
            }
        }
        if (wnd.widTop != wnd.widReal)
        {
            int offsetX, offsetY;
            Window child;
            if(XTranslateCoordinates(ScreenSource::xDisplay(), wnd.widReal, ScreenSource::xRootWindow(), 0, 0, &offsetX, &offsetY, &child)
                     && XGetWindowAttributes(ScreenSource::xDisplay(), wnd.widReal, &attributes))
            {
                inner = QRect(offsetX, offsetY, attributes.width, attributes.height);
            }
        }
        qDebug() << i << "/" << wnds.size() << ":" << wnd.widTop << "/" << wnd.widReal << outer << inner << windowName(wnd.widReal);
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
    return opt;
}

bool ScreenLayer::open(const Option &opt)
{
    m_shotOption = opt;
    if (opt.mode >= ScreenLayer::specWindow && ScreenSource::xCompcapIsValid())
    {
        return BaseLayer::open(QString::number(ulong(opt.widTop)));
    }
    return BaseLayer::open();
}

BaseSource *ScreenLayer::onCreateSource(const QString &sourceName)
{
    return new ScreenSource(layerType(), sourceName);
}

void ScreenLayer::onReleaseSource(BaseSource* source)
{
    if ( source != nullptr )
    {
        delete source;
    }
}

void ScreenLayer::draw()
{
    XLockDisplay(ScreenSource::xDisplay());
    XSync(ScreenSource::xDisplay(), 0);
    BaseLayer::draw();
    XUnlockDisplay(ScreenSource::xDisplay());
}
