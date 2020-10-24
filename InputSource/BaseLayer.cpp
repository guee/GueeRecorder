#include "BaseLayer.h"
#include "BaseSource.h"

QVector<BaseSource*>    BaseLayer::m_resPool;
QMutex BaseLayer::m_lockSources;

BaseLayer::BaseLayer()
{
    //从右下方开始，顺时针旋转绘制扇形，四个顶点。
    m_vertex[0].vert = QVector3D(1.0, -1.0, 0.0);
    m_vertex[0].text = QVector2D(1.0, 0.0);
    m_vertex[1].vert = QVector3D(-1.0, -1.0, 0.0);
    m_vertex[1].text = QVector2D(0.0, 0.0);
    m_vertex[2].vert = QVector3D(-1.0, 1.0, 0.0);
    m_vertex[2].text = QVector2D(0.0, 1.0);
    m_vertex[3].vert = QVector3D(1.0, 1.0, 0.0);
    m_vertex[3].text = QVector2D(1.0, 1.0);
    m_userdefOnView.setRect(-1, -1, 2.0, 2.0);
}

BaseLayer::~BaseLayer()
{
    close();
    onLayerRemoved(this);
}

const QString &BaseLayer::sourceName()
{
    if (m_resource)
    {
        return m_resource->m_sourceName;
    }
    static QString n;
    return n;
}

bool BaseLayer::open(const QString &sourceName)
{
    if ( m_resource )
    {
        if (m_resource->isSameSource(layerType(), sourceName))
        {
            if(m_status >= Opened)
            {
                return true;
            }
        }
        else
        {
            close();
        }
    }

    m_resource = findSource(layerType(), sourceName);
    if (m_resource == nullptr)
    {
        m_resource = onCreateSource(sourceName);
        if ( m_resource )
        {
            m_resource->setSourceFps(frameRate());
            if (m_resource->sourceOpen(this))
            {
                m_resPool.push_back(m_resource);
                m_status = Opened;
            }
            else
            {
                delete m_resource;
                m_resource = nullptr;
            }
        }
    }
    else
    {
        if (m_resource->sourceOpen(this))
        {
            m_status = Opened;
        }
        else
        {
            m_resource = nullptr;
        }
    }
    if (m_status >= Opened)
    {
        onLayerOpened(this);
        return true;
    }
    return false;
}

void BaseLayer::close()
{
    if ( m_resource )
    {
        m_status = NoOpen;
        if ( m_resource->sourceClose(this) )
        {
            //如果返回 true，表示已经没有被任何layer引用，需要删除对象
            int i = m_resPool.indexOf(m_resource);
            if (i >= 0)
            {
                m_resPool.remove(i);
            }
            onReleaseSource(m_resource);
        }
        m_resource = nullptr;
    }
}

void BaseLayer::onReleaseSource(BaseSource* source)
{
    if ( source != nullptr )
    {
        delete source;
    }
}

bool BaseLayer::play()
{
    if ( m_resource )
    {
        if(m_status == Palying) return true;
        if ( (m_status == Opened || m_status == Paused) && m_resource->sourcePlay(this) )
        {
            m_status = Palying;
            return true;
        }
    }
    return false;
}

bool BaseLayer::pause()
{
    if ( m_resource )
    {
        if(m_status == Paused) return true;
        if ( ( m_status == Palying || m_status == Opened ) && m_resource->sourcePause(this) )
        {
            m_status = Paused;
            return true;
        }
    }
    return false;
}

void BaseLayer::draw()
{
    if (m_resource == nullptr || !m_resource->m_hasImage || !m_resource->m_isVisable
            || m_program == nullptr || m_resource->m_texture == nullptr || !m_resource->m_texture->isCreated()) return;

    m_program->bind();
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->setUniformValue("yuvFormat", m_resource->m_intputYuvFormat);
    m_program->setUniformValue("textureSize", m_resource->m_texture->width(), m_resource->m_texture->height());
    m_program->setUniformValue("opaque", m_resource->m_isOpaque);
    m_program->setUniformValue("contrast", m_hslContrast);
    m_program->setUniformValue("bright", m_hslBright);
    m_program->setUniformValue("saturation", m_hslSaturation);
    m_program->setUniformValue("hue", m_hslHue);
    m_program->setUniformValue("hueDye", m_hslHueDye);
    m_program->setUniformValue("transparence", 1.0f - m_transparence);

    if ( m_vbo == nullptr )
    {
        m_vbo = new QOpenGLBuffer();
        m_vbo->create();
        m_vbo->bind();
        m_vbo->allocate(&m_vertex, 5 * 4 * sizeof(GLfloat));
        m_vertexChanged = false;
    }
    else
    {
        m_vbo->bind();
        if (m_vertexChanged)
        {
            m_vbo->write(0, &m_vertex, 5 * 4 * sizeof(GLfloat));
            m_vertexChanged = false;
        }
    }


    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    m_resource->m_texture->bind(0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

int32_t BaseLayer::stride() const
{
     return m_resource ? m_resource->m_stride : 0;
}

const void *BaseLayer::lockImage()
{
    if ( m_resource )
    {
        m_resource->m_imageLock.lock();
        int left = m_rectOnSourceInited ? m_rectOnSource.left() : 0;
        int top = m_rectOnSourceInited ? m_rectOnSource.top() : 0;
        const uint8_t* buf = m_resource->m_imageBuffer
            + m_resource->pixelBits(m_resource->m_pixFormat) * left / 8 + m_resource->m_stride * top;
        return buf;
    }
    return nullptr;
}

void BaseLayer::unlockImage()
{
    if ( m_resource )
    {
        m_resource->m_imageLock.unlock();
    }
}

bool BaseLayer::isVisabled()
{
    return (!m_parent || (m_resource && m_resource->m_isVisable));
}

bool BaseLayer::hasImage()
{
    return (!m_parent || (m_resource && m_resource->m_hasImage));
}

bool BaseLayer::moveToLayer(int32_t layer)
{
    return setParent(m_parent, layer);
}

bool BaseLayer::setParent(BaseLayer *parent, int32_t layer)
{
    if ( parent == nullptr )
    {
        return false;
    }
    if (m_parent)
    {
        auto &par = m_parent->lockChilds();
        int i = par.indexOf(this);
        if (i < 0)
            return false;
        if (parent == m_parent)
        {
            if (i == layer)
            {
                return true;
            }
        }
        par.remove(i);
        m_parent->unlockChilds();
    }
    auto &par = parent->lockChilds();
    layer = std::min(layer, par.size());
    layer = std::max(layer, 0);
    par.insert(par.begin() + layer, this);
    parent->unlockChilds();
    m_parent = parent;
    return true;
}

int32_t BaseLayer::childLayerCount()
{
    return static_cast<int32_t>(m_childs.size());
}

BaseLayer *BaseLayer::childLayer(int32_t i)
{
    BaseLayer* lay = nullptr;
    auto& cds = lockChilds();
    if (i >= 0 && i < cds.size())
    {
        lay = cds[i];
    }
    unlockChilds();
    return lay;
}

int32_t BaseLayer::layerIndex()
{
    if ( m_parent )
    {
        auto& cds = m_parent->lockChilds();
        for ( int i = 0; i < cds.size(); ++i)
        {
            if ( cds[i] == this )
            {
                m_parent->unlockChilds();
                return i;
            }
        }
        m_parent->unlockChilds();
    }
    return 0;
}

BaseLayer *BaseLayer::childLayer(const QPointF &pos, bool onlyChild, bool realBox)
{
    BaseLayer* inp = nullptr;
    if ( m_parent && ( m_resource == nullptr || !m_resource->m_hasImage || !m_resource->m_isVisable )) return nullptr;

    m_mutexChilds.lock();
    for ( auto i:m_childs )
    {
        inp = i->childLayer(pos, false, realBox);
        if (inp)
        {
            m_mutexChilds.unlock();
            return inp;
        }
    }
    m_mutexChilds.unlock();
    if (!onlyChild)
    {
        QPointF p((pos.x() - 0.5) * m_glViewportSize.width(), (pos.y() - 0.5) * m_glViewportSize.height());
        if ( realBox ? m_realBoxOnView.contains(p) : m_userdefOnView.contains(p) )
        {
            return this;
        }
    }
    return nullptr;
}

void BaseLayer::setRect(const QRectF& rect)
{
    m_fullViewport = false;
    if (rect.width() < (m_glViewportSize.width() / m_pixViewportSize.width())) return;
    if (rect.height() < (m_glViewportSize.height() / m_pixViewportSize.height())) return;
    m_userdefOnView.setX((rect.x() - 0.5) * m_glViewportSize.width());
    m_userdefOnView.setY((rect.y() - 0.5) * m_glViewportSize.height());
    m_userdefOnView.setWidth(rect.width() * m_glViewportSize.width());
    m_userdefOnView.setHeight(rect.height() * m_glViewportSize.height());
    onSizeChanged(this);
}

void BaseLayer::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_fullViewport = false;
    if (w < (m_glViewportSize.width() / m_pixViewportSize.width())) return;
    if (h < (m_glViewportSize.height() / m_pixViewportSize.height())) return;
    m_userdefOnView.setX((x - 0.5) * m_glViewportSize.width());
    m_userdefOnView.setY((y - 0.5) * m_glViewportSize.height());
    m_userdefOnView.setWidth(w * m_glViewportSize.width());
    m_userdefOnView.setHeight(h * m_glViewportSize.height());
    onSizeChanged(this);

}

void BaseLayer::movCenter(qreal x, qreal y)
{
    m_fullViewport = false;
    x = (x - 0.5) * m_glViewportSize.width();
    y = (y - 0.5) * m_glViewportSize.height();
    m_userdefOnView.moveCenter(QPointF(x, y));
    onSizeChanged(this);
}

int BaseLayer::movLeft(qreal left, bool realBox)
{
    int swapRound = 0;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        m_userdefOnView = m_realBoxOnView;
        swapRound = mov_Left(left, m_userdefOnView);
        if (0 > swapRound) return -1;
        qreal h = m_userdefOnView.width() * m_realBoxOnView.height() / m_realBoxOnView.width();
        if (h < (m_glViewportSize.height() / m_pixViewportSize.height())) return - 1;
        m_userdefOnView.setY(m_realBoxOnView.y() - (h - m_realBoxOnView.height()) * 0.5 );
        m_userdefOnView.setHeight(h);
    }
    else
    {
        swapRound = mov_Left(left, m_userdefOnView);
        if (0 > swapRound) return -1;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound;
}

int BaseLayer::movRight(qreal right, bool realBox)
{
    int swapRound = 0;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        m_userdefOnView = m_realBoxOnView;
        swapRound = mov_Right(right, m_userdefOnView);
        if (0 > swapRound) return -1;
        qreal h = m_userdefOnView.width() * m_realBoxOnView.height() / m_realBoxOnView.width();
        if (h < (m_glViewportSize.height() / m_pixViewportSize.height())) return -1;
        m_userdefOnView.setY(m_realBoxOnView.y() - (h - m_realBoxOnView.height()) * 0.5 );
        m_userdefOnView.setHeight(h);
    }
    else
    {
        swapRound = mov_Right(right, m_userdefOnView);
        if (0 > swapRound) return -1;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound;
}

int BaseLayer::movTop(qreal top, bool realBox)
{
    int swapRound = 0;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        m_userdefOnView = m_realBoxOnView;
        swapRound = mov_Top(top, m_userdefOnView);
        if (0 > swapRound)
        {
            return -1;
        }
        qreal w = m_userdefOnView.height() * m_realBoxOnView.width() / m_realBoxOnView.height();
        if (w < (m_glViewportSize.width() / m_pixViewportSize.width()))
        {
            m_userdefOnView = m_realBoxOnView;
            return -1;
        }
        m_userdefOnView.setX(m_realBoxOnView.x() - (w - m_realBoxOnView.width()) * 0.5 );
        m_userdefOnView.setWidth(w);
        qDebug() << m_userdefOnView;
    }
    else
    {
        swapRound = mov_Top(top, m_userdefOnView);
        if (0 > swapRound) return -1;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound;
}

int BaseLayer::movBottom(qreal bottom, bool realBox)
{
    int swapRound = 0;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        m_userdefOnView = m_realBoxOnView;
        swapRound = mov_Bottom(bottom, m_userdefOnView);
        qreal w = m_userdefOnView.height() * m_realBoxOnView.width() / m_realBoxOnView.height();
        if (w < (m_glViewportSize.width() / m_pixViewportSize.width())) return -1;
        m_userdefOnView.setX(m_realBoxOnView.x() - (w - m_realBoxOnView.width()) * 0.5 );
        m_userdefOnView.setWidth(w);
    }
    else
    {
        swapRound = mov_Bottom(bottom, m_userdefOnView);
        if (0 > swapRound) return -1;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound;
}

int BaseLayer::movTopLeft(qreal top, qreal left, bool realBox)
{
    QRectF box = (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource) ? m_realBoxOnView : m_userdefOnView;
    int swapRound1 = mov_Top(top, box);
    int swapRound2 = mov_Left(left, box);
    if (0 > swapRound1 || 0 > swapRound2) return -1;

    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        QSizeF s = m_realBoxOnView.size().scaled(box.size(), m_aspectRatioMode);
        if (s.width() < (m_glViewportSize.width() / m_pixViewportSize.width()) ||
            s.height() < (m_glViewportSize.height() / m_pixViewportSize.height()))
                return -1;
        m_userdefOnView = box;
        if (swapRound1)
            m_userdefOnView.setBottom(box.top() + s.height());
        else
            m_userdefOnView.setTop(box.bottom() - s.height());
        if (swapRound2)
            m_userdefOnView.setRight(box.left() + s.width());
        else
            m_userdefOnView.setLeft(box.right() - s.width());
    }
    else
    {
        m_userdefOnView = box;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound1 + swapRound2;
}

int BaseLayer::movTopRight(qreal top, qreal right, bool realBox)
{
    QRectF box = (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource) ? m_realBoxOnView : m_userdefOnView;
    int swapRound1 = mov_Top(top, box);
    int swapRound2 = mov_Right(right, box);
    if (0 > swapRound1 || 0 > swapRound2) return -1;

    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        QSizeF s = m_realBoxOnView.size().scaled(box.size(), m_aspectRatioMode);
        if (s.width() < (m_glViewportSize.width() / m_pixViewportSize.width()) ||
            s.height() < (m_glViewportSize.height() / m_pixViewportSize.height()))
                return -1;
        m_userdefOnView = box;
        if (swapRound1)
            m_userdefOnView.setBottom(box.top() + s.height());
        else
            m_userdefOnView.setTop(box.bottom() - s.height());
        if (swapRound2)
            m_userdefOnView.setLeft(box.right() - s.width());
        else
            m_userdefOnView.setRight(box.left() + s.width());
    }
    else
    {
        m_userdefOnView = box;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound1 + swapRound2;
}

int BaseLayer::movBottomLeft(qreal bottom, qreal left, bool realBox)
{
    QRectF box = (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource) ? m_realBoxOnView : m_userdefOnView;
    int swapRound1 = mov_Bottom(bottom, box);
    int swapRound2 = mov_Left(left, box);
    if (0 > swapRound1 || 0 > swapRound2) return -1;

    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        QSizeF s = m_realBoxOnView.size().scaled(box.size(), m_aspectRatioMode);
        if (s.width() < (m_glViewportSize.width() / m_pixViewportSize.width()) ||
            s.height() < (m_glViewportSize.height() / m_pixViewportSize.height()))
                return -1;
        m_userdefOnView = box;
        if (swapRound1)
            m_userdefOnView.setTop(box.bottom() - s.height());
        else
            m_userdefOnView.setBottom(box.top() + s.height());
        if (swapRound2)
            m_userdefOnView.setRight(box.left() + s.width());
        else
            m_userdefOnView.setLeft(box.right() - s.width());
    }
    else
    {
        m_userdefOnView = box;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound1 + swapRound2;
}

int BaseLayer::movBottomRight(qreal bottom, qreal right, bool realBox)
{
    QRectF box = (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource) ? m_realBoxOnView : m_userdefOnView;
    int swapRound1 = mov_Bottom(bottom, box);
    int swapRound2 = mov_Right(right, box);
    if (0 > swapRound1 || 0 > swapRound2) return -1;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        QSizeF s = m_realBoxOnView.size().scaled(box.size(), m_aspectRatioMode);
        if (s.width() < (m_glViewportSize.width() / m_pixViewportSize.width()) ||
            s.height() < (m_glViewportSize.height() / m_pixViewportSize.height()))
                return -1;
        m_userdefOnView = box;
        if (swapRound1)
            m_userdefOnView.setTop(box.bottom() - s.height());
        else
            m_userdefOnView.setBottom(box.top() + s.height());
        if (swapRound2)
            m_userdefOnView.setLeft(box.right() - s.width());
        else
            m_userdefOnView.setRight(box.left() + s.width());
    }
    else
    {
        m_userdefOnView = box;
    }
    m_fullViewport = false;
    onSizeChanged(this);
    return swapRound1 + swapRound2;
}

void BaseLayer::setWidth(qreal w, bool realBox)
{
    m_fullViewport = false;
    w *= m_glViewportSize.width();
    if (w < (m_glViewportSize.width() / m_pixViewportSize.width())) return;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        qreal h = w * m_realBoxOnView.height() / m_realBoxOnView.width();
        m_userdefOnView.setX(m_realBoxOnView.x() - (w - m_realBoxOnView.width()) * 0.5 );
        m_userdefOnView.setWidth(w);
        m_userdefOnView.setY(m_realBoxOnView.y() - (h - m_realBoxOnView.height()) * 0.5 );
        m_userdefOnView.setHeight(h);
    }
    else
    {
        m_userdefOnView.setX(m_userdefOnView.x() - (w - m_userdefOnView.width()) * 0.5 );
        m_userdefOnView.setWidth(w);
    }
    onSizeChanged(this);
}

void BaseLayer::setHeight(qreal h, bool realBox)
{
    m_fullViewport = false;
    h *= m_glViewportSize.height();
    if (h < (m_glViewportSize.height() / m_pixViewportSize.height())) return;
    if (realBox && m_aspectRatioMode != Qt::IgnoreAspectRatio && m_resource)
    {
        qreal w = h * m_realBoxOnView.width() / m_realBoxOnView.height();
        m_userdefOnView.setX(m_realBoxOnView.x() - (w - m_realBoxOnView.width()) * 0.5 );
        m_userdefOnView.setWidth(w);
        m_userdefOnView.setY(m_realBoxOnView.y() - (h - m_realBoxOnView.height()) * 0.5 );
        m_userdefOnView.setHeight(h);
    }
    else
    {
        m_userdefOnView.setY(m_userdefOnView.y() - (h - m_userdefOnView.height()) * 0.5 );
        m_userdefOnView.setHeight(h);
    }
    onSizeChanged(this);
}

void BaseLayer::fullViewport(bool full)
{
    if (m_fullViewport != full)
    {
        m_fullViewport = full;
        onSizeChanged(this);
    }
}

float BaseLayer::imgContrast()
{
    return m_hslContrast;
}

void BaseLayer::setImgContrast(float c)
{
    m_hslContrast = qMax(-1.0f, qMin(1.0f, c));
}

float BaseLayer::imgBright()
{
    return m_hslBright;
}

void BaseLayer::setImgBright(float b)
{
    m_hslBright = qMax(-1.0f, qMin(1.0f, b));
}

float BaseLayer::imgSaturation()
{
    return m_hslSaturation;
}

void BaseLayer::setImgSaturation(float s)
{
    m_hslSaturation = qMax(-1.0f, qMin(1.0f, s));
}

float BaseLayer::imgHue()
{
    return m_hslHue;
}

void BaseLayer::setImgHue(float h)
{
    m_hslHue = qMax(-1.0f, qMin(1.0f, h));
}

bool BaseLayer::imgHueDye()
{
    return m_hslHueDye;
}

void BaseLayer::setImgHueDye(bool d)
{
    m_hslHueDye = d;
}

float BaseLayer::imgTransparence()
{
    return m_transparence;
}

void BaseLayer::setImgTransparence(float t)
{
    m_transparence = qMax(0.0f, qMin(1.0f, t));
}

QRectF BaseLayer::rect(bool realBox) const
{
    if (realBox)
    {
        return QRectF( m_realBoxOnView.x() / m_glViewportSize.width() + 0.5,
                    m_realBoxOnView.y() / m_glViewportSize.height() + 0.5,
                    m_realBoxOnView.width() / m_glViewportSize.width(),
                    m_realBoxOnView.height() / m_glViewportSize.height());
    }
    return m_fullViewport ?
                QRectF(0.0, 0.0, 1.0, 1.0) :
                QRectF( m_userdefOnView.x() / m_glViewportSize.width() + 0.5,
                m_userdefOnView.y() / m_glViewportSize.height() + 0.5,
                m_userdefOnView.width() / m_glViewportSize.width(),
                        m_userdefOnView.height() / m_glViewportSize.height());
}

void BaseLayer::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    m_aspectRatioMode = mode;
    onSizeChanged(this);
}

Qt::AspectRatioMode BaseLayer::aspectRatioMode() const
{
    return m_aspectRatioMode;
}

void BaseLayer::setViewportSize(const QSizeF& glSize, const QSize& pixSize, bool childs)
{
    m_glViewportSize = glSize;
    m_pixViewportSize = pixSize;

    if (m_fullViewport)
    {
        m_userdefOnView.setRect(glSize.width() * -0.5, glSize.height() * -0.5, glSize.width(), glSize.height());
    }

    if (m_program)
    {
        QMatrix4x4 m;
        m.ortho(static_cast<float>(glSize.width() * -0.5), static_cast<float>(glSize.width() * 0.5),
                static_cast<float>(glSize.height() * 0.5), static_cast<float>(glSize.height() * -0.5),
                -1, 1);
        m_program->bind();
        m_program->setUniformValue("qt_ModelViewProjectionMatrix",m);
    }
    if (childs)
    {
        m_mutexChilds.lock();
        for (auto it:m_childs)
        {
            it->setViewportSize(glSize, pixSize, childs);
        }
        m_mutexChilds.unlock();
    }
    onSizeChanged(this);
}

void BaseLayer::setShaderProgram(QOpenGLShaderProgram *prog)
{

    m_program = prog;
    if (m_program)
    {
        setViewportSize(m_glViewportSize, m_pixViewportSize);
    }

}

bool BaseLayer::updateSourceTextures()
{
    bool ret = false;
    for (auto it:m_resPool)
    {
        if (it->updateToTexture())
        {
            ret = true;
        }
    }
    return ret;
}

void BaseLayer::readySourceNextImage(int64_t next_timestamp)
{
    for (auto it:m_resPool)
    {
        it->readyNextImage(next_timestamp);
    }
}

BaseSource *BaseLayer::findSource(const QString& typeName, const QString& sourceName)
{
    for (auto it:m_resPool)
    {
        if (it->isSameSource(typeName, sourceName) )
            return it;
    }
    return nullptr;
}

void BaseLayer::setSourcesFramerate(float fps)
{
    for (auto it:m_resPool)
    {
        it->setSourceFps(fps);
    }
}

void BaseLayer::onSizeChanged(BaseLayer* layer)
{
    if (layer == this)
    {
        if (m_resource == nullptr) return;

        QSizeF s(m_rectOnSourceInited ? m_rectOnSource.size() : QSize(m_resource->width(), m_resource->height()));
        QRectF u = m_userdefOnView;

        if (m_fullViewport)
        {
            u.setRect(m_glViewportSize.width() * -0.5, m_glViewportSize.height() * -0.5, m_glViewportSize.width(), m_glViewportSize.height());
        }
        s.scale(u.size(), m_aspectRatioMode);

        QRectF r(u.x() + ( u.width() - s.width() ) * 0.5,
                 u.y() + ( u.height() - s.height() ) * 0.5,
                 s.width(), s.height());

        m_realBoxOnView = r;

        m_vertex[0].vert.setX(static_cast<float>(r.right()));
        m_vertex[0].vert.setY(static_cast<float>(r.y()));

        m_vertex[1].vert.setX(static_cast<float>(r.x()));
        m_vertex[1].vert.setY(static_cast<float>(r.y()));

        m_vertex[2].vert.setX(static_cast<float>(r.x()));
        m_vertex[2].vert.setY(static_cast<float>(r.bottom()));

        m_vertex[3].vert.setX(static_cast<float>(r.right()));
        m_vertex[3].vert.setY(static_cast<float>(r.bottom()));

        m_vertexChanged = true;
    }
    if (m_parent) m_parent->onSizeChanged(layer);
}

void BaseLayer::setRectOnSource(const QRect& rect)
{
    m_rectOnSource = rect;
    m_vertex[0].text.setX((m_rectOnSource.right() + 1.0f) / m_resource->m_width);
    m_vertex[0].text.setY(m_rectOnSource.y() * 1.0f / m_resource->m_height);

    m_vertex[1].text.setX(m_rectOnSource.x() * 1.0f / m_resource->m_width);
    m_vertex[1].text.setY(m_rectOnSource.y() * 1.0f / m_resource->m_height);

    m_vertex[2].text.setX(m_rectOnSource.x() * 1.0f / m_resource->m_width);
    m_vertex[2].text.setY((m_rectOnSource.bottom() + 1.0f) / m_resource->m_height);

    m_vertex[3].text.setX((m_rectOnSource.right() + 1.0f) / m_resource->m_width);
    m_vertex[3].text.setY((m_rectOnSource.bottom() + 1.0f) / m_resource->m_height);
    if (!m_rectOnSourceInited)
    {
        m_rectOnSourceInited = true;
        QSizeF s(m_rectOnSource.size());
        s.scale(m_pixViewportSize, Qt::KeepAspectRatio);
        if (s.width() > m_rectOnSource.width() || s.height() > m_rectOnSource.height())
        {
            s = m_rectOnSource.size();
        }
        setRect( (m_pixViewportSize.width() - s.width()) * 0.5 / m_pixViewportSize.width(),
                 (m_pixViewportSize.height() - s.height()) * 0.5 / m_pixViewportSize.height(),
                 s.width() / m_pixViewportSize.width(), s.height() / m_pixViewportSize.height());
    }
    else
    {
        onSizeChanged(this);
    }
}

void BaseLayer::onLayerOpened(BaseLayer *layer)
{
    if (m_parent) m_parent->onLayerOpened(layer);
}

void BaseLayer::onLayerRemoved(BaseLayer *layer)
{
    m_mutexChilds.lock();
    if (layer == this)
    {
        while(m_childs.size())
        {
            delete m_childs.front();
        }
    }
    else
    {
        int i = m_childs.indexOf(layer);
        if (i >= 0)
        {
            m_childs.remove(i);
        }
    }
    m_mutexChilds.unlock();
    if (m_parent)
    {
        m_parent->onLayerRemoved(layer);
    }
}

QVector<BaseLayer *> &BaseLayer::lockChilds()
{
    m_mutexChilds.lock();
    return m_childs;
}

void BaseLayer::unlockChilds()
{
    m_mutexChilds.unlock();
}

int BaseLayer::mov_Left(qreal left, QRectF& box)
{
    left = (left - 0.5) * m_glViewportSize.width();
    if (qAbs(left - box.right()) < (m_glViewportSize.width() / m_pixViewportSize.width())) return -1;

    if (left > box.right())
    {
       box.moveLeft(box.right());
       box.setWidth(left - box.left());
       return 1;
    }
    box.setLeft(left);
    return 0;
}

int BaseLayer::mov_Right(qreal right, QRectF& box)
{
    right = (right - 0.5) * m_glViewportSize.width();
    if (qAbs(right - box.left()) < (m_glViewportSize.width() / m_pixViewportSize.width())) return -1;

    if (right < box.left())
    {
        box.setWidth(box.left() - right);
        box.moveLeft(right);
        return 1;
    }
    box.setRight(right);
    return 0;
}

int BaseLayer::mov_Top(qreal top, QRectF& box)
{
    top = (top - 0.5) * m_glViewportSize.height();
    if (qAbs(top - box.bottom()) < (m_glViewportSize.height() / m_pixViewportSize.height()))
    {
        return -1;
    }
    if (top > box.bottom())
    {
       box.moveTop(box.bottom());
       box.setHeight(top - box.top());
       return 2;
    }
    box.setTop(top);
    return 0;
}

int BaseLayer::mov_Bottom(qreal bottom, QRectF& box)
{
    bottom = (bottom - 0.5) * m_glViewportSize.height();
    if (qAbs(bottom - box.top()) < (m_glViewportSize.height() / m_pixViewportSize.height())) return -1;
    if (bottom < box.top())
    {
       box.setHeight(box.top() - bottom);
       box.moveTop(bottom);
       return 2;
    }
    box.setBottom(bottom);
    return 0;
}
