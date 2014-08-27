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

#include "buildstatus.h"

#ifdef SPINSIDE
#include "mainspinwindow.h"
#define MAINWINDOW MainSpinWindow
#else
#include "mainwindow.h"
#define MAINWINDOW MainWindow
#endif

BuildStatus::BuildStatus(QWidget *parent) : QPlainTextEdit(parent)
{
    Properties *p = static_cast<MAINWINDOW*>(parent)->propDialog;
    highlighter = new HighlightBuild(this->document(), p);
}

BuildStatus::~BuildStatus()
{
    delete highlighter;
}
