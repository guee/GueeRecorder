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
    if (m_resource == nullptr || m_program == nullptr || m_resource->m_texture == nullptr || !m_resource->m_texture->isCreated()) return;

    m_program->bind();
    m_program->setUniformValue("qt_Texture0", 0);
    m_program->setUniformValue("yuvFormat", m_resource->m_intputYuvFormat);
    m_program->setUniformValue("textureSize", m_resource->m_texture->width(), m_resource->m_texture->height());

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
        auto &par = m_parent->m_childs;
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
    }
    auto &par = parent->m_childs;
    layer = std::min(layer, par.size());
    layer = std::max(layer, 0);
    par.insert(par.begin() + layer, this);
    m_parent = parent;
    return true;
}

int32_t BaseLayer::childLayerCount()
{
    return static_cast<int32_t>(m_childs.size());
}

BaseLayer *BaseLayer::childLayer(int32_t i)
{
    if (i >= 0 && i < static_cast<int32_t>(m_childs.size()))
    {
        return m_childs[static_cast<size_t>(i)];
    }
    return nullptr;
}

int32_t BaseLayer::layerIndex()
{
    if ( m_parent )
    {
        for ( int i = 0; i < m_parent->m_childs.size(); ++i)
        {
            if ( m_parent->m_childs[i] == this )
            {
                return static_cast<int32_t>(i);
            }
        }
    }
    return 0;
}

BaseLayer *BaseLayer::childLayer(const QPointF &pos, bool onlyChild)
{
    BaseLayer* inp = nullptr;
    for ( auto i:m_childs )
    {
        inp = i->childLayer(pos, false);
        if (inp) return inp;
    }
    if (!onlyChild)
    {
        QPointF p((pos.x() - 0.5) * m_glViewportSize.width(), (pos.y() - 0.5) * m_glViewportSize.height());
        if ( m_userdefOnView.contains(p) )
        {
            return this;
        }
    }
    return nullptr;
}

void BaseLayer::setRect(const QRectF& rect)
{
    m_fullViewport = false;
    m_userdefOnView.setX((rect.x() - 0.5) * m_glViewportSize.width());
    m_userdefOnView.setY((rect.y() - 0.5) * m_glViewportSize.height());
    m_userdefOnView.setWidth(rect.width() * m_glViewportSize.width());
    m_userdefOnView.setHeight(rect.height() * m_glViewportSize.height());
    onSizeChanged(this);
}

void BaseLayer::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_fullViewport = false;
    m_userdefOnView.setX((x - 0.5) * m_glViewportSize.width());
    m_userdefOnView.setY((y - 0.5) * m_glViewportSize.height());
    m_userdefOnView.setWidth(w * m_glViewportSize.width());
    m_userdefOnView.setHeight(h * m_glViewportSize.height());
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

QRectF BaseLayer::rect() const
{
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

void BaseLayer::setViewportSize(const QSizeF &s, bool childs)
{
    m_glViewportSize = s;

    if (m_fullViewport)
    {
        m_userdefOnView.setRect(s.width() * -0.5, s.height() * -0.5, s.width(), s.height());
    }

    if (m_program)
    {
        QMatrix4x4 m;
        m.ortho(static_cast<float>(s.width() * -0.5), static_cast<float>(s.width() * 0.5),
                static_cast<float>(s.height() * 0.5), static_cast<float>(s.height() * -0.5),
                -1, 1);
        m_program->bind();
        m_program->setUniformValue("qt_ModelViewProjectionMatrix",m);
    }
    if (childs)
    {
        for (auto it:m_childs)
        {
            it->setViewportSize(s, childs);
        }
    }
    onSizeChanged(this);
}

void BaseLayer::setShaderProgram(QOpenGLShaderProgram *prog)
{

    m_program = prog;
    if (m_program)
    {
        setViewportSize(m_glViewportSize);
    }

}

bool BaseLayer::updateSourceTextures(int64_t requestTimestamp)
{
    bool ret = false;
    for (auto it:m_resPool)
    {
        if (it->updateToTexture(requestTimestamp))
        {
            ret = true;
        }
    }
    return ret;
}

BaseSource *BaseLayer::findSource(const QString& typeName, const QString& sourceName)
{
    for (auto it:m_resPool)
    {
        if (it->m_typeName == typeName && it->m_sourceName == sourceName )
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
    m_rectOnSourceInited = true;
    m_rectOnSource = rect;
    m_vertex[0].text.setX((m_rectOnSource.right() + 1.0f) / m_resource->m_width);
    m_vertex[0].text.setY(m_rectOnSource.y() * 1.0f / m_resource->m_height);

    m_vertex[1].text.setX(m_rectOnSource.x() * 1.0f / m_resource->m_width);
    m_vertex[1].text.setY(m_rectOnSource.y() * 1.0f / m_resource->m_height);

    m_vertex[2].text.setX(m_rectOnSource.x() * 1.0f / m_resource->m_width);
    m_vertex[2].text.setY((m_rectOnSource.bottom() + 1.0f) / m_resource->m_height);

    m_vertex[3].text.setX((m_rectOnSource.right() + 1.0f) / m_resource->m_width);
    m_vertex[3].text.setY((m_rectOnSource.bottom() + 1.0f) / m_resource->m_height);

    onSizeChanged(this);
}

void BaseLayer::onLayerOpened(BaseLayer *layer)
{
    if (m_parent) m_parent->onLayerOpened(layer);
}

void BaseLayer::onLayerRemoved(BaseLayer *layer)
{
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
    if (m_parent)
    {
        m_parent->onLayerRemoved(layer);
    }
}
