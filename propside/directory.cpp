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

#include "directory.h"
#include <QApplication>

Directory::Directory()
{
}

bool Directory::isInFilterList(QString file, QStringList list)
{
    if(list.isEmpty())
        return false;

    foreach(QString item, list) {
        QRegExp reg(item);
        reg.setPatternSyntax(QRegExp::Wildcard);
        if(reg.exactMatch(file))
            return true;
    }
    return false;
}

/*
 * check if copying source path to destination will cause an infinite folder.
 */
bool Directory::isPossibleInfiniteFolder(QString spath, QString dpath)
{
    if(dpath.endsWith("/")) dpath = dpath.left(dpath.lastIndexOf("/"));
    if(spath.endsWith("/")) spath = spath.left(spath.lastIndexOf("/"));
    if(dpath.endsWith("..")) dpath = dpath.left(dpath.lastIndexOf(".."));
    if(spath.endsWith("..")) spath = spath.left(spath.lastIndexOf(".."));
    return dpath.contains(spath);
}

void Directory::recursiveCopyDir(QString srcdir, QString dstdir, QString notlist)
{
    QString file;

    QStringList slist;
    QStringList flist;

    if(srcdir.length() < 1)
        return;
    if(dstdir.length() < 1)
        return;

    QApplication::processEvents();

    QStringList list;
    if(notlist.isEmpty() == false)
        list = notlist.split(" ", QString::SkipEmptyParts);

    if(srcdir.at(srcdir.length()-1) != '/')
        srcdir += '/';
    if(dstdir.at(dstdir.length()-1) != '/')
        dstdir += '/';

    QDir spath(srcdir);
    QDir dpath(dstdir);

    if(spath.exists() == false)
        return;
    if(isPossibleInfiniteFolder(srcdir, dstdir))
        return;
    if(dpath.exists() == false)
        dpath.mkdir(dstdir);

    flist = spath.entryList(QDir::AllEntries, QDir::DirsLast);
    foreach(file, flist) {
        if(file.compare(".") == 0)
            break;
        if(file.compare("..") == 0)
            break;
        QApplication::processEvents();
        if(isInFilterList(file,list) == false) {
            if(QFile::exists(dstdir+file)) {
                QFile::remove(dstdir+file);
            }
            QFile::copy(srcdir+file, dstdir+file);
        }
    }
    slist = spath.entryList(QDir::AllDirs, QDir::DirsLast);
    foreach(file, slist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        if(isInFilterList(file,list) == false)
            recursiveCopyDir(srcdir+file, dstdir+file, notlist);
    }
}

/*
 * This is just in the idea stage.
 */
void Directory::recursiveRemoveDirSpecial(QString dir, QString parent)
{
#ifdef DONTDEF
    QStringList dirs = dir.split("/", QString::SkipEmptyParts);

    QString smaller;
    QString tmpdir;

    for(int n = 0; n < dirs.count(); n++) {
        tmpdir = dirs[n];
        if(smaller.length() < 150) {
            smaller += tmpdir + "/";
        }
        else {
            break;
        }
    }
    exit(1);

    //QDir::rename(smaller, parent+tmpdir+"_tempdir");

    QDir dpath(dir);
    QString file;

    QStringList dlist;
    QStringList flist;

    if(dir.length() < 1)
        return;

    if(dir.at(dir.length()-1) != '/')
        dir += '/';

    flist = dpath.entryList(QDir::AllEntries, QDir::DirsLast);
    foreach(file, flist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        QFile::remove(dir+file);
    }
    dlist = dpath.entryList(QDir::AllDirs, QDir::DirsLast);
    foreach(file, dlist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        recursiveRemoveDirSpecial(dir+file, parent);
        dpath.rmdir(dir+file);
    }
    dpath.rmdir(dir);
#endif
}

void Directory::recursiveRemoveDir(QString dir)
{
    /*
     * Windows can't remove a folder with name > 256 without special treatment.
     * While this should never happen, it is possible that some previous versions
     * of this program created recursive folders. We need to remove such folders.
     *
     * Not implemented yet.
     *
    if(dir.length() > 10) {
        recursiveRemoveDirSpecial(dir, dir);
    }
    */

    QDir dpath(dir);
    QString file;

    QStringList dlist;
    QStringList flist;

    if(dir.length() < 1)
        return;

    QApplication::processEvents();

    if(dir.at(dir.length()-1) != '/')
        dir += '/';

    flist = dpath.entryList(QDir::AllEntries, QDir::DirsLast);
    foreach(file, flist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        QApplication::processEvents();
        QFile::remove(dir+file);
    }
    dlist = dpath.entryList(QDir::AllDirs, QDir::DirsLast);
    foreach(file, dlist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        recursiveRemoveDir(dir+file);
        dpath.rmdir(dir+file);
    }
    dpath.rmdir(dir);
}

QString Directory::find(QString file, QString find)
{
    QFile ofile(file);
    if(ofile.open(QFile::ReadOnly)) {
        QTextStream in(&file);
        QString data = in.readAll();
        ofile.close();
        if(data.contains(find))
            return file;
    }
    return QString("");
}

#if 1
bool Directory::isCSourceCommented(QString find, QString line, int num, QStringList lines)
{
    // return false if not commented

    // easy single line comment
    if(line.indexOf("//") > -1) {
        if(line.indexOf("//") < line.indexOf(find))
            return true;
    }

    // harder block comment
    QString lineStr;
    int comment = 0;
    QRegExp blk("/\\*.*\\*/");
    blk.setMinimal(true);

    for(int n = 0; n < num; n++) {
        lineStr = lines[n];
        QString s = lineStr;
        while(s.indexOf(blk) > -1) {
            s = s.replace(blk,"");
        }
        lineStr = s;
        if(lineStr.indexOf("/*") > -1) {
            lineStr = lineStr.mid(0,lineStr.indexOf("/*"));
            comment++;
        }
        if(lineStr.indexOf("*/") > -1) {
            lineStr = lineStr.mid(lineStr.indexOf("*/")+2);
            if(comment > 0)
                comment--;
        }
    }
    return comment > 0;
}

QStringList Directory::findCSourceList(QString fileName, QString find)
{
    QStringList list;
    QString data;
    QFile ofile(fileName);
    if(ofile.open(QFile::ReadOnly)) {
        QTextStream in(&ofile);
        data = in.readAll();
        ofile.close();
        QStringList lines = data.split("\n", QString::SkipEmptyParts);
        for(int num = 0; num < lines.count(); num++) {
            QString line = lines[num];
            if(line.contains(find)) {
                if(!isCSourceCommented(find, line, num, lines))
                    list.append(line);
            }
        }
    }
    return list;
}
#else
QStringList Directory::findCSourceList(QString fileName, QString find)
{
    QStringList list;
    QString data;
    QFile ofile(fileName);
    if(ofile.open(QFile::ReadOnly)) {
        QTextStream in(&ofile);
        data = in.readAll();
        ofile.close();
        QStringList lines = data.split("\n", QString::SkipEmptyParts);
        int comment = 0;
        QRegExp blk("/\\*.*\\*/");
        blk.setMinimal(true);
        foreach(QString line, lines) {
            //if(line.indexOf(blk) > -1)
            {
                QString s = line;
                while(s.indexOf(blk) > -1) {
                    s = s.replace(blk,"");
                }
                line = s;
            }
            if(line.indexOf("/*") > -1) {
                line = line.mid(0,line.indexOf("/*"));
                comment++;
                if(line.contains(find))
                    list.append(line);
            }
            if(line.indexOf("*/") > -1) {
                line = line.mid(line.indexOf("*/")+2);
                if(comment > 0)
                    comment--;
            }
            if(comment > 0)
                continue;
            if(line.indexOf("//") > -1 && line.indexOf("//") < line.indexOf(find))
                continue;
            if(line.contains(find))
                list.append(line);
        }
    }
    return list;
}
#endif

QString Directory::recursiveFind(QString dir, QString find)
{
    QDir dpath(dir);
    QString file;

    QStringList dlist;
    QStringList flist;

    if(dir.length() < 1)
        return file;

    if(dir.at(dir.length()-1) != QDir::separator())
        dir += QDir::separator();

    flist = dpath.entryList(QDir::AllEntries, QDir::DirsLast);
    foreach(file, flist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        // check for find string
        QString retfile = Directory::find(file, find);
        if(retfile.isEmpty() == false)
            return retfile;
    }
    dlist = dpath.entryList(QDir::AllDirs, QDir::DirsLast);
    foreach(file, dlist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        return recursiveFind(dir+file, find);
    }
    return QString("");
}

QString Directory::recursiveFindFile(QString dir, QString findfile)
{
    QDir dpath(dir);
    QString file;

    QStringList dlist;
    QStringList flist;

    if(dir.length() < 1)
        return file;

    QChar sep = dir.at(dir.length()-1);
    if(sep != '/' && sep != '\\')
        dir += "/";

    flist = dpath.entryList(QDir::AllEntries, QDir::DirsLast);
    foreach(file, flist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        // check for find string
        if(file.compare(findfile) == 0)
            return dir+file;
    }
    dlist = dpath.entryList(QDir::AllDirs, QDir::DirsLast);
    foreach(file, dlist) {
        if(file.compare(".") == 0)
            continue;
        if(file.compare("..") == 0)
            continue;
        QString val = recursiveFindFile(dir+file, findfile);
        if(val.isEmpty() == false)
            return val;
    }
    return QString("");
}
