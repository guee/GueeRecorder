#include "GlScreenSelect.h"
#include "DialogSelectScreen.h"
#include "VideoSynthesizer.h"

GlScreenSelect::GlScreenSelect(QWidget *parent) :
    QOpenGLWidget(parent)
{
    m_lineColor = QVector4D( 0, 174.0f/255.0f, 1.0f, 1.0f );
    m_screenSize = ScreenLayer::screenBound().size();
    m_boxEditing = false;
    m_infoHide = true;
    setMouseTracking(true);
}

GlScreenSelect::~GlScreenSelect()
{
    if (m_repairTimerId)
    {
        killTimer(m_repairTimerId);
        m_repairTimerId = 0;
    }

    setMouseTracking(false);
    releaseKeyboard();

    makeCurrent();
    m_vboScreen.destroy();
    if ( m_mainTexture ) delete m_mainTexture;
    if ( m_programScreen ) delete m_programScreen;
    doneCurrent();
}

void GlScreenSelect::setMainScale(qreal scale)
{
    m_mainScale = scale;
    m_edgeSize = static_cast<int32_t>(lrint(m_edgeSize * m_mainScale - 1.0) / 2 * 2 + 1);

    QSize zoomBox( static_cast<int32_t>(lrint(4 * m_mainScale) * 29), static_cast<int32_t>(lrint(4 * m_mainScale) * 21) );
    int32_t edge = static_cast<int32_t>(lrint(3 * m_mainScale));
    m_InfoBox.setRect(0, 0, zoomBox.width() + edge * 2, zoomBox.height() + edge * 2);
    m_whiteBox = m_InfoBox.adjusted(1, 1, -1, -1);
    m_zoomBox.setRect(edge, edge, zoomBox.width(), zoomBox.height());

    m_fontSize = static_cast<int32_t>(lrint(16 * m_mainScale));
    m_infoTexBox.setRect(m_zoomBox.x(), m_InfoBox.height(), m_zoomBox.width(), m_fontSize );
    m_InfoBox.setHeight(m_InfoBox.height() + m_fontSize * 2 + 4);

    m_font = font();
    m_font.setPixelSize(12);
    m_font.setBold(false);
    m_font.setItalic(false);
}

void GlScreenSelect::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        m_pressKeyPos = ScreenLayer::mousePhysicalCoordinates();
        if ( m_boxEditing )
        {
            m_hitType = hitTest(m_pressKeyPos);
            if (m_hitType == Qt::NoSection)
            {
                m_infoHide = true;
            }
            else
            {
                emit editing(false, QRect());
                m_oldBox = m_realBox;
                m_infoHide = ( m_hitType == Qt::TitleBarArea );
                m_leftDown = true;
                editBoxWithMouse();
            }
        }
        else
        {
        }
    }
}

void GlScreenSelect::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint pos = ScreenLayer::mousePhysicalCoordinates();
    if (m_boxEditing)
    {
        if (event->button() == Qt::LeftButton && m_leftDown)
        {
            //选择区域编辑过程中放开了左键，完成选区编辑。
            if ( m_oldBox != m_realBox )
            {
                m_selOpt.mode = ScreenLayer::rectOfScreen;
                m_selOpt.geometry = m_realBox;
                m_selOpt.margins = QMargins();
            }

            m_infoHide = true;
            m_leftDown = false;
            m_lastMovePos = QPoint(-1, -1);
            emit editing(true, QRectF(m_realBox.left() / m_mainScale,
                                     m_realBox.top() / m_mainScale,
                                     m_realBox.width() / m_mainScale,
                                     m_realBox.height() / m_mainScale).toRect());
            update();
        }
        else if (event->button() == Qt::RightButton)
        {
            m_boxEditing = false;
            m_leftDown = false;
            m_infoHide = false;
            m_hitType = Qt::NoSection;
            m_selOpt.mode = ScreenLayer::unspecified;
            m_lastMovePos = QPoint(-1, -1);
            emit editing(false, QRect());
            setCursor(Qt::PointingHandCursor);
            getMouseOnWindow();
        }
    }
    else
    {
        if (event->button() == Qt::LeftButton)
        {
            //在选择窗口的状态下，放开左键，选区即为窗口
            if ( m_hitType == Qt::NoSection )
            {
                m_oldBox = m_realBox;
                m_hitType = hitTest(pos);
                setHitCursor(m_hitType);
                m_boxEditing = true;
                m_infoHide = true;
                m_leftDown = false;
                QRegion rgn(rect());
                parentWidget()->setMask(rgn);
                emit editing(true, QRectF(m_realBox.left() / m_mainScale,
                                         m_realBox.top() / m_mainScale,
                                         m_realBox.width() / m_mainScale,
                                         m_realBox.height() / m_mainScale).toRect());

                update();
            }
        }
        else if (event->button() == Qt::RightButton)
        {
            emit selected(true);
        }
    }
}

void GlScreenSelect::mouseMoveEvent(QMouseEvent *event)
{
    if ( m_boxEditing )
    {
        if ( event->buttons() & Qt::LeftButton )
        {
            if ( m_hitType != Qt::NoSection )
            {
                editBoxWithMouse();
            }
        }
        else
        {
            QPoint pos = ScreenLayer::mousePhysicalCoordinates();
            Qt::WindowFrameSection hit = hitTest(pos);
            if ( m_hitType != hit )
            {
                m_hitType = hit;
                setHitCursor(hit);
            }
        }
    }
    else
    {
        getMouseOnWindow();
        if ( event->buttons() & Qt::LeftButton )
        {
            QPoint pos = ScreenLayer::mousePhysicalCoordinates();
            //按下鼠标拖动一段距离，首次创建选择区域
            if ( abs(pos.x() - m_pressKeyPos.x()) >= 3 && abs(pos.y() - m_pressKeyPos.y()) >= 3 )
            {
                m_pressKeyPos = pos;
                m_boxEditing = true;
                m_leftDown = true;
                m_infoHide = false;
                m_realBox = QRect(m_pressKeyPos, pos).normalized();
                m_oldBox = m_realBox;
                m_selOpt.mode = ScreenLayer::rectOfScreen;
                m_selOpt.geometry = m_realBox;
                m_selOpt.margins = QMargins();
                m_hitType = hitTest(pos);
                setHitCursor(m_hitType);
                QRegion rgn(rect());
                parentWidget()->setMask(rgn);
                update();
            }
        }

    }
}

void GlScreenSelect::keyPressEvent(QKeyEvent *event)
{
    fprintf( stderr, "event->key()=%d\n", event->key());
    if(event->key() == Qt::Key_Escape)
    {
        emit selected(true);
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit selected(false);
        return;
    }
    event->ignore();
}

void GlScreenSelect::initializeGL()
{
    initializeOpenGLFunctions();

    ScreenLayer::fullScreenImage(m_imgScreen);
    m_mainTexture = new QOpenGLTexture(m_imgScreen);

    initVBO();

   // glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    if ( !vshader->compileSourceFile(":/Shaders/SelectScreen.vert") )
    {
        qDebug() << "compileSourceFile error log:" << vshader->log().toUtf8().data();
    }
    m_programScreen = new QOpenGLShaderProgram;
    m_programScreen->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/SelectScreen.vert" );
    m_programScreen->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/SelectScreen.frag" );
    m_programScreen->bindAttributeLocation("qt_Vertex", 0);
    m_programScreen->bindAttributeLocation("qt_MultiTexCoord0", 1);

    m_infoHide = false;
    getMouseOnWindow();
}

void GlScreenSelect::paintGL()
{
    glViewport(0, 0, ScreenLayer::screenBound().width(), ScreenLayer::screenBound().height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 m;
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
    m.translate(0.0f, 0.0f, -10.0f);
    m.rotate(0.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(0.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(0.0f, 0.0f, 0.0f, 1.0f);

    if (m_infoHide == false)
    {
        QPointF offset(ceil(6 * m_mainScale), ceil(24 * m_mainScale));

        for ( int32_t i = 0; i < ScreenLayer::screenCount(); ++i)
        {
            QRect geometry = ScreenLayer::screenRect(i);
            if ( geometry.contains(m_lastMovePos) )
            {
                if ( m_lastMovePos.y() + offset.y() + m_InfoBox.height() > geometry.bottom() )
                {
                    offset.setY( -(offset.y() + m_InfoBox.height()) );
                }
                if ( m_lastMovePos.x() + offset.x() + m_InfoBox.width() > geometry.right() )
                {
                    offset.setX( -(offset.x() + m_InfoBox.width()) );
                }
            }
        }
        m_InfoBox.moveTo(offset.toPoint() + m_lastMovePos);
    }

    m_programScreen->bind();
    m_programScreen->setUniformValue("qt_ModelViewProjectionMatrix", m);
    m_programScreen->setUniformValue("qt_Texture0", 0);
    m_programScreen->setUniformValue("lineColor", m_lineColor);
    m_programScreen->setUniformValue("bmpSize", m_screenSize);
    m_programScreen->setUniformValue("pointSize", float(m_edgeSize));
    m_programScreen->setUniformValue("topLeft", m_realBox.topLeft());
    m_programScreen->setUniformValue("bottomRight", m_realBox.bottomRight());
    m_programScreen->setUniformValue("boxEditing", m_boxEditing);
    m_programScreen->setUniformValue("showInfo", !m_infoHide);
    m_programScreen->setUniformValue("infoBoxTL", m_InfoBox.topLeft());
    m_programScreen->setUniformValue("infoBoxBR", m_InfoBox.bottomRight());
    m_programScreen->setUniformValue("whiteBoxTL", m_whiteBox.translated(m_InfoBox.topLeft()).topLeft());
    m_programScreen->setUniformValue("whiteBoxBR", m_whiteBox.translated(m_InfoBox.topLeft()).bottomRight());
    m_programScreen->setUniformValue("zoomBoxTL", m_zoomBox.translated(m_InfoBox.topLeft()).topLeft());
    m_programScreen->setUniformValue("zoomBoxSize", m_zoomBox.size());
    m_programScreen->setUniformValue("zoomPixel", m_zoomBox.height() / 21.0f);
    m_programScreen->setUniformValue("mousePos", m_lastMovePos);

    m_vboScreen.bind();
    m_programScreen->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_programScreen->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    m_programScreen->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_programScreen->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    m_mainTexture->bind(0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_mainTexture->release(0);
    m_vboScreen.release();
    m_programScreen->release();

    if (m_infoHide == false)
    {
        QPainter pnt;
        pnt.begin(this);
        pnt.setFont(m_font);
        QRectF draw = m_infoTexBox.translated(m_InfoBox.topLeft());
        draw.setRect(draw.x() / m_mainScale, draw.y() / m_mainScale, draw.width() / m_mainScale, draw.height() / m_mainScale);
        pnt.setPen(Qt::white);
        pnt.drawText(draw, Qt::AlignLeft | Qt::AlignVCenter, QString("%1 x %2").arg(m_realBox.width()).arg(m_realBox.height()));
        draw.adjust(0, m_fontSize / m_mainScale, 0, m_fontSize / m_mainScale);
        QColor co = m_imgScreen.pixelColor(m_lastMovePos);
        pnt.drawText(draw, Qt::AlignLeft | Qt::AlignVCenter, QString("RGB:(%1,%2,%3)").arg(co.red()).arg(co.green()).arg(co.blue()));
        pnt.end();
    }
}

void GlScreenSelect::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    //grabMouse();
    grabKeyboard();
    //setCursor(Qt::BlankCursor);
    //setCursor(Qt::PointingHandCursor);
    setHitCursor(m_hitType);
    if (!m_boxEditing)
    {
        getMouseOnWindow();
    }
    if (m_repairTimerId == 0)
    {
        m_repairTimerId = startTimer(10);
    }
    //update();
}

void GlScreenSelect::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit selected(false);
}

void GlScreenSelect::timerEvent(QTimerEvent *event)
{
    if ( m_repairTimerId == event->timerId())
    {
        if (m_boxEditing)
        {

        }
        else
        {
            getMouseOnWindow();
        }
    }
}

bool GlScreenSelect::saveSelectToFile(const QString &filePath)
{
    QImage img = m_imgScreen.copy(m_selOpt.geometry + m_selOpt.margins);
    return img.save(filePath, nullptr, 100);
}

void GlScreenSelect::getMouseOnWindow()
{
    QPoint phyPos = ScreenLayer::mousePhysicalCoordinates();
    QPoint wndPos = phyPos / m_mainScale;
    bool needUpdate = false;
    if (phyPos != m_lastMovePos)
    {
        m_lastMovePos = phyPos;
        m_currMouseRect.setRect(wndPos.x() - 1, wndPos.y() - 1, 3, 3);
        needUpdate = true;
    }

    ScreenLayer::Option opt = ScreenLayer::posOnWindow(phyPos, parentWidget()->winId());
//    qDebug() << "On" << opt.windowId << ",My" << this->winId() << ",Pa" << parentWidget()->winId();

    if ( opt.mode != m_selOpt.mode || opt.windowId != m_selOpt.windowId || opt.geometry != m_selOpt.geometry || opt.margins != m_selOpt.margins )
    {
        m_selOpt = opt;
        if ( opt.mode != ScreenLayer::unspecified )
        {
            m_realBox = opt.geometry + opt.margins;
        }
        else
        {
            m_realBox = QRect();
        }
        needUpdate = true;
    }
    if (needUpdate) update();

}

Qt::WindowFrameSection GlScreenSelect::hitTest(const QPoint &pos)
{
    Qt::WindowFrameSection hit = Qt::NoSection;
    int32_t minLen = m_edgeSize / 2 + 1 + 1;
    QPoint corner;
    corner = m_realBox.bottomRight();
    if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
    {
        hit = Qt::BottomRightSection;
        minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
    }
    corner = m_realBox.topRight();
    if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
    {
        hit = Qt::TopRightSection;
        minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
    }
    corner = m_realBox.topLeft();
    if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
    {
        hit = Qt::TopLeftSection;
        minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
    }
    corner = m_realBox.bottomLeft();
    if ( abs(corner.x() - pos.x()) < minLen && abs(corner.y() - pos.y()) < minLen )
    {
        hit = Qt::BottomLeftSection;
        minLen = qMin(abs(corner.x() - pos.x()), abs(corner.y() - pos.y()));
    }

    if (hit == Qt::NoSection)
    {
        int32_t  cl = 0;
        if ( pos.y() > m_realBox.top() && pos.y() < m_realBox.bottom() )
        {
            cl = abs(pos.x() - m_realBox.right());
            if ( cl < minLen )
            {
                hit = Qt::RightSection;
                minLen = cl;
            }
            cl = abs(pos.x() - m_realBox.left());
            if ( cl < minLen )
            {
                hit = Qt::LeftSection;
                minLen = cl;
            }
        }
        if ( pos.x() > m_realBox.left() && pos.x() < m_realBox.right() )
        {
            cl = abs(pos.y() - m_realBox.bottom());
            if ( cl < minLen )
            {
                hit = Qt::BottomSection;
                minLen = cl;
            }
            cl = abs(pos.y() - m_realBox.top());
            if ( cl < minLen )
            {
                hit = Qt::TopSection;
                minLen = cl;
            }
        }
    }

    if (hit == Qt::NoSection)
    {
        if ( pos.x() > m_realBox.left() && pos.x() < m_realBox.right() && pos.y() > m_realBox.top() && pos.y() < m_realBox.bottom() )
        {
            hit = Qt::TitleBarArea;
        }
    }
    return hit;
}

void GlScreenSelect::setHitCursor(Qt::WindowFrameSection hit)
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

void GlScreenSelect::editBoxWithMouse()
{
    QPoint pos = ScreenLayer::mousePhysicalCoordinates();
    if (pos == m_lastMovePos || m_leftDown == false)
    {
        return;
    }
    m_lastMovePos = pos;
    int32_t x = pos.x();
    int32_t y = pos.y();
    int32_t l = m_oldBox.left();
    int32_t t = m_oldBox.top();
    int32_t r = m_oldBox.right();
    int32_t b = m_oldBox.bottom();
    Qt::WindowFrameSection hit = m_hitType;
    switch(m_hitType)
    {
    case Qt::TitleBarArea:
        if (!checkOffsetX(l,x))
        {
            r = l + m_realBox.width() - 1;
        }
        else
        {
            checkOffsetX(r, x);
            l = r - m_realBox.width() + 1;
        }
        if (!checkOffsetY(t, y))
        {
            b = t + m_realBox.height() - 1;
        }
        else
        {
            checkOffsetY(b, y);
            t = b - m_realBox.height() + 1;
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
    m_realBox.setRect(l, t, r - l + 1, b - t + 1);
    m_selOpt.geometry = m_realBox;
    if ( m_hitType != hit )
    {
        m_hitType = hit;
        setHitCursor(hit);
        m_pressKeyPos = pos;
        m_oldBox = m_realBox;
    }
    update();
}

void GlScreenSelect::initVBO()
{
    static const int coords[4][3] = {
        { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 }
    };
    QVector<GLfloat> vertData;
    for (int j = 0; j < 4; ++j) {
        // vertex position
        vertData.append(static_cast<GLfloat>(0.5 * coords[j][0]));
        vertData.append(static_cast<GLfloat>(0.5 * coords[j][1]));
        vertData.append(static_cast<GLfloat>(0.5 * coords[j][2]));
        // texture coordinate
        vertData.append(j == 0 || j == 3);
        vertData.append(j == 2 || j == 3);
    }

    m_vboScreen.create();
    m_vboScreen.bind();
    m_vboScreen.allocate(vertData.constData(),
                         vertData.count() * static_cast<int>(sizeof(GLfloat)));
}


