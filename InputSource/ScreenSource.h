#ifndef SCREENSOURCE_H
#define SCREENSOURCE_H
#include "BaseSource.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <QThread>
#include <QSemaphore>

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
    static QImage::Format checkPixelFormat(XImage* image);
    static QVector<QRect>& screenRects() {if(nullptr == m_display) static_init(); return m_screenRects;}
    static QRect& screenBound() {if(nullptr == m_display) static_init(); return m_screenBound;}
    static Display* xDisplay() { if(nullptr == m_display) static_init(); return m_display;}
    static Window xRootWindow() {if(nullptr == m_display) static_init(); return m_rootWid;}

    static Atom atom_wm_state() {if(nullptr == m_display) static_init(); return m_atom_wm_state; }
    static Atom atom_net_wm_state() {if(nullptr == m_display) static_init(); return m_atom_net_wm_state; }
    static Atom atom_net_wm_state_hidden() {if(nullptr == m_display) static_init(); return m_atom_net_wm_state_hidden; }

    bool allocImage(uint width, uint height);
    void freeImage();
    bool captureWindow();
    void releaseWindow();

    bool shotScreen(const QRect* rect = nullptr);

    QRect calcShotRect();
    virtual bool updateToTexture(int64_t next_timestamp);
    
    static bool windowImage(Window, QImage& img);
protected:
    static Display *m_display;
    static int m_screen;
    static Window m_rootWid;
    static Visual *m_visual;
    static int m_depth;
    static bool m_useShm;
    static QVector<QRect> m_screenRects;
    //static QVector<QRect> m_screenDeadRects;
    static QRect m_screenBound;
    static bool m_recordCursor;
    static bool m_cursorUseable;
    static QSet<Window> m_changedWindows;
    static Atom m_atom_wm_state;
    static Atom m_atom_net_wm_state;
    static Atom m_atom_net_wm_state_hidden;

    XImage *m_img = nullptr;
    Window m_wid = 0;
    Pixmap m_pix = 0;
    GLXPixmap m_glxPix = 0;
    XWindowAttributes m_attr;
    XShmSegmentInfo m_shmInfo;
    bool m_shmServerAttached;
    QRect m_shotRect;
    QSemaphore  m_semShot;

    typedef void (APIENTRYP PFNGLXBINDTEXIMAGEEXTPROC)(Display*, GLXDrawable, int, const int*);
    static PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT;
    typedef void (APIENTRYP PFNGLXRELEASETEXIMAGEEXTPROC)(Display*, GLXDrawable, int);
    static PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT;

    void run();
    void drawCursor();

    virtual bool onClose();
    virtual bool onPlay();
    virtual bool onPause();
    virtual bool onOpen();
};

#endif // SCREENSOURCE_H
