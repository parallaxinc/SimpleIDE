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

#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include "qtversion.h"
#include <QTimer>

#ifdef REMOVE_UNUSED_StatusDialogThread
class StatusDialogThread : public QThread
{
    Q_OBJECT
public:
    explicit StatusDialogThread(QObject *parent = 0);
    virtual ~StatusDialogThread() {}
    void run();
    void startit();
    void stop(int count = 0);

signals:
    void nextBump();
    void hideit();

public:
    int running;
    int nextDelay;
};
#endif

class StatusDialog : public QDialog
{
    Q_OBJECT
public:
    StatusDialog(QWidget *parent = 0);
    virtual ~StatusDialog();

    void init(const QString title, const QString message);
    void addMessage(const QString message);
    void setMessage(const QString message);
    QString getMessage();

    void stop(int count = 0);

    bool isRunning();

public slots:
    void animate();
    void nextBump();

private:
    int index;
#ifdef REMOVE_UNUSED_StatusDialogThread
    StatusDialogThread thread;
#endif
    QLabel  *messageLabel;
    QList<QLabel*> bump;
    QTimer *displayTimer;
};

#endif // STATUSDIALOG_H
