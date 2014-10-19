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
#if defined(IDEDEBUG) && !defined(QT5)
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

#if !defined(IDEDEBUG) || defined(QT5)
    MainSpinWindow w;
#endif

    if(argc > 1) {
        QString s = QString(argv[1]);
        while(s.indexOf("\\") > -1)
            s = s.replace("\\","/");
        if (s.endsWith(".side") && s.startsWith(QDir::tempPath()) &&
            s.left(s.lastIndexOf("/")).endsWith(".zip")) {
            qDebug() << s;
            qDebug() << "The .side file is in a temporary directory. It will not work.";
            QMessageBox::critical(&w, ("Can't Open File"), ("Can't open a .side file from within a .zip file.\nPlease open the .zip file from within SimpleIDE."));
        }
        else {
            qDebug() << "Opening" << s;
            w.openFileName(s);
        }
    }
    w.show();

    return a.exec();
}
