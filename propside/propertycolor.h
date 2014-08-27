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

#ifndef PROPERTYCOLORS_H
#define PROPERTYCOLORS_H

#include "qtversion.h"

class PColor : public QColor
{
private:
    QString mName;
    QString mTrName;
    Qt::GlobalColor mValue;

public:
    PColor (QString trname, QString name, Qt::GlobalColor value) {
        mTrName = trname;
        mName = name;
        mValue = value;
    }
    QString getName() {
        return mName;
    }
    QString getTrName() {
        return mTrName;
    }
    Qt::GlobalColor getValue() {
        return mValue;
    }

    enum Colors {
        Black, DarkGray,
        Gray, LightGray,
        Blue, DarkBlue,
        Cyan, DarkCyan,
        Green, DarkGreen,
        Magenta, DarkMagenta,
        Red, DarkRed,
        Yellow, DarkYellow,
        White, LastColor };

};

#endif // PROPERTYCOLORS_H
