#include <QtGui>
#include "mainspinwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if defined(IDEDEBUG)
    MainSpinWindow w;
#endif

#if defined(Q_WS_MAC)
    a.setWindowIcon(QIcon(":/images/SimpleIDE.icns"));
#else
    a.setWindowIcon(QIcon(":/images/SimpleIDE-all.ico"));
#endif
    a.setApplicationName(ASideGuiKey);

    qDebug() << a.applicationName() << "argument count " << argc;
    qDebug() << "Arguments: ";
    foreach(QString arg, a.arguments()) {
        qDebug() << arg;
    }

    QString dir = QApplication::applicationDirPath();

    // dir returned from above should not have a trailing /
    dir = dir.mid(0,dir.lastIndexOf("/"));
    QString transpath = dir+"/translations/";

    QTranslator qTranslator;
    qTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qTranslator);

    QTranslator qtTranslator;
    QString progName = QString(ASideGuiKey)+"_";

    qDebug() << "Locale: ";
    qDebug() << transpath+progName+QLocale::system().name()+".qm";

    /*
     * according to QTranslator::load, this will pick up:
     * foo_en_us.qm, foo_en.qm, foo_zh_SG.qm, foo_zh_TW.qm or foo_zh.qm
     */
    if(qtTranslator.load(progName + QLocale::system().name(), transpath)) {
        qDebug() << "Translating ...";
        a.installTranslator(&qtTranslator);
    }

    qDebug() << "Temp directory";
    qDebug() << QDir::toNativeSeparators(QDir::tempPath());

#if !defined(IDEDEBUG)
    MainSpinWindow w;
#endif

    if(argc > 1) {
        QString s = QString(argv[1]);
        if(s.contains(QDir::toNativeSeparators(QDir::tempPath())) &&
           s.contains(".zip",Qt::CaseInsensitive)) {
            QMessageBox::critical(&w, w.tr("Cannot Open from Zip"),
                w.tr("The file is in a zipped archive. Unzip to")+"\n"+
                w.tr("a folder first, and open from there instead."));
        }
        else {
            s = s.mid(s.lastIndexOf("."));
            if(s.contains(".side",Qt::CaseInsensitive))
                w.closeTab(0);
            w.openFile(QString(argv[1]));
        }
    }

    w.show();

    return a.exec();
}
