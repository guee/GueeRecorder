#ifndef SCREENSOURCE_H
#define SCREENSOURCE_H
#include "BaseSource.h"

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xinerama.h>

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
    static QVector<QRect>& screenRects() {return m_screenRects;}
    static QRect& screenBound() {return m_screenBound;}
    static Display* xDisplay() { return m_x11_Display;}
    static Window xRootWindow() {return m_x11_RootWindow;}

    bool allocImage(uint width, uint height);
    void freeImage();
	void shotThread();
    bool shotScreen(const QRect* rect = nullptr);
    QRect calcShotRect();
protected:
    static Display *m_x11_Display;
    static int m_x11_Screen;
    static Window m_x11_RootWindow;
    static Visual *m_x11_Visual;
    static int m_x11_Depth;
    static bool m_x11_UseShm;
    static QVector<QRect> m_screenRects;
    //static QVector<QRect> m_screenDeadRects;
    static QRect m_screenBound;
    static bool m_recordCursor;
    static bool m_cursorUseable;

    XImage *m_x11_image;
    XShmSegmentInfo m_x11_shm_info;
    bool m_x11_shm_server_attached;
    QRect m_shotRect;
    std::thread m_thread;
    FrameSynchronization    m_screenSync;

    void drawCursor();

    virtual bool onClose();
    virtual bool onPlay();
    virtual bool onPause();
    virtual bool onOpen();
};

#endif // SCREENSOURCE_H
