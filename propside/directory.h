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
