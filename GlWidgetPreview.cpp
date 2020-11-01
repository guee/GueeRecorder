
#include "GlWidgetPreview.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QFileDialog>

#include "InputSource/ScreenLayer.h"

GlWidgetPreview::GlWidgetPreview(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_clearColor = QVector4D(0.0, 0.05f, 0.1f, 1.0);
    m_vertex[0].vert = QVector3D(1.0, -1.0, 0.0);
    m_vertex[1].vert = QVector3D(-1.0, -1.0, 0.0);
    m_vertex[2].vert = QVector3D(-1.0, 1.0, 0.0);
    m_vertex[3].vert = QVector3D(1.0, 1.0, 0.0);

    m_vertex[0].text = QVector2D(1.0, 1.0);
    m_vertex[1].text = QVector2D(0.0, 1.0);
    m_vertex[2].text = QVector2D(0.0, 0.0);
    m_vertex[3].text = QVector2D(1.0, 0.0);

    setMouseTracking(true);
}

GlWidgetPreview::~GlWidgetPreview()
{
    makeCurrent();
    m_vbo.destroy();
    if (m_program)
    {
        delete m_program;
        m_program = nullptr;
    }
    doneCurrent();
}

QSize GlWidgetPreview::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GlWidgetPreview::sizeHint() const
{
    return QSize(200, 200);
}

void GlWidgetPreview::initializeGL()
{
    initializeOpenGLFunctions();
    emit initGL();




    makeObject();
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);

}

void GlWidgetPreview::paintGL()
{
    if (m_editingLayer && (!m_editingLayer->hasImage() || !m_editingLayer->isVisabled()))
    {
        m_editingLayer = nullptr;
    }
    if (m_enterLayer && (!m_enterLayer->hasImage() || !m_enterLayer->isVisabled()))
    {
        m_enterLayer = nullptr;
    }
    if (m_program == nullptr) return;
    //QOpenGLFramebufferObject::bindDefault();
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(m_clearColor.x(), m_clearColor.y(), m_clearColor.z(), m_clearColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //绘制非预览区域的网格背景
    glColor4ub(32, 32, 32, 255);
    for (int x = 0; x < m_viewportSize.width(); x += 32)
    {
        for (int y = m_viewportSize.height(); y > 0; y -= 32)
        {
            glRectf(x * 2.0f / m_viewportSize.width() -1.0f, y * 2.0f / m_viewportSize.height() - 1.0f,
                    (x + 16) * 2.0f / m_viewportSize.width() -1.0f, (y - 16) * 2.0f / m_viewportSize.height() - 1.0f);
            glRectf((x + 16) * 2.0f / m_viewportSize.width() -1.0f, (y - 16) * 2.0f / m_viewportSize.height() - 1.0f,
                    (x + 32) * 2.0f / m_viewportSize.width() -1.0f, (y - 32) * 2.0f / m_viewportSize.height() - 1.0f);
        }
    }
    //绘制图像超出预览区域的替代图像
    if (m_editingLayer)
    {
        QRect   rtOrg = m_boxOfEditing.translated(m_offsetOfViewport);
        //qDebug() <<"rtOrg" << rtOrg << ", view" << QRect(m_offsetOfViewport, m_viewportSize) << ", sect" << rtOrg.intersected(QRect(m_offsetOfViewport, m_viewportSize));
        if (rtOrg.intersected(QRect(m_offsetOfViewport, m_displayOfSceeen.size())) != rtOrg)
        {
            QRectF  rt(rtOrg);
            rt.translate(0.5, 0.5);
            rt.setCoords(rt.x() / m_viewportSize.width() * 2.0 - 1.0,
                         ( 1.0 - ( rt.bottom() - 1.0 ) / m_viewportSize.height() ) * 2.0 - 1.0,
                         ( rt.right() - 1.0 ) / m_viewportSize.width() * 2.0 - 1.0,
                         ( 1.0 - rt.y() / m_viewportSize.height() ) * 2.0 - 1.0);
            glColor4ub(64, 96, 128, 255);
            glRectd(rt.x(), rt.y(), rt.right(), rt.bottom());

            glEnable(GL_SCISSOR_TEST);
            glColor4ub(64, 128, 192, 255);
            glScissor( rtOrg.x(), m_viewportSize.height() - rtOrg.bottom(), rtOrg.width(), rtOrg.height() );
            QPointF stepLen(16.0 / m_viewportSize.width(), 16.0 / m_viewportSize.height());
            QPointF offset = rt.topLeft();
            int maxStep = qCeil(rt.width() / stepLen.x()) + qCeil(rt.height() / stepLen.y());
            for ( int i = 0; i < maxStep; i += 2 )
            {
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2d(offset.x(), rt.top());
                glVertex2d(rt.left(), offset.y());
                glVertex2d(offset.x() + stepLen.x(), rt.top());
                glVertex2d(rt.left(), offset.y() + stepLen.y());
                glEnd();
                offset += stepLen * 2;
            }
            glDisable(GL_SCISSOR_TEST);
        }
    }

    //绘制预览图像
    m_program->bind();
    m_program->setUniformValue("qt_ModelViewProjectionMatrix", m_matrixView);
    m_vbo.bind();


    if( m_sharedTextureId)
    {
        glBindTexture(GL_TEXTURE_2D, m_sharedTextureId);
    }
    //

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_program->release();

    //绘制控制点和边框线
    //glEnable(GL_LINE_SMOOTH);
    QSizeF potSize( m_edgeSize / m_viewportSize.width(), m_edgeSize / m_viewportSize.height() );
    if ( m_enterLayer && m_enterLayer != m_editingLayer )
    {
        QRectF rt = m_boxEnterLayer.translated(m_offsetOfViewport);
        rt.translate(0.5, 0.5);
        rt.setCoords(rt.x() / m_viewportSize.width() * 2.0 - 1.0,
                     ( 1.0 - ( rt.bottom() - 1.0 ) / m_viewportSize.height() ) * 2.0 - 1.0,
                     ( rt.right() - 1.0 ) / m_viewportSize.width() * 2.0 - 1.0,
                     ( 1.0 - rt.y() / m_viewportSize.height() ) * 2.0 - 1.0);
        glColor4ub(0, 0, 0, 128);
        glRectd(-1, -1, 1, rt.top());
        glRectd(-1, rt.bottom(), 1, 1);
        glRectd(-1, rt.top(), rt.left(), rt.bottom());
        glRectd(rt.right(), rt.top(), 1, rt.bottom());

        if (m_editingLayer != m_enterLayer )
        {
            glColor4ub(255, 255, 0, 255);
            glLineWidth(static_cast<float>(m_edgeSize));
            glBegin(GL_LINE_LOOP);
            glVertex2d(rt.left(), rt.top());
            glVertex2d(rt.left(), rt.bottom());
            glVertex2d(rt.right(), rt.bottom());
            glVertex2d(rt.right(), rt.top());
            glEnd();

            glRectd( rt.x() - potSize.width(), rt.y() - potSize.height(), rt.x() + potSize.width(), rt.y() + potSize.height() );
            glRectd( rt.x() - potSize.width(), rt.bottom() - potSize.height(), rt.x() + potSize.width(), rt.bottom() + potSize.height() );
            glRectd( rt.right() - potSize.width(), rt.bottom() - potSize.height(), rt.right() + potSize.width(), rt.bottom() + potSize.height() );
            glRectd( rt.right() - potSize.width(), rt.y() - potSize.height(), rt.right() + potSize.width(), rt.y() + potSize.height() );
        }
    }
   // glColor4ub(255, 255, 0, 255);


    if (m_editingLayer)
    {
        QRectF rt = m_boxOfEditing.translated(m_offsetOfViewport);
        rt.translate(0.5, 0.5);
        rt.setCoords(rt.x() / m_viewportSize.width() * 2.0 - 1.0,
                     ( 1.0 - ( rt.bottom() - 1.0 ) / m_viewportSize.height() ) * 2.0 - 1.0,
                     ( rt.right() - 1.0 ) / m_viewportSize.width() * 2.0 - 1.0,
                     ( 1.0 - rt.y() / m_viewportSize.height() ) * 2.0 - 1.0);
        glColor4ub(255, 255, 0, 255);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2d(rt.left(), rt.top());
        glVertex2d(rt.left(), rt.bottom());
        glVertex2d(rt.right(), rt.bottom());
        glVertex2d(rt.right(), rt.top());
        glEnd();

        glRectd( rt.x() - potSize.width(), rt.y() - potSize.height(), rt.x() + potSize.width(), rt.y() + potSize.height() );
        glRectd( rt.x() - potSize.width(), ( rt.y() + rt.bottom() ) * 0.5 - potSize.height(), rt.x() + potSize.width(), ( rt.y() + rt.bottom() ) * 0.5 + potSize.height() );
        glRectd( rt.x() - potSize.width(), rt.bottom() - potSize.height(), rt.x() + potSize.width(), rt.bottom() + potSize.height() );

        glRectd( ( rt.x() + rt.right() ) * 0.5 - potSize.width(), rt.y() - potSize.height(), ( rt.x() + rt.right() ) * 0.5 + potSize.width(), rt.y() + potSize.height() );
        glRectd( ( rt.x() + rt.right() ) * 0.5 - potSize.width(), rt.bottom() - potSize.height(), ( rt.x() + rt.right() ) * 0.5 + potSize.width(), rt.bottom() + potSize.height() );

        glRectd( rt.right() - potSize.width(), rt.bottom() - potSize.height(), rt.right() + potSize.width(), rt.bottom() + potSize.height() );
        glRectd( rt.right() - potSize.width(), ( rt.y() + rt.bottom() ) * 0.5 - potSize.height(), rt.right() + potSize.width(), ( rt.y() + rt.bottom() ) * 0.5 + potSize.height() );
        glRectd( rt.right() - potSize.width(), rt.y() - potSize.height(), rt.right() + potSize.width(), rt.y() + potSize.height() );
    }
    //glDisable(GL_LINE_SMOOTH);

}
void GlWidgetPreview::resizeGL(int width, int height)
{
    if (width > 0 && height > 0)
    {
        m_layerTools->setGeometry(width - m_layerTools->width(), 0 , m_layerTools->width(), height);
    }

    fixOffsetAsScreen();

    float sx = float((m_viewportSize.width() - m_displayOfSceeen.width())) / m_displayOfSceeen.width();
    float sy = float((m_viewportSize.height() - m_displayOfSceeen.height())) / m_displayOfSceeen.height();
    if (m_program == nullptr) return;
    if (m_program->bind())
    {
        QMatrix4x4 m;
        m.ortho(-1.0f - sx, +1.0f + sx, +1.0f + sy, -1.0f - sy, -1.0f, 1.0f);
        m_matrixView = m;
        m_program->setUniformValue("qt_ModelViewProjectionMatrix", m_matrixView);
    }
    if ( m_enterLayer )
    {
        QRectF rt = m_enterLayer->rect();
        m_boxEnterLayer.setRect(qRound(rt.x() * m_displayOfSceeen.width()),
                                qRound(rt.y() * m_displayOfSceeen.height()),
                                qRound(rt.width() * m_displayOfSceeen.width()),
                                qRound(rt.height() * m_displayOfSceeen.height()));
    }
    if ( m_editingLayer )
    {
        QRectF rt = m_editingLayer->rect();
        m_boxOfEditing.setRect(qRound(rt.x() * m_displayOfSceeen.width()),
                                qRound(rt.y() * m_displayOfSceeen.height()),
                                qRound(rt.width() * m_displayOfSceeen.width()),
                                qRound(rt.height() * m_displayOfSceeen.height()));
    }
}

void GlWidgetPreview::fixOffsetAsScreen()
{
    QRect rect = nativeParentWidget()->geometry();
    for(QScreen *screen : QApplication::screens())
    {
        QRect geometry = screen->geometry();
        qreal ratio = screen->devicePixelRatio();
        QRect physical_geometry(geometry.x(), geometry.y(),
                                qFloor(geometry.width() * ratio),
                                qFloor(geometry.height() * ratio));
        if(physical_geometry.contains(rect.center()))
        {
            m_screenScale = ratio;
            m_edgeSize = 5.0 * m_screenScale;
            if (m_layerTools->isVisible() || m_layerTools->windowIsPeg())
            {
                m_viewportSize.setWidth(qFloor((width() - m_layerTools->width()) * m_screenScale));
            }
            else
            {
                m_viewportSize.setWidth(qFloor(width() * m_screenScale));
            }
            m_viewportSize.setHeight(qFloor(height() * m_screenScale));

            QSizeF dispSize = QSizeF(m_video->width(), m_video->height()).scaled(m_viewportSize, Qt::KeepAspectRatio);
            m_offsetOfViewport.setX(qRound((m_viewportSize.width() - dispSize.width()) * 0.5));
            m_offsetOfViewport.setY(qRound((m_viewportSize.height() - dispSize.height()) * 0.5));
            QPoint pos = mapToGlobal(QPoint(0,0));
            m_displayOfSceeen.setRect( (qRound((pos.x() - geometry.x()) * ratio + geometry.x()) + m_offsetOfViewport.x()),
                                       (qRound((pos.y() - geometry.y()) * ratio + geometry.y()) + m_offsetOfViewport.y()),
                                       (m_viewportSize.width() - m_offsetOfViewport.x() * 2),
                                       (m_viewportSize.height() - m_offsetOfViewport.y() * 2));
            break;
        }
    }
}

void GlWidgetPreview::setVideoObject(VideoSynthesizer *videoObj, FormLayerTools* toolWidget)
{
    m_video = videoObj;
    m_layerTools = toolWidget;
}

void GlWidgetPreview::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
       // ui.widgetContents->hide();
        QPoint pos = ScreenLayer::mousePhysicalCoordinates() - m_displayOfSceeen.topLeft();
        hitTest(pos);
        m_boxOfPressKey = m_boxOfEditing;
        m_posOfPressKey = pos;
    }
}

void GlWidgetPreview::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    QPoint pos = ScreenLayer::mousePhysicalCoordinates() - m_displayOfSceeen.topLeft();
    if ( event->buttons() & Qt::LeftButton )
    {
        if (m_hitType == Qt::NoSection && m_enterLayer)
        {
            m_editingLayer = m_enterLayer;
            m_boxOfEditing = m_boxEnterLayer;
            m_boxOfPressKey = m_boxOfEditing;
            hitTest(m_posOfPressKey);
            notifySelectLayer(m_editingLayer);
        }
        if (m_editingLayer)
        {
            int32_t x = pos.x();
            int32_t y = pos.y();
            int32_t l = m_boxOfPressKey.left();
            int32_t t = m_boxOfPressKey.top();
            int32_t r = m_boxOfPressKey.right() + 1;
            int32_t b = m_boxOfPressKey.bottom() + 1;
            Qt::WindowFrameSection hit = m_hitType;

            switch(m_hitType)
            {
            case Qt::TitleBarArea:
                x = m_boxOfPressKey.center().x() + (x - m_posOfPressKey.x());
                y = m_boxOfPressKey.center().y() + (y - m_posOfPressKey.y());
                m_editingLayer->movCenter(x * 1.0 / m_displayOfSceeen.width(),
                                          y * 1.0 / m_displayOfSceeen.height());
                break;
            case Qt::LeftSection:
                l += x - m_posOfPressKey.x();
                if (m_editingLayer->movLeft(l * 1.0 / m_displayOfSceeen.width()) > 0)
                {
                    swap(l,r);
                    hit = Qt::RightSection;
                }
                break;
            case Qt::RightSection:
                r += x - m_posOfPressKey.x();
                if (m_editingLayer->movRight(r * 1.0 / m_displayOfSceeen.width()) > 0)
                {
                    swap(l,r);
                    hit = Qt::LeftSection;
                }
                break;
            case Qt::TopSection:
                t += y - m_posOfPressKey.y();
                if (m_editingLayer->movTop(t * 1.0 / m_displayOfSceeen.height()) > 0)
                {
                    swap(t,b);
                    hit = Qt::BottomSection;
                }
                break;
            case Qt::BottomSection:
                b += y - m_posOfPressKey.y();
                if (m_editingLayer->movBottom(b * 1.0 / m_displayOfSceeen.height()) > 0)
                {
                    swap(t,b);
                    hit = Qt::TopSection;
                }
                break;
            case Qt::TopLeftSection:
                t += y - m_posOfPressKey.y();
                l += x - m_posOfPressKey.x();
                switch(m_editingLayer->movTopLeft(t * 1.0 / m_displayOfSceeen.height(), l * 1.0 / m_displayOfSceeen.width()))
                {
                case 1:
                    swap(l, r); hit = Qt::TopRightSection;
                    break;
                case 2:
                    swap(t, b); hit = Qt::BottomLeftSection;
                    break;
                case 3:
                    swap(l, r); swap(t, b); hit = Qt::BottomRightSection;
                    break;
                }
                break;
            case Qt::TopRightSection:
                t += y - m_posOfPressKey.y();
                r += x - m_posOfPressKey.x();
                switch(m_editingLayer->movTopRight(t * 1.0 / m_displayOfSceeen.height(), r * 1.0 / m_displayOfSceeen.width()))
                {
                case 1:
                    swap(l, r); hit = Qt::TopLeftSection;
                    break;
                case 2:
                    swap(t, b); hit = Qt::BottomRightSection;
                    break;
                case 3:
                    swap(l, r); swap(t, b); hit = Qt::BottomLeftSection;
                    break;
                }
                break;
            case Qt::BottomLeftSection:
                b += y - m_posOfPressKey.y();
                l += x - m_posOfPressKey.x();
                switch(m_editingLayer->movBottomLeft(b * 1.0 / m_displayOfSceeen.height(), l * 1.0 / m_displayOfSceeen.width()))
                {
                case 1:
                    swap(l, r); hit = Qt::BottomRightSection;
                    break;
                case 2:
                    swap(t, b); hit = Qt::TopLeftSection;
                    break;
                case 3:
                    swap(l, r); swap(t, b); hit = Qt::TopRightSection;
                    break;
                }
                break;
            case Qt::BottomRightSection:
                b += y - m_posOfPressKey.y();
                r += x - m_posOfPressKey.x();
                switch(m_editingLayer->movBottomRight(b * 1.0 / m_displayOfSceeen.height(), r * 1.0 / m_displayOfSceeen.width()))
                {
                case 1:
                    swap(l, r); hit = Qt::BottomLeftSection;
                    break;
                case 2:
                    swap(t, b); hit = Qt::TopRightSection;
                    break;
                case 3:
                    swap(l, r); swap(t, b); hit = Qt::TopLeftSection;
                    break;
                }
                break;
            default:
                return;
            }
            if ( m_hitType != hit )
            {
                m_hitType = hit;
                setHitCursor(hit);
                m_posOfPressKey = pos;
                m_boxOfPressKey.setRect(l, t, r - l, b - t);
            }
            update();
        }
    }
    else
    {
        hitTest(pos);
    }
}

void GlWidgetPreview::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton )
    {
        QPoint pos = ScreenLayer::mousePhysicalCoordinates() - m_displayOfSceeen.topLeft();
        if (m_editingLayer && m_hitType != Qt::NoSection)
        {
            hitTest(pos);
            return;
        }
        hitTest(pos);
        if (m_hitType == Qt::NoSection && m_enterLayer)
        {
            m_editingLayer = m_enterLayer;
            m_boxOfEditing = m_boxEnterLayer;
        }
        notifySelectLayer(m_editingLayer);
    }
    else if (event->button() == Qt::RightButton )
    {
        m_editingLayer = nullptr;
        notifySelectLayer(m_editingLayer);
        QPoint pos = ScreenLayer::mousePhysicalCoordinates() - m_displayOfSceeen.topLeft();
        hitTest(pos);
    }
}

void GlWidgetPreview::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_enterLayer = m_editingLayer;
    if (m_editingLayer)
    {
       // m_layerTools->show();
    }
}

void GlWidgetPreview::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_enterLayer = nullptr;
}

void GlWidgetPreview::makeObject()
{
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(&m_vertex, 4 * sizeof(BaseLayer::VertexArritb));

    QMatrix4x4 m;
    m.ortho(0.0f, +1.0f, +1.0f, 0.0f, -1.0f, 1.0f);
    m_matrixView = m;
    m_program = VideoSynthesizer::instance().programPool().createProgram("base");
    if (m_program == nullptr) return;
    m_program->bind();
    m_program->setUniformValue("qt_ModelViewProjectionMatrix", m_matrixView);
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->setUniformValue("transparence", 1.0f);

    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(BaseLayer::VertexArritb));
    m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector3D), 2, sizeof(BaseLayer::VertexArritb));
}

void GlWidgetPreview::hitTest(const QPoint &pos)
{
    Qt::WindowFrameSection hit = Qt::NoSection;
    if (m_editingLayer && m_editingLayer->hasImage() && m_editingLayer->isVisabled())
    {
        QRectF rt = m_editingLayer->rect();
        m_boxOfEditing.setRect(qRound(rt.x() * m_displayOfSceeen.width()),
                                    qRound(rt.y() * m_displayOfSceeen.height()),
                                    qRound(rt.width() * m_displayOfSceeen.width()),
                                    qRound(rt.height() * m_displayOfSceeen.height()));
        int32_t minLen = qFloor(m_edgeSize / 2) + 1 + 1;
        QPoint corner;
        corner = m_boxOfEditing.bottomRight();
        if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
        {
            hit = Qt::BottomRightSection;
            minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
        }
        corner = m_boxOfEditing.topRight();
        if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
        {
            hit = Qt::TopRightSection;
            minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
        }
        corner = m_boxOfEditing.topLeft();
        if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
        {
            hit = Qt::TopLeftSection;
            minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
        }
        corner = m_boxOfEditing.bottomLeft();
        if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
        {
            hit = Qt::BottomLeftSection;
            minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
        }

        if (hit == Qt::NoSection)
        {
            int32_t  cl = 0;
            if ( pos.y() > m_boxOfEditing.top() && pos.y() < m_boxOfEditing.bottom() )
            {
                cl = abs(pos.x() - m_boxOfEditing.right());
                if ( cl < minLen )
                {
                    hit = Qt::RightSection;
                    minLen = cl;
                }
                cl = abs(pos.x() - m_boxOfEditing.left());
                if ( cl < minLen )
                {
                    hit = Qt::LeftSection;
                    minLen = cl;
                }
            }
            if ( pos.x() > m_boxOfEditing.left() && pos.x() < m_boxOfEditing.right() )
            {
                cl = abs(pos.y() - m_boxOfEditing.bottom());
                if ( cl < minLen )
                {
                    hit = Qt::BottomSection;
                    minLen = cl;
                }
                cl = abs(pos.y() - m_boxOfEditing.top());
                if ( cl < minLen )
                {
                    hit = Qt::TopSection;
                    minLen = cl;
                }
            }
        }

        if (hit == Qt::NoSection)
        {
            if ( pos.x() > m_boxOfEditing.left() && pos.x() < m_boxOfEditing.right() && pos.y() > m_boxOfEditing.top() && pos.y() < m_boxOfEditing.bottom() )
            {
                hit = Qt::TitleBarArea;
            }
        }
    }

    if (hit == Qt::NoSection)
    {
        BaseLayer* layer = m_video->childLayer(QPointF(static_cast<qreal>(pos.x()) / m_displayOfSceeen.width(),
                                                       static_cast<qreal>(pos.y()) / m_displayOfSceeen.height()));
        if ( layer != m_enterLayer )
        {
            m_enterLayer = layer;
            if ( m_enterLayer )
            {
                QRectF rt = m_enterLayer->rect();
                m_boxEnterLayer.setRect(qRound(rt.x() * m_displayOfSceeen.width()),
                                        qRound(rt.y() * m_displayOfSceeen.height()),
                                        qRound(rt.width() * m_displayOfSceeen.width()),
                                        qRound(rt.height() * m_displayOfSceeen.height()));
            }
        }
        if (m_editingLayer && (!m_editingLayer->hasImage() || !m_editingLayer->isVisabled()))
        {
            m_editingLayer = m_enterLayer;
            m_boxOfEditing = m_boxEnterLayer;
        }
    }
    else
    {
        m_enterLayer = m_editingLayer;
        m_boxEnterLayer = m_boxOfEditing;
    }

    if ( hit != m_hitType )
    {
        m_hitType = hit;
        setHitCursor(hit);
    }
}

void GlWidgetPreview::setHitCursor(Qt::WindowFrameSection hit)
{
    switch(hit)
    {
    case Qt::LeftSection:
    case Qt::RightSection:
        setCursor(Qt::SizeHorCursor);
        break;
    case Qt::TopSection:
    case Qt::BottomSection:
        setCursor(Qt::SizeVerCursor);
        break;
    case Qt::TopLeftSection:
    case Qt::BottomRightSection:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Qt::TopRightSection:
    case Qt::BottomLeftSection:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Qt::TitleBarArea:
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void GlWidgetPreview::notifySelectLayer(BaseLayer *layer)
{
    qDebug() << "notifySelectLayer:" << layer;
    if (!m_layerTools->windowIsPeg())
    {
        if (layer)
        {
            if (m_layerTools->isHidden())
            {
                m_layerTools->show();
                resizeGL(width(), height());
                update();
            }
        }
        else
        {
            if (m_layerTools->isVisible())
            {
                m_layerTools->hide();
                resizeGL(width(), height());
                update();
            }
        }
    }
    emit selectLayer(layer);
}

void GlWidgetPreview::on_videoSynthesizer_frameReady(uint textureId)
{
    m_sharedTextureId = textureId;
    if (m_sharedTextureId)
    {
        update();
    }
    else
    {
        resizeGL(0,0);
    }
}

void GlWidgetPreview::on_layerRemoved(BaseLayer *layer)
{
    if (layer)
    {
        if (layer == m_enterLayer)
            m_enterLayer = nullptr;
        if (layer == m_editingLayer)
            m_editingLayer = nullptr;
    }
}

void GlWidgetPreview::on_selectLayer(BaseLayer *layer)
{
    if (layer && (!layer->hasImage() || !layer->isVisabled()))
        layer = nullptr;
    else
        notifySelectLayer(layer);
    if (layer == m_editingLayer)
    {
        m_enterLayer = nullptr;
        return;
    }
    m_editingLayer = layer;
    m_enterLayer = layer;
    if (layer)
    {
        QRectF rt = m_editingLayer->rect();
        m_boxOfEditing.setRect(qRound(rt.x() * m_displayOfSceeen.width()),
                                    qRound(rt.y() * m_displayOfSceeen.height()),
                                    qRound(rt.width() * m_displayOfSceeen.width()),
                                    qRound(rt.height() * m_displayOfSceeen.height()));
        m_boxEnterLayer = m_boxOfEditing;
    }
}

void GlWidgetPreview::on_layerMoved(BaseLayer *layer)
{
    if (layer && m_editingLayer == layer)
    {
        QRectF rt = m_editingLayer->rect();
        m_boxOfEditing.setRect(qRound(rt.x() * m_displayOfSceeen.width()),
                                    qRound(rt.y() * m_displayOfSceeen.height()),
                                    qRound(rt.width() * m_displayOfSceeen.width()),
                                    qRound(rt.height() * m_displayOfSceeen.height()));
        m_boxEnterLayer = m_boxOfEditing;
    }
}
