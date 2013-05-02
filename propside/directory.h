#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore>

class Directory
{
public:
    Directory();

    static bool isInFilterList(QString file, QStringList list);
    static void recursiveCopyDir(QString srcdir, QString dstdir, QString notlist = "");
    static void recursiveRemoveDir(QString dir);
    static QString find(QString file, QString find);
    static QStringList findList(QString file, QString find);
    static QString recursiveFind(QString dir, QString find);
    static QString recursiveFindFile(QString dir, QString file);

};

#endif // DIRECTORY_H
