

#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include "VideoSynthesizer.h"
#include "FormLayerTools.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)
QT_FORWARD_DECLARE_CLASS(QOpenGLFramebufferObject)
//#include "CleanBackground.h"

class GlWidgetPreview : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GlWidgetPreview(QWidget *parent = nullptr);
    ~GlWidgetPreview() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void fixOffsetAsScreen();
    void setVideoObject(VideoSynthesizer* videoObj, FormLayerTools* toolWidget);

signals:
    void initGL();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent*event) override;
    void leaveEvent(QEvent*event) override;

private:
    void makeObject();
    QVector4D m_clearColor;
    qreal m_screenScale = 1.0;
    QSize m_viewportSize;
    GLuint m_sharedTextureId = 0;
    QOpenGLShaderProgram *m_program = nullptr;
    BaseLayer::VertexArritb m_vertex[4];
    QOpenGLBuffer m_vbo;
    VideoSynthesizer*   m_video = nullptr;
    QRect m_displayOfSceeen;
    QPoint m_offsetOfViewport;
    qreal m_edgeSize;
    BaseLayer* m_enterLayer = nullptr;
    QRect m_boxEnterLayer;    //鼠标移入的层在预览画面上的区域（像素单位）
    BaseLayer* m_editingLayer = nullptr;
    QRect m_boxOfEditing;  //当前编辑的层在预览画面上的区域（像素单位）
    QRect m_boxOfPressKey;
    QPoint m_posOfPressKey;
    QMatrix4x4 m_matrixView;
    bool m_boxEditing = false;
    Qt::WindowFrameSection m_hitType = Qt::NoSection;
    FormLayerTools* m_layerTools = nullptr;
    void hitTest(const QPoint& pos);
    void setHitCursor(Qt::WindowFrameSection hit);
    void notifySelectLayer(BaseLayer* layer);

    inline void swap(int32_t& a, int32_t& b)
    {
        int32_t c = a;
        a = b;
        b = c;
    }
public slots:
    void on_videoSynthesizer_frameReady(uint textureId);
    void on_layerRemoved(BaseLayer* layer);
    void on_selectLayer(BaseLayer* layer);
    void on_layerMoved(BaseLayer* layer);
signals:
    void selectLayer(BaseLayer* layer);
};

#endif
