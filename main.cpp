#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

static bool is3A4000()
{
    static bool isgeted = false;
    static bool is3a4000 = false;
    if (!isgeted)
    {
        QFile file("/proc/cpuinfo");
        if (file.open(QFile::ReadOnly))
        {
            QByteArray buf = file.readAll();
            is3a4000 = (buf.indexOf("Loongson-3A4000")>0);
            isgeted = true;
            file.close();
        }
    }
    return is3a4000;
}


QString& initLibPaths(int i)
{
    static QString x264libPath;
    static QString faaclibPath;

    if (x264libPath.isEmpty())
    {
        x264libPath = QApplication::applicationDirPath();
        if (!x264libPath.endsWith("/")) x264libPath.append("/");
        faaclibPath = x264libPath;
    #ifdef Q_PROCESSOR_MIPS_64
        if (is3A4000())
        {
            x264libPath.append("lib/lisa64/3a4000/libx264.so.161");
            faaclibPath.append("lib/lisa64/3a4000/libfaac.so.0");
        }
        else
        {
            x264libPath.append("lib/lisa64/3a3000/libx264.so.161");
            faaclibPath.append("lib/lisa64/3a3000/libfaac.so.0");
        }
    #endif

    #ifdef Q_PROCESSOR_ARM
    #endif

    #ifdef Q_PROCESSOR_X86_64
        x264libPath.append("lib/amd64/libx264.so.161");
        faaclibPath.append("lib/amd64/libfaac.so.0");
    #endif
    }
    switch(i)
    {
    case 0:
        return x264libPath;
    case 1:
        return faaclibPath;
    }
    return x264libPath;
}

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
