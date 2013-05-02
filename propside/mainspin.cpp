#include <QtGui>
#include "mainspinwindow.h"

//#undef IDEDEBUG

QPlainTextEdit *status;

void myMessageOutput(QtMsgType type, const char *msg)
{
    status->appendPlainText(msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainSpinWindow w;

#if defined(IDEDEBUG)
    status = w.getDebugEditor();
    qInstallMsgHandler(myMessageOutput);
#endif

    a.setWindowIcon(QIcon(":/images/SimpleIDE322.png"));

    a.setApplicationName(ASideGuiKey);
    qDebug() << a.applicationName() << "arg count " << argc;

    foreach(QString arg, a.arguments()) {
        qDebug() << "arg " << arg;
    }
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

    qDebug() << "temp directory" << QDir::toNativeSeparators(QDir::tempPath());

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
