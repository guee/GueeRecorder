#ifndef DIALOGSCREENAREA_H
#define DIALOGSCREENAREA_H

#include "InputSource/ScreenLayer.h"
#undef Bool
#include <QOpenGLWidget>
#include <QOpenGLFunctions>



class GlScreenSelect : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GlScreenSelect(QWidget *parent = nullptr);
    ~GlScreenSelect() override;
    void setMainScale(qreal scale);
    qreal mainScale() { return m_mainScale; }

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void enterEvent(QEvent*event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void timerEvent(QTimerEvent* event) override;
    ScreenLayer::Option& option() { return m_selOpt; }
    QRect& box() { return m_realBox; }
    void setToolsBox(const QRect& rect) { m_toolsBox = rect;}
    bool saveSelectToFile(const QString& filePath);
private:
    bool   m_boxEditing = false;
    bool   m_leftDown = false;
    bool   m_infoHide = false;
    int32_t m_repairTimerId = 0;
    int32_t m_edgeSize = 5;
    qreal m_mainScale = 0;
    QSize m_screenSize;
    int32_t m_fontSize = 0;
    QFont m_font;
    QImage m_imgScreen;

    QRect m_toolsBox;
    QRect m_zoomBox;
    QRect m_whiteBox;
    QRect m_InfoBox;
    QRect m_infoTexBox;
    ScreenLayer::Option m_selOpt;

    QOpenGLShaderProgram *m_programScreen = nullptr;
    QOpenGLBuffer m_vboScreen;
    QOpenGLTexture* m_mainTexture = nullptr;

    QVector4D m_lineColor;

    //QRect m_box;
    QPoint m_lastMovePos;
    QPoint m_pressKeyPos;
    QRect m_realBox;
    QRect m_oldBox;
    Qt::WindowFrameSection m_hitType = Qt::NoSection;


    QRect m_currMouseRect;
    void getMouseOnWindow();
    Qt::WindowFrameSection hitTest(const QPoint& pos);
    void setHitCursor(Qt::WindowFrameSection hit);
    void editBoxWithMouse();
    void initVBO();
    inline bool checkOffsetX(int32_t& v, int32_t x)
    {
        v += x - m_pressKeyPos.x();
        if ( v < 0 )
        {
            x -= v;
            v = 0;
            return false;
        }
        else if ( v >= m_screenSize.width() )
        {
            x += m_screenSize.width() - 1 - v;
            v = m_screenSize.width() - 1;
            return false;
        }
        return true;
    }
    inline bool checkOffsetY(int32_t& v, int32_t y)
    {
        v += y - m_pressKeyPos.y();
        if ( v < 0 )
        {
            y -= v;
            v = 0;
            return false;
        }
        if ( v >= m_screenSize.height() )
        {
            y += m_screenSize.height() - 1 - v;
            v = m_screenSize.height() - 1;
            return false;
        }
        return true;
    }
    inline void swap(int32_t& a, int32_t& b)
    {
        int32_t c = a;
        a = b;
        b = c;
    }
signals:
    void selected(bool cancel);
    void editing(bool ready, const QRect& box);
};

#endif // DIALOGSCREENAREA_H
