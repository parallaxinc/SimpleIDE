#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore>

class Directory
{
public:
    Directory();

    static bool isInFilterList(QString file, QStringList list);
    static bool isPossibleInfiniteFolder(QString spath, QString dpath);
    static void recursiveCopyDir(QString srcdir, QString dstdir, QString notlist = "");
    static void recursiveRemoveDirSpecial(QString dir, QString parent);
    static void recursiveRemoveDir(QString dir);
    static QString find(QString file, QString find);
    static QStringList findCSourceList(QString file, QString find);
    static QString recursiveFind(QString dir, QString find);
    static QString recursiveFindFile(QString dir, QString file);

private:
    static bool isCSourceCommented(QString find, QString line, int num, QStringList lines);

};

#endif // DIRECTORY_H
