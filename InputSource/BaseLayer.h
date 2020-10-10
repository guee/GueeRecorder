#ifndef BASELAYER_H
#define BASELAYER_H

#include "./Common/FrameSynchronization.h"
#include "./Common/FrameRateCalc.h"
#include <QtCore>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMutex>

class BaseSource;
class BaseLayer
{
public:
    enum EStatus
    {
        NoOpen,         //没有打开
        Opening,        //正在打开
        Opened,         //已经打开成功，随时可以播放
        Palying,        //正在播放
        Paused,         //暂停中
    };
    struct VertexArritb
    {
        QVector3D vert;
        QVector2D text;
    };
    BaseLayer();
    virtual ~BaseLayer();
    virtual const QString& layerType() const = 0;
    virtual const QString& sourceName();

    //设置为指定的源，并打开它。
    //如果再次调用且 source 与上次不同，则会先自动关闭再重新打开。
    bool open(const QString &sourceName = QString());
    void close();

    bool play();
    bool pause();

    static void destroy(BaseLayer *layer)
    {
        if ( layer )
        {
            delete layer;
        }
    }

    EStatus status() const {return m_status;}

    virtual void draw();

    int32_t widthOfPixel() const { return m_rectOnSource.width(); }
    int32_t heightOfPixel() const { return m_rectOnSource.height(); }
    int32_t stride() const;
    const void *lockImage();
    void unlockImage();


    BaseLayer* parent() {return m_parent;}
    bool moveToLayer(int32_t layer);
    //0 层在上面
    bool setParent(BaseLayer* parent, int32_t layer = 0);
    int32_t childLayerCount();
    BaseLayer* childLayer(int32_t i);
    int32_t layerIndex();
    //以在整个画面上的归一化坐标取得对应的层
    //onlyChild ： 只检查本层的子层，不包含本层。
    //realBox : 只检查层的实际显示区域。
    BaseLayer* childLayer(const QPointF &pos, bool onlyChild = true, bool realBox = true);
    //以归一化的坐标设置图像在画面上的区域(X坐标从左到右为0到1，Y坐标从上到下为0到1)。
    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal w, qreal h );
    //返回 1 表示需要交换 Hit 左右方向，2 表示交换上下方向，3 表示两者。
    void movCenter(qreal x, qreal y);
    int movLeft(qreal left, bool realBox = true);
    int movRight(qreal right, bool realBox = true);
    int movTop(qreal top, bool realBox = true);
    int movBottom(qreal bottom, bool realBox = true);
    int movTopLeft(qreal top, qreal left, bool realBox = true);
    int movTopRight(qreal top, qreal right, bool realBox = true);
    int movBottomLeft(qreal bottom, qreal left, bool realBox = true);
    int movBottomRight(qreal bottom, qreal right, bool realBox = true);
    void setWidth(qreal width, bool realBox = true);
    void setHeight(qreal height, bool realBox = true);

    void setRectOnSource(const QRect& rect);
    void fullViewport(bool full);
    bool isFullViewport() const{ return m_fullViewport;}

    float imgContrast();
    void setImgContrast(float c);
    float imgBright();
    void setImgBright(float b);
    float imgSaturation();
    void setImgSaturation(float s);
    float imgHue();
    void setImgHue(float h);
    bool imgHueDye();
    void setImgHueDye(bool d);
    float imgTransparence();
    void setImgTransparence(float t);

    //取得图像在画面上的归一化坐标区域(X坐标从左到右为0到1，Y坐标从上到下为0到1)。
    QRectF rect(bool realBox = true) const;
    //设置和取得图像保持比例的模式
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    Qt::AspectRatioMode aspectRatioMode() const;

    void setViewportSize( const QSizeF& glSize, const QSize& pixSize, bool childs = false );

    void setShaderProgram(QOpenGLShaderProgram* prog);

protected:


    friend BaseSource;
    EStatus m_status = NoOpen;
    BaseLayer* m_parent = nullptr;


    QString m_friendlyName;
    QRect m_rectOnSource;
    QRectF m_userdefOnView;  //用户定义的在OpenGL绘图区域上的坐标和大小
    QRectF m_realBoxOnView;
    Qt::AspectRatioMode m_aspectRatioMode = Qt::KeepAspectRatio;  //保持比例的模式
    QSizeF m_glViewportSize;   //OpenGL绘图区域的宽高
    QSize m_pixViewportSize;
    QOpenGLBuffer* m_vbo = nullptr;
    QOpenGLShaderProgram* m_program = nullptr;
    bool m_fullViewport = true;        //是否缩放到整个画面。
    VertexArritb    m_vertex[4];

    static bool updateSourceTextures(int64_t requestTimestamp);
    static BaseSource* findSource(const QString& typeName, const QString& sourceName);
    static void setSourcesFramerate(float fps);
    virtual BaseSource* onCreateSource(const QString &sourceName) = 0;
    virtual void onReleaseSource(BaseSource* source);
    virtual float frameRate() const { return m_parent ? m_parent->frameRate() : -1.0f; }
    virtual void onSizeChanged(BaseLayer* layer);
    virtual void onLayerOpened(BaseLayer* layer);
    virtual void onLayerRemoved(BaseLayer* layer);

    QVector<BaseLayer*>& lockChilds();
    void unlockChilds();
private:
    static QVector<BaseSource*>    m_resPool;
    static QMutex m_lockSources;
    static QVector<QOpenGLShaderProgram*>   m_progPool;
    QVector<BaseLayer*> m_childs;
    QMutex m_mutexChilds;

    int mov_Left(qreal left, QRectF& box);
    int mov_Right(qreal right, QRectF& box);
    int mov_Top(qreal top, QRectF& box);
    int mov_Bottom(qreal bottom, QRectF& box);

    bool            m_rectOnSourceInited = false;
    bool            m_vertexChanged = false;
    BaseSource* m_resource = nullptr;

    float m_hslContrast = 0.0f;
    float m_hslBright = 0.0f;
    float m_hslSaturation = 0.0f;
    float m_hslHue = 0.0f;
    float m_transparence = 0.0f;
    bool m_hslHueDye = false;
};

#endif // BASELAYER_H
