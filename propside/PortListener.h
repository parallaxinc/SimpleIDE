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

#ifndef PORTLISTENER_H_
#define PORTLISTENER_H_

#include "console.h"
#include "qextserialport.h"

class PortListener : public QThread
{
Q_OBJECT
public:
    PortListener(QObject *parent, Console *term);
    void init(const QString &portName = 0, BaudRateType baud = BAUD115200, QString ipaddr = 0);
    void setDtr(bool enable);
    void setRts(bool enable);
    bool open();
    void close();
    bool isOpen();
    void setTerminalWindow(QPlainTextEdit *editor);
    void send(QByteArray &data);
    int  readData(char *buff, int length);
    void run();

    QString getPortName();
    BaudRateType getBaudRate();

private:
    bool            useSerial;
    Console         *terminal;
    QextSerialPort  *port;
    XBeeSerialPort  *xbport;
    QPlainTextEdit  *textEditor;

private slots:
    void onDsrChanged(bool status);
    void updateReady(QextSerialPort*);
    void updateReady(XBeeSerialPort*);

signals:
    void readyRead(int length);
    void updateEvent(QextSerialPort*);
    void updateEvent(XBeeSerialPort*);
};


#endif /*PORTLISTENER_H_*/
