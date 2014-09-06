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

#ifndef WORKSPACEDIALOG_H
#define WORKSPACEDIALOG_H

#include "qtversion.h"

#include <QDialog>
#include <QObject>
#include <QString>

class WorkspaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceDialog(QWidget *parent = 0);

    bool replaceDialog(QString folder, QString &selected);
    QString getWorkspace();

private:
    QLineEdit   leditFolder;
    QPushButton btnOk;
    QPushButton btnShowChange;
    QPushButton btnChange;

    QLabel      dialogLabel;
    QLabel      blankLabel;
    QLabel      changeLabel;

    QGridLayout grid;
    QVBoxLayout chgLayout;

    QString     workspace;

signals:

public slots:
    void accept();
    void changeFolder();
    void showChangeFolder();
};

#endif // WORKSPACEDIALOG_H
