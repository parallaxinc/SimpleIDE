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

#define TELNET_POLL 0

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
    serialPort = new QextSerialPort(QextSerialPort::Polling);
    connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));

    wifiPort = new XEsp8266port();

    // polling doesn't work very well :)
    //connect(this, SIGNAL(updateEvent(XEsp8266port*)), this, SLOT(updateReady(XEsp8266port*)));

    // use event driven code with Telnet based Wifi
    connect(wifiPort, SIGNAL(updateEvent(XEsp8266port*)), this, SLOT(updateReady(XEsp8266port*)));
}

void PortListener::init(const QString & portName, BaudRateType baud, QString ipaddr)
{
    serialPort->setPortName("");
    wifiPort->setPortName("");

    if (ipaddr.length() == 0) {
        useSerial = true;
        serialPort->setPortName(portName);
        serialPort->setBaudRate(baud);
        serialPort->setFlowControl(FLOW_OFF);
        serialPort->setParity(PAR_NONE);
        serialPort->setDataBits(DATA_8);
        serialPort->setStopBits(STOP_1);
        serialPort->setTimeout(10);
    }
    else {
        useSerial = false;
        if (portName.compare(wifiPort->getPortName()) && wifiPort->isOpen()) {
            wifiPort->close();
            msleep(500); // just in case the port has not been released yet.
        }
        //wifiPort->open(QHostAddress(ipaddr), baud);
        wifiPort->setPortName(portName);
        wifiPort->setIpAddress(ipaddr);
        wifiPort->setBaudRate(baud);
    }
}

QString PortListener::getPortName()
{
    if (useSerial) {
        return serialPort->portName();
    }
    else {
        return wifiPort->getPortName();
    }
}

BaudRateType PortListener::getBaudRate()
{
    return (useSerial) ? serialPort->baudRate() : (BaudRateType)wifiPort->getBaudRate();
}

void PortListener::setDtr(bool enable)
{
    if (useSerial) serialPort->setDtr(enable);
}

void PortListener::setRts(bool enable)
{
    if (useSerial) serialPort->setRts(enable);
}

bool PortListener::open()
{
    if(!textEditor) // no text editor, no open
        return false;

    if(terminal == NULL)
        return false;

    if (useSerial) {
        if(serialPort == NULL)
            return false;

        if(serialPort->isOpen() == true)
            return false;

        serialPort->open(QIODevice::ReadWrite);

        connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
        this->start();
    }
    else {
        connect(wifiPort, SIGNAL(updateEvent(XEsp8266port*)), this, SLOT(updateReady(XEsp8266port*)));
        wifiPort->open(QHostAddress(wifiPort->getIpAddress()), wifiPort->getBaudRate());
    }
    return true;
}

void PortListener::close()
{
    if (useSerial) {
        if(serialPort == NULL) return;
        disconnect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
        serialPort->close();
    }
    else {
        disconnect(wifiPort, SIGNAL(updateEvent(XEsp8266port*)), this, SLOT(updateReady(XEsp8266port*)));
        wifiPort->close();
    }
    msleep(500); // just in case the port has not been released yet.
}

bool PortListener::isOpen()
{
    return (useSerial) ? serialPort->isOpen() : wifiPort->isOpen();
}

void PortListener::setTerminalWindow(QPlainTextEdit *editor)
{
    textEditor = editor;
}

void PortListener::send(QByteArray &data)
{
    if (useSerial) {
        serialPort->write(data.constData(),1);
    }
    else {
        wifiPort->write(data.constData(),1);
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

void PortListener::updateReady(XEsp8266port* port)
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
        while(serialPort->isOpen()) {
            if(terminal->serialPollEnabled()) {
                msleep(POLL_DELAY);
                QApplication::processEvents();
                if(terminal->enabled()) {
                    emit updateEvent(serialPort);
                }
            }
        }
    }
}

