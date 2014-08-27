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

#ifndef ASIDECONFIG_H
#define ASIDECONFIG_H

#include "qtversion.h"

#include "asideboard.h"

#define GENERIC_BOARD "GENERIC"

class ASideConfig
{
public:
    static const QString SdRun;
    static const QString SdLoad;
    static const QString Serial;
    static const QString IDE;
    static const QString SubDelimiter;
    static const QString UserDelimiter;

    ASideConfig();
    ~ASideConfig();

    int         loadBoards(QString filePath);
    int         addBoards(QString filePath);
    ASideBoard *newBoard(QString name);
    void        deleteBoardByName(QString name);

    QStringList getBoardNames();
    ASideBoard *getBoardByName(QString name);
    ASideBoard *getBoardData(QString name);


private:
    QString             filePath;
    QStringList         boardNames;
    QList<ASideBoard*> *boards;
};

#endif // ASIDECONFIG_H
