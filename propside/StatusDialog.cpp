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

#include "StatusDialog.h"

StatusDialog::StatusDialog(QWidget *parent) : QDialog(parent, 0)
{
    QVBoxLayout *vlayout = new QVBoxLayout();
    QHBoxLayout *hlayout = new QHBoxLayout();

    for(int j = 0; j < 5; j++) {
        QLabel *r = new QLabel();
        r->setPixmap(QPixmap(":/images//Target2.png"));
        //r->setBackgroundRole(QPalette::Base);
        r->setEnabled(false);
        bump.append(r);
        hlayout->addWidget(r);
    }

    messageLabel = new QLabel(this);
    QFont mfont = this->font();
    mfont.setPixelSize(14);
    messageLabel->setFont(mfont);
    vlayout->addWidget(messageLabel);
    vlayout->addLayout(hlayout);

    displayTimer = new QTimer(this);
    connect(displayTimer, SIGNAL(timeout()), this, SLOT(animate()));

    this->setWindowTitle("Status Message Dialog");
    this->setStyleSheet("QDialog { background-color: white; }");
    this->setLayout(vlayout);
}

StatusDialog::~StatusDialog()
{
}

void StatusDialog::init(const QString title, const QString message)
{
    QApplication::processEvents();
    this->setWindowTitle(title);

    QWidget *win = this->parentWidget();
    int w = 200;
    int h = 60;
    QRect   rect(win->x()+win->width()/2-w/2, win->y()+win->height()/2-h/2, w, h);

    this->setGeometry(rect);
    this->setMaximumWidth(w);

    messageLabel->setText(message);

    this->setWindowFlags(Qt::Tool);

    index = 0;
    displayTimer->start(100);

    this->show();
    QApplication::processEvents();
}

void StatusDialog::addMessage(const QString message)
{
    messageLabel->setText(messageLabel->text() + message);
}

void StatusDialog::setMessage(const QString message)
{
    messageLabel->setText(message);
}

QString StatusDialog::getMessage()
{
    return messageLabel->text();
}

void StatusDialog::stop(int count)
{
    displayTimer->stop();
    hide();
}

void StatusDialog::nextBump()
{
    for(int n = 0; n < bump.count(); n++) {
        bump[n]->setEnabled(false);
    }
    index = (index+1) % bump.count();
    bump[index]->setEnabled(true);
}

void StatusDialog::animate()
{
    nextBump();
    QApplication::processEvents();
}

bool StatusDialog::isRunning()
{
    return displayTimer->isActive();
}

#ifdef REMOVE_UNUSED_StatusDialogThread
StatusDialogThread::StatusDialogThread(QObject *parent) : QThread(parent)
{
    nextDelay = 300;
}

void StatusDialogThread::startit()
{
    running = -1;
    start();
}

void StatusDialogThread::stop(int count)
{
    running = count;
}

void StatusDialogThread::run()
{
    while(--running) {
        emit nextBump();
        QApplication::processEvents();
        msleep(nextDelay);
    }
    emit hideit();
}
#endif
