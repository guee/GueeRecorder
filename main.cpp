#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);

    QApplication::setOrganizationName("Guee");
    QApplication::setOrganizationDomain("looongson.xyz");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setApplicationName("GueeRecorder");
    QApplication::setApplicationDisplayName("GueeScreenRecorder");



    MainWindow w;
    w.show();
    return a.exec();
}
