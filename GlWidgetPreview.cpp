
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
    m_clearColor = QVector4D(0.0, 0.0, 0.0, 1.0);
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
    if (m_layerTools)
    {
        delete m_layerTools;
        m_layerTools = nullptr;
    }
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
    if (m_program == nullptr) return;
    //QOpenGLFramebufferObject::bindDefault();
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(m_clearColor.x(), m_clearColor.y(), m_clearColor.z(), m_clearColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LINE_SMOOTH);

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

    QSizeF potSize( m_edgeSize / m_viewportSize.width(), m_edgeSize / m_viewportSize.height() );


    if ( m_enterLayer )
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

       // glRectd(x0, y0, x1, y1);

      //  glRectd(x0, y0, x1, y1);
       // glBegin(GL_LINES);
       //     glVertex2f(0.0, 0.0);
       //     glVertex2f(1.5f, 0.0f);
       // glEnd();

}
void GlWidgetPreview::resizeGL(int width, int height)
{
    Q_UNUSED(width)
    Q_UNUSED(height)
    fixOffsetAsScreen();
    float sx = static_cast<float>((m_viewportSize.width() - m_displayOfSceeen.width())) / m_displayOfSceeen.width();
    float sy = static_cast<float>((m_viewportSize.height() - m_displayOfSceeen.height())) / m_displayOfSceeen.height();
    if (m_program == nullptr) return;
    if (m_program->bind())
    {
        QMatrix4x4 m;
        m.ortho(-1.0f - sx, +1.0f + sx, +1.0f + sy, -1.0f - sy, -1.0f, 1.0f);
        m_matrixView = m;
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
            m_viewportSize.setWidth(qFloor(width() * m_screenScale));
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

void GlWidgetPreview::setVideoObject(VideoSynthesizer *videoObj)
{
    m_video = videoObj;
    if ( m_layerTools == nullptr )
    {
        m_layerTools = new FormLayerTools(m_video, this);
        m_layerTools->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        connect( m_layerTools, &FormLayerTools::removeLayer, this, &GlWidgetPreview::on_layerToolbox_removeLayer );
        connect( m_layerTools, &FormLayerTools::selectLayer, this, &GlWidgetPreview::on_layerToolbox_selectLayer );
        connect( m_layerTools, &FormLayerTools::movedLayer, this, &GlWidgetPreview::on_layerToolbox_movedLayer );
    }
}

void GlWidgetPreview::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
       // ui.widgetContents->hide();
        QPoint pos = ScreenLayer::mousePhysicalCoordinates() - m_displayOfSceeen.topLeft();
        hitTest(pos);
        //if (m_hitType != Qt::NoSection)
        {
            m_boxOfPressKey = m_boxOfEditing;
            m_posOfPressKey = pos;
        }
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
        }
        if (m_editingLayer)
        {
            int32_t x = pos.x();
            int32_t y = pos.y();
            int32_t l = m_boxOfPressKey.left();
            int32_t t = m_boxOfPressKey.top();
            int32_t r = m_boxOfPressKey.right();
            int32_t b = m_boxOfPressKey.bottom();
            Qt::WindowFrameSection hit = m_hitType;
            switch(m_hitType)
            {
            case Qt::TitleBarArea:
                if (!checkOffsetX(l,x))
                {
                    r = l + m_boxOfPressKey.width() - 1;
                }
                else
                {
                    checkOffsetX(r, x);
                    l = r - m_boxOfPressKey.width() + 1;
                }
                if (!checkOffsetY(t, y))
                {
                    b = t + m_boxOfPressKey.height() - 1;
                }
                else
                {
                    checkOffsetY(b, y);
                    t = b - m_boxOfPressKey.height() + 1;
                }
                break;
            case Qt::LeftSection:
                checkOffsetX(l,x);
                if (l > r)
                {
                    swap(l,r);
                    hit = Qt::RightSection;
                }
                break;
            case Qt::RightSection:
                checkOffsetX(r,x);
                if (l > r)
                {
                    swap(l,r);
                    hit = Qt::LeftSection;
                }
                break;
            case Qt::TopSection:
                checkOffsetY(t,y);
                if (t > b)
                {
                    swap(t,b);
                    hit = Qt::BottomSection;
                }
                break;
            case Qt::BottomSection:
                checkOffsetY(b,y);
                if (t > b)
                {
                    swap(t,b);
                    hit = Qt::TopSection;
                }
                break;
            case Qt::TopLeftSection:
                checkOffsetY(t,y);
                checkOffsetX(l,x);
                if ( t > b && l > r )
                {
                    swap(l, r);
                    swap(t, b);
                    hit = Qt::BottomRightSection;
                }
                else if (t > b)
                {
                    swap(t, b);
                    hit = Qt::BottomLeftSection;
                }
                else if (l > r)
                {
                    swap(l, r);
                    hit = Qt::TopRightSection;
                }
                break;
            case Qt::TopRightSection:
                checkOffsetY(t,y);
                checkOffsetX(r,x);
                if ( t > b && l > r )
                {
                    swap(l, r);
                    swap(t, b);
                    hit = Qt::BottomLeftSection;
                }
                else if (t > b)
                {
                    swap(t, b);
                    hit = Qt::BottomRightSection;
                }
                else if (l > r)
                {
                    swap(l, r);
                    hit = Qt::TopLeftSection;
                }
                break;
            case Qt::BottomLeftSection:
                checkOffsetY(b,y);
                checkOffsetX(l,x);
                if ( t > b && l > r )
                {
                    swap(l, r);
                    swap(t, b);
                    hit = Qt::TopRightSection;
                }
                else if (t > b)
                {
                    swap(t, b);
                    hit = Qt::TopLeftSection;
                }
                else if (l > r)
                {
                    swap(l, r);
                    hit = Qt::BottomRightSection;
                }
                break;
            case Qt::BottomRightSection:
                checkOffsetY(b,y);
                checkOffsetX(r,x);
                if ( t > b && l > r )
                {
                    swap(l, r);
                    swap(t, b);
                    hit = Qt::TopLeftSection;
                }
                else if (t > b)
                {
                    swap(t, b);
                    hit = Qt::TopRightSection;
                }
                else if (l > r)
                {
                    swap(l, r);
                    hit = Qt::BottomLeftSection;
                }
                break;
            default:
                return;
            }
            m_boxOfEditing.setRect(l, t, r - l + 1, b - t + 1);
            m_boxEnterLayer = m_boxOfEditing;
            m_editingLayer->setRect(m_boxOfEditing.x() * 1.0 / m_displayOfSceeen.width(),
                                    m_boxOfEditing.y() * 1.0 / m_displayOfSceeen.height(),
                                    m_boxOfEditing.width() * 1.0 / m_displayOfSceeen.width(),
                                    m_boxOfEditing.height() * 1.0 / m_displayOfSceeen.height());
            if ( m_hitType != hit )
            {
                m_hitType = hit;
                setHitCursor(hit);
                m_posOfPressKey = pos;
                m_boxOfPressKey = m_boxOfEditing;
            }
            update();
            m_video->immediateUpdate();
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
        hitTest(pos);
        if (m_hitType == Qt::NoSection && m_enterLayer)
        {
            m_editingLayer = m_enterLayer;
            m_boxOfEditing = m_boxEnterLayer;
        }
        resetToolboxPos(false);
    }
    else if (event->button() == Qt::RightButton )
    {
        m_editingLayer = nullptr;
        resetToolboxPos(true);
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


    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(BaseLayer::VertexArritb));
    m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector3D), 2, sizeof(BaseLayer::VertexArritb));
}

void GlWidgetPreview::resetToolboxPos(bool mustHide)
{
    if (m_editingLayer && !mustHide)
    {
        QRect rt(parentWidget()->mapToGlobal(QPoint(0,0)),
                 parentWidget()->frameSize());

        m_layerTools->setGeometry(rt.right(), rt.top(), m_layerTools->width(), rt.height());
        m_layerTools->setStyleIsLeft(false);

        m_layerTools->setCurrLayer(m_editingLayer);
        m_layerTools->show();
    }
    else
    {
        m_layerTools->hide();
    }
}

void GlWidgetPreview::hitTest(const QPoint &pos)
{
    Qt::WindowFrameSection hit = Qt::NoSection;
    if (m_editingLayer)
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

void GlWidgetPreview::on_layerToolbox_removeLayer(BaseLayer *layer)
{
    if (layer)
    {
        if (layer == m_enterLayer)
            m_enterLayer = nullptr;
        if (layer == m_editingLayer)
            m_editingLayer = nullptr;
        layer->destroy(layer);
        m_video->immediateUpdate();
    }

}

void GlWidgetPreview::on_layerToolbox_selectLayer(BaseLayer *layer)
{
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

void GlWidgetPreview::on_layerToolbox_movedLayer(BaseLayer *layer)
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
