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

#include <QApplication>
#include "PortConnectionMonitor.h"
#include "qextserialenumerator.h"

PortConnectionMonitor::PortConnectionMonitor(QObject *parent) :
    QThread(parent)
{
    running = true;
    start();
}

void PortConnectionMonitor::stop()
{
    running = false;
    this->msleep(600); // let run finish. don't terminate it.
}

QStringList PortConnectionMonitor::enumeratePorts()
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    QStringList stringlist;
    QStringList myPortList;
    QString name;
    stringlist << "List of ports:";
    for (int i = 0; i < ports.size(); i++) {
        stringlist << "port name:" << ports.at(i).portName;
        stringlist << "friendly name:" << ports.at(i).friendName;
        stringlist << "physical name:" << ports.at(i).physName;
        stringlist << "enumerator name:" << ports.at(i).enumName;
        stringlist << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
        stringlist << "product ID:" << QString::number(ports.at(i).productID, 16);
        stringlist << "===================================";
#if defined(Q_OS_WIN32)
        name = ports.at(i).portName;
        myPortList.append(name);
#elif defined(Q_OS_MAC)
        name = ports.at(i).portName;
        if(name.indexOf("usbserial") > -1)
            myPortList.append(name);
#else
        name = "/"+ports.at(i).physName;
        myPortList.append(name);
#endif
    }
    return myPortList;
}

void PortConnectionMonitor::run()
{
    QStringList ports;
    portList = enumeratePorts();
    while(running) {
        QApplication::processEvents();
        this->msleep(300);
        ports = enumeratePorts();
        if(ports != portList) {
            portList = ports;
            emit portChanged();
        }
    }
}
