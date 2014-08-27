/*
 * This file is part of the Parallax Propeller SimpleIDE development environment.
 *
 * Copyright (C) 2014 Parallax Incorporated
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainspinwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if defined(IDEDEBUG)
    MainSpinWindow w;
#endif

#if defined(Q_OS_MAC)
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
        while(s.indexOf("\\") > -1)
            s = s.replace("\\","/");
#ifndef TODO
        w.openFileName(s);
#else
        // remove this after testing
        if(s.endsWith(".zip",Qt::CaseInsensitive)) {
            Zipper  zip;
            QString fileName;
            QString data;
            if(zip.unzipFileCount(s) > 0) {
                QString folder = s.mid(0,s.lastIndexOf(".zip"));
                QString projName = folder;
                projName = projName.mid(projName.lastIndexOf("/")+1);
                QString projFile = projName+".side";
                if(!zip.unzipFileExists(s,projFile)) {
                    projFile = projName+"/"+projName+".side";
                }
                if(zip.unzipFileExists(s,projFile)) {
                    QString pathName = QDir::tempPath()+"/SimpleIDE_"+projName;
                    while(pathName.indexOf("\\") > -1)
                        pathName = pathName.replace("\\","/");
                    QDir dst(pathName);
                    if(!dst.exists())
                        dst.mkdir(pathName);
                    zip.unzipAll(s,pathName);
                    w.openProject(pathName+"/"+projFile);
                }
                else {
                    data = zip.unzipFirstFile(s, &fileName);
                }
            }
            if(fileName.length() && data.length()) {
                w.openFileStringTab(fileName, data);
            }
#if 0
        if(s.contains(QDir::toNativeSeparators(QDir::tempPath())) &&
           s.endsWith(".zip",Qt::CaseInsensitive)) {
#if 1
            Zipper zip;
            QString fileName;
            QString data = zip.unzipFirstFile(s, &fileName);
            w.openFileStringTab(fileName, data);
#else
            QMessageBox::critical(&w, w.tr("Cannot Open from Zip"),
                w.tr("The file is in a zipped archive. Unzip to")+"\n"+
                w.tr("a folder first, and open from there instead."));
#endif
#endif
        }
        else {
            s = s.mid(s.lastIndexOf("."));
            if(s.contains(".side",Qt::CaseInsensitive))
                w.closeTab(0);
            w.openFile(QString(argv[1]));
        }
#endif
    }
    w.show();

    return a.exec();
}
