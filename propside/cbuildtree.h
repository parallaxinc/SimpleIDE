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

#ifndef CBUILDTREE_H
#define CBUILDTREE_H

#include "treeitem.h"
#include "treemodel.h"

class CBuildTree : public TreeModel
{
public:

    CBuildTree(const QString &shortFileName, QObject *parent);

    QStringList getRowList();

    void addRootItem(QString text);
    void aSideIncludes(QString &text);
    void aSideIncludes(QString &filePath, QString &incPath, QString &separator, QString &text, bool root = false);
    void addFileReferences(QString &filePath, QString &incPath, QString &separator, QString &text, bool root = false);

};

#endif // CBUILDTREE_H
