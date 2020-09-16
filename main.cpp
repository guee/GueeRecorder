#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{


    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);

    QApplication::setOrganizationName("Guee");
    QApplication::setOrganizationDomain("looongson.xyz");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setApplicationName("GueeRecorder");
   // QApplication::setApplicationDisplayName("GueeScreenRecorder");
    QApplication::setApplicationVersion("2020-09-16");


    QSharedMemory singleton(a.applicationName());
    if (!singleton.create(8))
    {
        if (singleton.attach())
        {

        }
        QMessageBox::information(nullptr, "Guee 录屏机", "程序已经在运行了，请不要重复启动。");

        return false;
    }


    MainWindow w;
    w.show();
    return a.exec();
}
