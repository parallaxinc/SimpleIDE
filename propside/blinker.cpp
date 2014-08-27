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

#include "blinker.h"

Blinker::Blinker(QLabel *status)
{
    this->status = status;
}

/*
 * use for blinking status color
 */
void Blinker::run()
{
    int count = 71;
    while(count-- > 0) {
        msleep(100);
        QApplication::processEvents();
        switch(count) {
            case 60:
            case 40:
            case 20:
                emit statusNone();
                break;
            case 70:
            case 50:
            case 30:
            case 10:
                emit statusFailed();
                break;
        }
    }
}


