#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

void customMessageHandle(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    fprintf(stderr, "%s\n", msg.toUtf8().data());
}


int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);

    QApplication::setOrganizationName("Guee");
    QApplication::setOrganizationDomain("looongson.xyz");
    QApplication::setApplicationVersion("1.0.1");
    QApplication::setApplicationName("GueeRecorder");
    QApplication::setApplicationDisplayName("Guee 录屏机");

    bool alreadyExists = false;
    Window wid = 0;
    QSharedMemory singleton(a.applicationName());
    if (!singleton.create(sizeof(Window)))
    {
        if ( singleton.error() == 4 )
        {
            singleton.attach();
            memcpy(&wid, singleton.data(), sizeof(Window));
            if (wid)
            {
                Display* disp = XOpenDisplay(nullptr);
                if (disp)
                {
                    XWindowAttributes attributes;
                    if ( XGetWindowAttributes(disp, wid, &attributes) )
                    {
                        alreadyExists = true;
                        XSetInputFocus(disp, wid, RevertToPointerRoot, CurrentTime);
                        XRaiseWindow(disp, wid);
                    }
                    XCloseDisplay(disp);
                }
            }
        }
        else
        {
            alreadyExists = true;
        }
        if (alreadyExists)
        {
            singleton.detach();
            return 0;
        }
    }
    qInstallMessageHandler(customMessageHandle);
    MainWindow w;
    w.show();
    wid = w.winId();
    memcpy(singleton.data(), &wid, sizeof(Window));
    return a.exec();;
}
