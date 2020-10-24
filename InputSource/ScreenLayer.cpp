#include "ScreenLayer.h"
#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/extensions/Xfixes.h>

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
    ScreenSource* ss = (ScreenSource*)m_resource;
    if (ss == nullptr || !ss->m_hasImage || !ss->m_isVisable) return;
    XFixesCursorImage* ci;
    QOpenGLTexture* tex;

    if (ss->m_isXCompcapMode)
    {
        XLockDisplay(ScreenSource::xDisplay());
        XSync(ScreenSource::xDisplay(), 0);
        BaseLayer::draw();
        XUnlockDisplay(ScreenSource::xDisplay());
        ci = ss->m_cursorImage2;
        tex = ss->m_cursorTexture2;
    }
    else
    {
        BaseLayer::draw();
        ci = ss->m_cursorImage1;
        tex = ss->m_cursorTexture1;
    }
    if (ci == nullptr || tex == nullptr) return;
    QRect curRect(ci->x - ci->xhot, ci->y - ci->yhot, ci->width, ci->height);
    QRect curDraw = curRect.intersected(m_shotOnScreen);
    if (curDraw.isNull()) return;
    qreal xScale = m_realBoxOnView.width() / m_shotOnScreen.width();
    qreal yScale = m_realBoxOnView.height() / m_shotOnScreen.height();
    QRectF onView(m_realBoxOnView.x() + (curDraw.x() - m_shotOnScreen.x()) * xScale,
                           m_realBoxOnView.y() + (curDraw.y() - m_shotOnScreen.y()) * yScale,
                           curDraw.width() * xScale,
                           curDraw.height() * yScale);
    float onTextL = float(curDraw.x() - curRect.x()) / float(curRect.width());
    float onTextT = float(curDraw.y() - curRect.y()) / float(curRect.height());
    float onTextR = onTextL + float(curDraw.width()) / float(curRect.width());
    float onTextB = onTextT + float(curDraw.height()) / float(curRect.height());

    VertexArritb    vertex[4] = {{QVector3D(float(onView.right()), float(onView.y()), 0.0f), QVector2D(onTextR, onTextT)},
                                 {QVector3D(float(onView.x()), float(onView.y()), 0.0f), QVector2D(onTextL, onTextT)},
                                 {QVector3D(float(onView.x()), float(onView.bottom()), 0.0f), QVector2D(onTextL, onTextB)},
                                 {QVector3D(float(onView.right()), float(onView.bottom()), 0.0f), QVector2D(onTextR, onTextB)}};
    if (m_vboCursor == nullptr)
    {
        m_vboCursor = new QOpenGLBuffer();
        m_vboCursor->create();
        m_vboCursor->bind();
        m_vboCursor->allocate(&vertex, 5 * 4 * sizeof(GLfloat));
    }
    else
    {
        m_vboCursor->bind();
        m_vboCursor->write(0, &vertex, 5 * 4 * sizeof(GLfloat));
    }


    m_program->bind();
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->setUniformValue("yuvFormat", 0);
    m_program->setUniformValue("textureSize", ci->width, ci->height);
    m_program->setUniformValue("opaque", false);
    m_program->setUniformValue("transparence", 1.0f);

    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    tex->bind(0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
