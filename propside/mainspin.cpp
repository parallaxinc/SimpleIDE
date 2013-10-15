#include <QtGui>
#include "mainspinwindow.h"
#include "quazip.h"
#include "quazipfile.h"
#include "directory.h"

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

    QString progName = QString(ASideGuiKey)+"_";

    qDebug() << "Locale: ";
    qDebug() << transpath+progName+QLocale::system().name()+".qm";

    /*
     * according to QTranslator::load, this will pick up:
     * foo_en_us.qm, foo_en.qm, foo_zh_SG.qm, foo_zh_TW.qm or foo_zh.qm
     */
    QTranslator qtTranslator;
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
        if(s.contains(".zip",Qt::CaseInsensitive)) {
            int rc;
            QString sname = s;
            QRegExp rz(".zip", Qt::CaseInsensitive);
            int end = sname.lastIndexOf(rz);
            end += 4;
            QString zipname = s.mid(end+1);
            zipname = zipname.trimmed();
            sname = sname.mid(0,end);
            QuaZip zip(sname);
            zip.open(QuaZip::mdUnzip);
            rc = zip.getZipError();
            if(rc | (zipname.length() == 0) ) {
                QMessageBox::critical(&w, w.tr("Cannot Open from Zip"),
                    w.tr("The file is in a zipped archive. Unzip to")+"\n"+
                    w.tr("a folder first, and open from there instead."));
            }
            else {
                QStringList files = zip.getFileNameList();
                QString tmpfolder = QDir::tempPath()+"/SimpleIDE";
                if(files.count() > 0) {
                    QDir dir(tmpfolder);
                    if(QFile::exists(tmpfolder))
                        Directory::recursiveRemoveDir(tmpfolder);
                    dir.mkdir(tmpfolder);
                }
#if 0
                foreach (QString file, files) {
                    zip.getCurrentFileName(file);
                }
#endif
                QuaZipFile file(&zip);
                bool f =zip.goToFirstFile();
                for(; f; f=zip.goToNextFile()) {
                    QuaZipFileInfo info;
                    file.getFileInfo(&info);
                    if(info.name.length() == 0)
                        continue;
                    QString name = info.name;
                    QStringList folders = name.split("/");

                    QString path = tmpfolder;
                    for (int n = 0; n < folders.count()-1; n++) {
                        path += "/" + folders[n];
                        QDir dir(path);
                        if(QFile::exists(path) == false)
                            dir.mkdir(path);
                    }

                    QFile newFile(tmpfolder+"/"+info.name);
                    file.open(QIODevice::ReadOnly);
                    newFile.open(QIODevice::WriteOnly);
                    QTextStream outToFile(&newFile);
                    outToFile << file.readAll();;
                    newFile.close();
                    file.close();
                }
                zip.close();

                w.closeProject();
                QString filename = tmpfolder+"/"+zipname;
                if(filename.endsWith(".c") ||
                   filename.endsWith(".cpp") ||
                   filename.endsWith(".cogc") ||
                   filename.endsWith(".h") ||
                   filename.endsWith(".spin")) {
                    w.openFile(tmpfolder+"/"+zipname);
                }
                /*
                 * Disallow .side projects or other file names for now.
                 */
                else if (filename.endsWith(".side")) {
                    QMessageBox::critical(&w, w.tr("Cannot Open Projects from Zip"),
                        w.tr("The project is in a zipped archive. Unzip to")+"\n"+
                        w.tr("a folder first, and open from there instead."));
                }
                else {
                    //w.openFile(tmpfolder+"/"+zipname+"/"+zipname+".side");
                    QMessageBox::critical(&w, w.tr("Cannot Open from Zip"),
                        w.tr("The file is in a zipped archive. Unzip to")+"\n"+
                        w.tr("a folder first, and open from there instead."));
                }
            }
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
