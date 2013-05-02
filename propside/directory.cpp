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
    if(dpath.exists() == false)
        return;

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

QStringList Directory::findList(QString fileName, QString find)
{
    QStringList list;
    QString data;
    QFile ofile(fileName);
    if(ofile.open(QFile::ReadOnly)) {
        QTextStream in(&ofile);
        data = in.readAll();
        ofile.close();
        QStringList lines = data.split("\n", QString::SkipEmptyParts);
        foreach(QString line, lines) {
            if(line.contains(find)) // && line.indexOf(find) == 0)
                list.append(line);
        }
    }
    return list;
}

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
