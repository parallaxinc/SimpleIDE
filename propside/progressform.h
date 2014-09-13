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

#ifndef PROGRESSFORM_H
#define PROGRESSFORM_H

#include <QWidget>

namespace Ui {
    class ProgressForm;
}

class ProgressForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressForm(QWidget *parent = 0);
    ~ProgressForm();

private:
    Ui::ProgressForm *ui;
};

#endif // PROGRESSFORM_H
