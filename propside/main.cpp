#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/SimpleIDE6.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
