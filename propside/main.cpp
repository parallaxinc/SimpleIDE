#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/SimpleIDE6.png"));

    QString dir = QApplication::applicationDirPath();
    // dir returned from above should not have a trailing /
    dir = dir.mid(0,dir.lastIndexOf("/"));
    QString transpath = dir+"/translations/";

    // TODO: remove later. only for testing with Kenichi's translations at the moment
    QLocale locale(QLocale::Chinese, QLocale::Singapore);

    QTranslator qtTranslator;
    QString progName = QString(ASideGuiKey)+"_";

    qDebug() << transpath+progName+QLocale::system().name()+".qm";
    qDebug() << transpath+progName+locale.name()+".qm";

    if(qtTranslator.load(progName + QLocale::system().name(), transpath)) {
        a.installTranslator(&qtTranslator);
    }
    else if(qtTranslator.load(progName + locale.name(), transpath)) {
        a.installTranslator(&qtTranslator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
