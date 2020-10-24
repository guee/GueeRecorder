#ifndef SCREENSOURCE_H
#define SCREENSOURCE_H
#include "BaseSource.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <QSemaphore>

class ScreenLayer;
class ScreenSource : public BaseSource
{
public:
    ScreenSource(const QString& typeName, const QString &sourceName = QString());
    virtual ~ScreenSource();

    static bool static_init();
    static void static_uninit();

    static bool setRecordCursor(bool record);
    static bool isRecordCursor();
    static bool readScreenConfig();
    static QImage::Format checkPixelFormat(const XImage* img);
    static QVector<QRect>& screenRects() {static_init(); return m_screenRects;}
    static QRect& screenBound() {static_init(); return m_screenBound;}
    static Display* xDisplay() {static_init(); return m_display;}
    static Window xRootWindow() {static_init(); return m_rootWid;}
    static bool xCompcapIsValid();
    static bool ewmhIsSupported();
    static bool getWindowName(Window wid, QString& windowName);
    static bool getWindowClass(Window wid, QString& windowClass);
    static bool windowIsMinimized(Window wid);
    static Window findTopWindow(const QString& windowName, const QString& windowClass);

    struct TopWindowInfo
    {
        Window widTop;
        Window widReal;
    };

    static QVector<TopWindowInfo> getTopLevelWindows(bool queryTree = true);

    bool allocImage(uint width, uint height);
    void freeImage();
    bool captureWindow();
    void releaseWindow();

    bool shotScreen(const QRect* rect = nullptr);

    QRect calcShotRect();
    virtual void readyNextImage(int64_t next_timestamp);

    static bool windowImage(Window, QImage& img);
    static void updateCursorTexture();
    static void releaseCursorTexture();
protected:
    friend ScreenLayer;
    static Display *m_display;
    static int m_screen;
    static Window m_rootWid;
    static Visual *m_visual;
    static int m_depth;
    static QVector<QRect> m_screenRects;
    //static QVector<QRect> m_screenDeadRects;
    static QRect m_screenBound;
    static bool m_useShm;
    static bool m_xCompcapIsValid;
    static bool m_recordCursor;
    static bool m_cursorUseable;

    static XFixesCursorImage* m_cursorImage1;
    static XFixesCursorImage* m_cursorImage2;
    static QOpenGLTexture* m_cursorTexture1;
    static QOpenGLTexture* m_cursorTexture2;

    static Atom m_atom_wm_state;
    static Atom m_atom_net_wm_state;
    static Atom m_atom_net_wm_state_hidden;

    bool m_isXCompcapMode = false;
    XImage *m_img = nullptr;
    Window m_wid = 0;
    Pixmap m_pix = 0;
    GLXPixmap m_glxPix = 0;
    XWindowAttributes m_attr;
    QString m_windowName;
    QString m_windowClass;
    XShmSegmentInfo m_shmInfo;
    bool m_shmServerAttached;
    QRect m_shotRect;
    QSemaphore  m_semShot;
    QTime m_timeCheck;

    typedef void (APIENTRYP PFNGLXBINDTEXIMAGEEXTPROC)(Display*, GLXDrawable, int, const int*);
    static PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT;
    typedef void (APIENTRYP PFNGLXRELEASETEXIMAGEEXTPROC)(Display*, GLXDrawable, int);
    static PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT;

    void run();
    void drawCursor();
    static bool getWindowString(Window wid, QString &str, const char* atom);
    static Window findRealWindow(Window window);

    virtual bool onClose();
    virtual bool onPlay();
    virtual bool onPause();
    virtual bool onOpen();
    virtual bool isSameSource(const QString& type, const QString& source);
};

#endif // SCREENSOURCE_H
