#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap(":/images/SimpleIDE-splash.png");
    QSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::WindowStaysOnTopHint);
    splash.show();
    a.processEvents();

    MainWindow w;
    w.show();
    splash.finish(&w);
    return a.exec();
}
