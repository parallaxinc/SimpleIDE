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

#ifndef ZIPPER_H
#define ZIPPER_H

#include "qtversion.h"

#include "StatusDialog.h"

#if 0
extern "C" {
    extern int minizip(const char *zipFileName, const char *fileListCSV);
    extern int miniunzip(const char *zipFile, const char *extractToFile);
}
#endif

class Zipper : public QObject
{
    Q_OBJECT
public:
    explicit Zipper(QObject *parent = 0);
    // special spinzip
    bool makeSpinZip(QString fileName, QStringList fileTree, QString libPath, StatusDialog *stat);
    // any zip
    bool zipit(QString fileName, QString folder);
    bool zipFileList(QString source, QStringList list, QString dstZipFile);

    bool unzipAll(QString fileName, QString folder, QString special = "");
    QString unzipFirstFile(QString zipName, QString *fileName);
    QString unzipTopTypeFile(QString zipName, QString type);
    QString unzipFile(QString zipName, QString fileName);
    bool unzipFileExists(QString zipName, QString fileName);
    int  unzipFileCount(QString zipName);

    QStringList directoryTreeList(QString folder);

private:
    QString getZipDestination(QString fileName);
    void    zipSpinProjectTree(QString fileName, QStringList fileTree);
    bool    createFolderZip(QString source, QString dstZipFile);
    QString filePathName(QString fileName);
    QString shortFileName(QString fileName);

    QString spinLibPath;
    StatusDialog *statusDialog;
    QString newProjectFolder;

signals:

public slots:

};

#endif // ZIPPER_H
