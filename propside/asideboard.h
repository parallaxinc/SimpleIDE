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

#ifndef ASIDEBOARD_H
#define ASIDEBOARD_H

#include <QHash>

class ASideBoard
{
public:
    ASideBoard();
    ~ASideBoard();

    QString   get(QString property);
    void        set(QString property, QString value);
    QString   getBoardName();
    void        setBoardName(QString name);
    QString   getFormattedConfig();
    QStringList *getAll();
    int         parseConfig(QString file);

    static const QString clkmode;
    static const QString pllmode;
    static const QString clkfreq;
    static const QString baudrate;
    static const QString reset;
    static const QString rxpin;
    static const QString txpin;
    static const QString tvpin;
    static const QString textseg;
    static const QString dataseg;
    static const QString flashsize;
    static const QString ramsize;
    static const QString cachedriver;
    static const QString cachesize;
    static const QString cacheparam1;
    static const QString cacheparam2;

private:

    QString   boardName;
    QStringList *propNames;
    QStringList *clkModes;
    QStringList *baudRates;
    QStringList *dataRam;
    QStringList *textRam;
    QList<int>  *pinList;
    QString     resetType;

    QHash<QString, QString> propHash;
};

#endif // ASIDEBOARD_H
