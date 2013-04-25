#include "directory.h"

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

void Directory::recursiveCopyDir(QString srcdir, QString dstdir, QString notlist)
{
    QString file;

    QStringList slist;
    QStringList flist;

    if(srcdir.length() < 1)
        return;
    if(dstdir.length() < 1)
        return;

    QStringList list;
    if(notlist.isEmpty() == false)
        list = notlist.split(" ", QString::SkipEmptyParts);

    if(srcdir.at(srcdir.length()-1) != QDir::separator())
        srcdir += QDir::separator();
    if(dstdir.at(dstdir.length()-1) != QDir::separator())
        dstdir += QDir::separator();

    QDir spath(srcdir);
    QDir dpath(dstdir);

    if(spath.exists() == false)
        return;
    if(dpath.exists() == false)
        dpath.mkdir(dstdir);

    flist = spath.entryList(QDir::AllEntries, QDir::DirsLast);
    foreach(file, flist) {
        if(file.compare(".") == 0)
            break;
        if(file.compare("..") == 0)
            break;
        if(isInFilterList(file,list) == false)
            QFile::copy(srcdir+file, dstdir+file);
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

void Directory::recursiveRemoveDir(QString dir)
{
    QDir dpath(dir);
    QString file;

    QStringList dlist;
    QStringList flist;

    if(dir.length() < 1)
        return;

    if(dir.at(dir.length()-1) != QDir::separator())
        dir += QDir::separator();

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
        recursiveRemoveDir(dir+file);
        dpath.rmdir(dir+file);
    }
    dpath.rmdir(dir);
}

