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

#ifndef BUILDSPIN_H
#define BUILDSPIN_H

#include "build.h"

class BuildSpin : public Build
{
    Q_OBJECT
public:
    BuildSpin(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb, Properties *p);

    int  runBuild(QString option, QString projfile, QString compiler);
    int  makeDebugFiles(QString fileName, QString projfile, QString compiler);

    int  runBstc(QString spinfile);
    void appendLoaderParameters(QString copts, QString projfile, QStringList *args);

};

#endif // BUILDSPIN_H
