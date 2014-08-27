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

#ifndef LOADER_H
#define LOADER_H

#include "qtversion.h"

class Loader : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Loader(QLabel *mainstatus, QPlainTextEdit *compileStatus, QProgressBar *progressBar, QWidget *parent);
    ~Loader();

    int  load(QString prog, QString path, QStringList args);
    int  reload(QString port);
    void setDisableIO(bool value);
    void setPortEnable(bool value);
    void setRunning(bool running);
    void sendCommand(QString command);
    bool enabled();
    void stop();

    void kill();

protected:
    void keyPressEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

public slots:
    void procStarted();
    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();

private:
    void setReady(bool value);

    QString program;
    QString workpath;

    QLabel          *status;
    QPlainTextEdit  *compiler;
    QProgressBar    *progress;
    QPlainTextEdit  *console;
    QProcess        *process;
    QMutex          mutex;
    bool            running;
    bool            ready;
    bool            disableIO;
};

#endif // LOADER_H
