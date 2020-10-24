#ifndef SCREENINPUT_H
#define SCREENINPUT_H

#include "BaseLayer.h"
#include "ScreenSource.h"

class ScreenLayer : public BaseLayer
{
public:
    ScreenLayer();
    ~ScreenLayer();

    enum Mode
    {
        unspecified,
        specScreen,
        fullScreen,
        rectOfScreen,
        specWindow,
        clientOfWindow,
    };
    struct Option
    {
        Mode    mode = unspecified;
        int32_t screenIndex = -1;
        Window  widTop = 0;
        Window  widReal = 0;

        QRect   geometry;       //
        QMargins margins;
    };

    const QString& layerType() const { static QString tn = "screen"; return tn; }

    static int32_t screenCount();
    static QRect screenRect(int32_t i);
    static QRect screenBound();
    static bool fullScreenImage(QImage& img);
    static QPoint mousePhysicalCoordinates();
    static QString windowName(Window wid);
    static Option posOnWindow(const QPoint& pos, Window exclude);
    bool open(const Option& opt);
    Option shotOption() const { return m_shotOption; }

private:
    friend ScreenSource;
    Option    m_shotOption;
    QRect     m_shotOnScreen;

    QOpenGLBuffer* m_vboCursor = nullptr;
    QRectF    m_cursorOnView;

    virtual BaseSource* onCreateSource(const QString &sourceName);
    virtual void onReleaseSource(BaseSource* source);
    virtual void draw();
};

#endif // SCREENINPUT_H
