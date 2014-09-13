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

#ifndef DEVICEID_H
#define DEVICEID_H

#include <QObject>
#include <QThread>

class DeviceID : public QThread
{
Q_OBJECT
public:
    DeviceID() {
        resetType = RESET_BY_DTR;
    }
    virtual ~DeviceID() {
    }

    virtual void run() {
    }

    virtual bool isDevice(QString port) {
        if(port.isEmpty())
            return false;
        return false;
    }

    enum { RESET_BY_DTR = 1 };
    enum { RESET_BY_RTS = 2 };

    int resetType;

    void setDtrReset() {
        resetType = RESET_BY_DTR;
    }
    void setRtsReset() {
        resetType = RESET_BY_RTS;
    }
};

#endif // DEVICEID_H
