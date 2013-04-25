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

};

#endif // DIRECTORY_H
