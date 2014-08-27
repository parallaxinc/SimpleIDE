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

#ifndef HINTDIALOG_H
#define HINTDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class HintDialog;
}

class HintDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit HintDialog(QString tag, QString hint, QWidget *parent = 0);
    ~HintDialog();
    int exec();

    static void hint(QString tag, QString hint, QWidget *parent = 0);

private slots:
    void on_okButton_clicked();
    
private:
    Ui::HintDialog *ui;
    QSettings settings;
    QString key;
};

#endif // HINTDIALOG_H
