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


#include "console.h"
#include "PortListener.h"
#include <QtDebug>

/*
 * We use Polling for the port because events are not
 * well behaved in the QextSerialPort library on windows.
 */
PortListener::PortListener(QObject *parent, Console *term) : QThread(parent)
{
    terminal = term;
    useSerial = false;

    /*
     * removed EVENT_DRIVEN code because it doesn't work on all platforms
     * and it would not be the same for serial and network ports.
     */
    port = new QextSerialPort(QextSerialPort::Polling);
    connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));

    xbport = new XBeeSerialPort();
    connect(this, SIGNAL(updateEvent(XBeeSerialPort*)), this, SLOT(updateReady(XBeeSerialPort*)));
}

void PortListener::init(const QString & portName, BaudRateType baud, QString ipaddr)
{
    port->setPortName("");
    xbport->setPortName("");

    if (ipaddr.length() == 0) {
        useSerial = true;
        port->setPortName(portName);
        port->setBaudRate(baud);
        port->setFlowControl(FLOW_OFF);
        port->setParity(PAR_NONE);
        port->setDataBits(DATA_8);
        port->setStopBits(STOP_1);
        port->setTimeout(10);
    }
    else {
        useSerial = false;
        if (portName.compare(xbport->portName()) && xbport->isOpen()) {
            xbport->close();
        }
        xbport->open(QHostAddress(ipaddr), baud);
        xbport->setPortName(portName);
    }
}

QString PortListener::getPortName()
{
    if (useSerial) {
        return port->portName();
    }
    else {
        return xbport->portName();
    }
}

BaudRateType PortListener::getBaudRate()
{
    return (useSerial) ? port->baudRate() : (BaudRateType)xbport->baudRate();
}

void PortListener::setDtr(bool enable)
{
    if (useSerial) port->setDtr(enable);
}

void PortListener::setRts(bool enable)
{
    if (useSerial) port->setRts(enable);
}

bool PortListener::open()
{
    if(!textEditor) // no text editor, no open
        return false;

    if(terminal == NULL)
        return false;

    if (useSerial) {
        if(port == NULL)
            return false;

        if(port->isOpen() == true)
            return false;

        port->open(QIODevice::ReadWrite);

        connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
        this->start();
    }
    else {
        connect(this, SIGNAL(updateEvent(XBeeSerialPort*)), this, SLOT(updateReady(XBeeSerialPort*)));
        this->start();
    }
    return true;
}

void PortListener::close()
{
    if (useSerial) {
        if(port == NULL) return;
        disconnect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
        port->close();
    }
    else {
        xbport->close();
    }
}

bool PortListener::isOpen()
{
    return (useSerial) ? port->isOpen() : xbport->isOpen();
}

void PortListener::setTerminalWindow(QPlainTextEdit *editor)
{
    textEditor = editor;
}

void PortListener::send(QByteArray &data)
{
    if (useSerial) {
        port->write(data.constData(),1);
    }
    else {
        xbport->write(data.constData(),1);
    }
}

void PortListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}

void PortListener::updateReady(QextSerialPort* port)
{
    if(terminal != NULL)
        if(terminal->enabled())
            terminal->updateReady(port);
}

void PortListener::updateReady(XBeeSerialPort* port)
{
    if(terminal != NULL)
        if(terminal->enabled())
            terminal->updateReady(port);
}

#if defined(Q_OS_WIN32)
// delay less than 25ms here is dangerous for windows
#define POLL_DELAY 25
#else
// delay for MAC/Linux
#define POLL_DELAY 10
#endif

/*
 * This is the port listener thread.
 * We have to use polling so that we can share the loader port.
 */
void PortListener::run()
{
    if (useSerial) {
        while(port->isOpen()) {
            msleep(POLL_DELAY);
            QApplication::processEvents();
            if(terminal->enabled())
                emit updateEvent(port);
        }
    }
    else {
        while(xbport->isOpen()) {
            msleep(POLL_DELAY);
            QApplication::processEvents();
            if(terminal->enabled())
                emit updateEvent(xbport);
        }
    }
}

