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
        union
        {
            int32_t    screenIndex = -1;
            Window windowId;
        };

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
    static bool windowIsMinimized(Window wid);
    static void enum_window(Display*display, Window window, int depth);
    //static bool windowImage(Window wid);
    static Option posOnWindow(const QPoint& pos, Window exclude);
    //static QRect mapToLogicaRect(const QRect& rect);
    bool setShotOption(const Option& opt);
    Option shotOption() const { return m_shotOption; }
    static Window findRealWindow(Window window);
private:
    friend ScreenSource;
    Option    m_shotOption;
    QRect     m_shotOnScreen;
    virtual BaseSource* onCreateSource(const QString &sourceName);
    virtual void onReleaseSource(BaseSource* source);
};

#endif // SCREENINPUT_H
