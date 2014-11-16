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

#ifndef GDB_H
#define GDB_H

#include <QtCore>
#include "terminal.h"

class GDB : public QObject
{
    Q_OBJECT
public:
    explicit GDB(QPlainTextEdit *terminal, QObject *parent = 0);
    ~GDB();

    void load(QString gdbprog, QString path, QString target, QString image, QString port);
    void setRunning(bool running);
    void setReady(bool ready);
    void sendCommand(QString command);

    bool enabled();
    void stop();
    bool parseResponse(QString resp);

    QString getResponseFile();
    int     getResponseLine();

    void kill();
    void backtrace();
    void runProgram();
    void next();
    void step();
    void interrupt();
    void finish();
    void until();

signals:
    void breakEvent();

public slots:
    void procStarted();
    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();

private:
    QPlainTextEdit  *status;
    QProcess        *process;
    QMutex          mutex;
    bool            gdbRunning;
    bool            gdbReady;

    bool            programRunning;

    QString         fileName;
    int             lineNumber;
};

#endif // GDB_H
