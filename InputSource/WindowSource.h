#ifndef WINDOWSOURCE_H
#define WINDOWSOURCE_H
#include "BaseSource.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>

#include <QThread>
#include <QSemaphore>

class WindowSource : public BaseSource
{
public:
    WindowSource(const QString& typeName, const QString &sourceName = QString());
    virtual ~WindowSource();
protected:
    Window m_wid = 0;
    Pixmap m_pix = 0;
    XImage* m_img = nullptr;
    XWindowAttributes m_attr;

    QSemaphore  m_semShot;

    void run();

    virtual bool onClose();
    virtual bool onPlay();
    virtual bool onPause();
    virtual bool onOpen();
};

#endif // WINDOWSOURCE_H
