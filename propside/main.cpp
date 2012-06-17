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

    QTranslator qtTranslator;
    QString progName = QString(ASideGuiKey)+"_";

    qDebug() << transpath+progName+QLocale::system().name()+".qm";

    /*
     * according to QTranslator::load, this will pick up:
     * foo_en_us.qm, foo_en.qm, foo_zh_SG.qm, foo_zh_TW.qm or foo_zh.qm
     */
    if(qtTranslator.load(progName + QLocale::system().name(), transpath)) {
        a.installTranslator(&qtTranslator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
