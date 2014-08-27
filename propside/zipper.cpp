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

#include "zipper.h"

#include "qtversion.h"

#include <QtCore>
#include "zipreader.h"
#include "zipwriter.h"

Zipper::Zipper(QObject *parent) :
    QObject(parent)
{
}

bool Zipper::makeSpinZip(QString fileName, QStringList fileTree, QString libPath, StatusDialog *stat)
{
    statusDialog = stat;
    spinLibPath = libPath;
    zipSpinProjectTree(fileName, fileTree);
    return true;
}

bool Zipper::zipit(QString fileName, QString folder)
{
    bool retval = false;
    retval = createFolderZip(fileName, folder);
    return retval;
}

bool Zipper::zipFileList(QString source, QStringList list, QString dstZipFile)
{
    bool retval = true;
    ZipWriter zip(dstZipFile);
    if(!zip.isWritable())
        return false;

    foreach(QString entry, list) {
        if(entry.compare(".") == 0) continue;
        if(entry.compare("..") == 0) continue;
        if(entry.endsWith("/")) {
            zip.addDirectory(entry);
        } else {
            QFile file(source+"/"+entry);
            if(file.open(QFile::ReadOnly)) {
                zip.addFile(entry, file.readAll());
                file.close();
            }
            else {
                retval = false;
                break;
            }
        }
    }
    zip.close();
    return retval;
}

#include "directory.h"

bool Zipper::unzipAll(QString fileName, QString folder, QString special)
{
    bool rc = false;
    ZipReader zipr(fileName);
    QList<ZipReader::FileInfo> info = zipr.fileInfoList();
    bool onefolder = true;
    QString s = info.at(0).filePath;
    QString first;
    QString sep = "/";
    if(s.indexOf(sep) > 0) {
        first = s.left(s.indexOf(sep));
    }
    else if(s.indexOf("\\") > 0) {
        sep = "\\";
        first = s.left(s.indexOf(sep));
    }
    for(int n = 0; n < info.count(); n++) {
        QApplication::processEvents();
        s = info[n].filePath;
        if(first.compare(s.left(s.indexOf(sep)))) {
            onefolder = false;
            break;
        }
    }
    if(onefolder == false) {
        rc = zipr.extractAll(folder);
    }
    else {
        QString tmp = QDir::tempPath()+"/SimpleIDE_TempZip";
        QDir tmpd(QDir::tempPath());
        tmpd.mkdir(tmp);
        rc = zipr.extractAll(tmp);
        if(rc) {
            if(first.compare(special) == 0) {
                Directory::recursiveCopyDir(tmp+"/"+first, folder+"/"+special);
            }
            else {
                Directory::recursiveCopyDir(tmp+"/"+first, folder);
            }
            Directory::recursiveRemoveDir(tmp);
        }
    }
    return rc;
}

QString Zipper::unzipFirstFile(QString zipName, QString *fileName)
{
    QByteArray bytes;
    ZipReader zipr(zipName);
    QList<ZipReader::FileInfo> info = zipr.fileInfoList();
    if(info.count() > 0) {
        *fileName = info.at(0).filePath;
        bytes = zipr.fileData(*fileName);
    }
    return QString(bytes);
}

QString Zipper::unzipFile(QString zipName, QString fileName)
{
    QByteArray bytes;
    ZipReader zipr(zipName);
    QList<ZipReader::FileInfo> info = zipr.fileInfoList();
    for(int n = 0; n < info.count(); n++) {
        QApplication::processEvents();
        if(info.at(n).filePath.compare(fileName) == 0) {
            bytes = zipr.fileData(fileName);
            break;
        }
    }
    return QString(bytes);
}

bool Zipper::unzipFileExists(QString zipName, QString fileName)
{
    QString file;
    ZipReader zipr(zipName);
    QList<ZipReader::FileInfo> info = zipr.fileInfoList();
    for(int n = 0; n < info.count(); n++) {
        QApplication::processEvents();
        file = info.at(n).filePath;
        qDebug() << file;
        if(!file.compare(fileName)) {
            return true;
        }
    }
    return false;
}

int  Zipper::unzipFileCount(QString zipName)
{
    ZipReader zipr(zipName);
    QList<ZipReader::FileInfo> info = zipr.fileInfoList();
    return info.count();
}

QString Zipper::getZipDestination(QString fileName)
{
    /*
     * ask the user for destination ... start within existing location
     */
    QString dstName     = fileName;
    dstName             = dstName.mid(0,dstName.lastIndexOf("."));

    /* Enumerate user input until it's unique */
    int num = 0;
    QString serial("");
    if(QFile::exists(dstName+serial+".zip")) {
        do {
            num++;
            serial = QString("(%1)").arg(num);
        } while(QFile::exists(dstName+serial+".zip"));
    }

    QFileDialog dialog;
    QString afilter("Zip File (*.zip)");

    QString dstPath = dialog.getSaveFileName(0, tr("Zip"), dstName+serial+".zip", afilter);
    return dstPath;
}

void Zipper::zipSpinProjectTree(QString fileName, QStringList fileTree)
{
    /*
     * method assumes that fileName is the top of the Spin tree
     * - source files come from local source & library directory
     */
    if(fileName.isEmpty())              /* no empty .zip files, please! */
    {
        QMessageBox::critical(
                    0,tr("No file"),
                    tr("Can't \"Zip\" from an empty file.")+"\n"+
                    tr("Please create a new file or open an existing one."),
                    QMessageBox::Ok);
        return;
    }

    QString spinCodePath    = filePathName(fileName);
    QDir spinPath(spinCodePath);

    if(spinPath.exists() == false) {
        QMessageBox::critical(
                0,tr("Source Folder not Found."),
                tr("Can't \"Zip\" from a non-existing folder."),
                QMessageBox::Ok);
        return;
    }

    /*
     * create a new archive (zip) file
     */
    QString dstName     = getZipDestination(fileName);
    if (dstName.length() < 1) {
        return;
    }
    QString sfile = shortFileName(dstName);
    qDebug() << "dstName: " << sfile;
    statusDialog->init("Zipping Spin", sfile);

    QString source = sfile.left(sfile.indexOf(".",Qt::CaseInsensitive));

    ZipWriter zip(dstName);
    if(!zip.isWritable()) {
        QMessageBox::critical(0, tr("Zip File Error"),
            tr("Can't create zip file")+"\n"+dstName);
        return;
    }
    foreach(QString entry, fileTree) {
        QString name;
        if(QFile::exists(spinCodePath+"/"+entry)) {
            name = spinCodePath+"/"+entry;
        }
        else if(QFile::exists(spinLibPath+"/"+entry)) {
            name = spinLibPath+"/"+entry;
        }
        else {
            QMessageBox::critical(0, tr("Zip File Error"),
                tr("Can't add to zip file")+"\n"+entry);
            zip.close();
            return;
        }
        QFile file(name);
        if(file.open(QFile::ReadOnly)) {
            zip.addFile(source+"/"+entry, file.readAll());
            file.close();
        }
    }
    zip.close();

    //QMessageBox::information(0, tr("Zipped"), tr("Created zip file")+"\n"+dstName);

#if 0

    QuaZip zip(dstName);                // dgately 03/06/2014 - SHOULD THIS BE dstPath???
    if(!zip.open(QuaZip::mdCreate)) {
        QMessageBox::critical(this,
              tr("Can't Save Zip"),
              tr("Can't open .zip file to write: ")+dstName);
        return;
    }

    /* create the .zip in the same directory as the top spin file */
    //QString spinLibPath     = propDialog->getSpinLibraryString();
    //QStringList fileTree    = spinParser.spinFileTree(fileName, spinLibPath);
    QuaZipFile  outFile(&zip, this); /* allows iterative access to the zip */

    /*
     * iterate thru spin source tree, match-up a file to its full path, open it for reading, write to zip file...
     */

    for(int n = 0; n < fileTree.count(); n ++) {

        QString spinFile        = fileTree[n];
        spinFile                = spinFile.trimmed();       /* remove leading spaces */

        /*
         * ...for each file in the spin source tree, match it to its path & open it
         */
        QString     fullSpinPath;
        QFile       file;
        QFile       inFile;

        if(file.exists(spinCodePath+spinFile)) {
            fullSpinPath = spinCodePath+spinFile;                    /* if file exists in the local source directory */
            inFile.setFileName(fullSpinPath);
        }
        else if(file.exists(spinLibPath+spinFile)) {
            fullSpinPath = spinLibPath+spinFile;                     /* else, if file exists in the library directory */
            inFile.setFileName(fullSpinPath);
        }

        if (!inFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this,
                       tr("Zip Error"),
                       QString("testCreate(): inFile.open(): %1").arg(inFile.errorString().toLocal8Bit().constData()));
            return;
        }

        statusDialog->setMessage("Zipping: "+spinFile);

        /*
         * write the zip file from source file, appending one byte at a time
         */
        if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(spinFile))) {
            QMessageBox::critical(this,
                       tr("Zip Error"),
                       QString("testCreate(): outFile.open(): %1").arg(outFile.getZipError()));
            return;
        }

        char    c;
        while (inFile.getChar(&c) && outFile.putChar(c))
            ; // yes, empty body

        QApplication::processEvents();          /* give the app some time to process events */

        if (outFile.getZipError() != UNZ_OK) {
            QMessageBox::critical(this,
                       tr("Zip Error"),
                       QString("testCreate(): outFile.putChar(): %1").arg(outFile.getZipError()));
            return;
        }
        outFile.close();                        /* will re-open for more input files */

        if (outFile.getZipError() != UNZ_OK) {
            QMessageBox::critical(this,
                       tr("Zip Error"),
                       QString("testCreate(): outFile.close(): %1").arg(outFile.getZipError()));
            return;
        }
        inFile.close();                         /* done reading an input file */
    }
    zip.close();                                /* done looping through the source file tree */

    if (zip.getZipError() != 0) {
        QMessageBox::critical(this,
                   tr("Zip Error"),
                   QString("testCreate(): zip.close(): %1").arg(zip.getZipError()));
        return;
    }
#endif
    statusDialog->stop(4);
}

QStringList Zipper::directoryTreeList(QString folder)
{
    QStringList list;
    QDir dir(folder);
    foreach(QString entry, dir.entryList(QDir::AllEntries, QDir::DirsFirst)) {
        QApplication::processEvents();
        if(entry.compare(".") == 0) continue;
        if(entry.compare("..") == 0) continue;
        QString name = dir.path()+"/"+entry;
        if(QFile::exists(name) && !QFileInfo(name).isFile()) {
            if(entry.endsWith("/") == false)
                entry.append("/");
            list.append(entry);
            QStringList dlist = directoryTreeList(name);
            foreach(QString s, dlist)
                list.append(entry+s);
        }
        else {
            list.append(entry);
        }
    }
    return list;
}

bool Zipper::createFolderZip(QString source, QString dstZipFile)
{
    ZipWriter zip(dstZipFile);
    if(!zip.isWritable())
        return false;

    QStringList list = directoryTreeList(source);
    foreach(QString entry, list) {
        QApplication::processEvents();
        if(entry.compare(".") == 0) continue;
        if(entry.compare("..") == 0) continue;
        if(entry.endsWith("/")) {
            zip.addDirectory(entry);
        } else {
            QFile file(source+"/"+entry);
            if(file.open(QFile::ReadOnly)) {
                zip.addFile(entry, file.readAll());
                file.close();
            }
        }
    }
    zip.close();
    return true;
}

QString Zipper::filePathName(QString fileName)
{
    QString rets;
    if(fileName.lastIndexOf("/") > -1)
        rets = fileName.mid(0,fileName.lastIndexOf("/")+1);
    else
        rets = fileName;
    return rets;
}

QString Zipper::shortFileName(QString fileName)
{
    QString rets;
    if(fileName.indexOf("/") > -1)
        rets = fileName.mid(fileName.lastIndexOf("/")+1);
    else
        rets = fileName;
    return rets;
}

