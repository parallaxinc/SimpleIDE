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

#ifndef BUILDC_H
#define BUILDC_H

#include "build.h"

class BuildC : public Build
{
    Q_OBJECT
public:
    BuildC(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb, Properties *p);

    int  runBuild(QString option, QString projfile, QString compiler);
    int  makeDebugFiles(QString fileName, QString projfile, QString compiler);
    QString getOutputPath(QString projfile);

    int  showCompilerVersion();

    int  runCOGC(QString filename, QString outext);
    int  runBstc(QString spinfile);
    int  runCogObjCopy(QString datfile, QString tarfile, QString outpath="");
    int  runObjCopyRedefineSym(QString oldsym, QString newsym, QString file);
    int  runObjCopy(QString datfile, QString outpath="");
    int  runGAS(QString datfile);
    int  runPexMake(QString fileName);
    int  runAR(QStringList copts, QString libname);
    int  runCompiler(QStringList copts);

    int  autoAddLib(QString projectPath, QString srcFile, QString libDir, QStringList incList, QStringList *newList);

    QStringList getCompilerParameters(QStringList copts);
    int  getCompilerParameters(QStringList copts, QStringList *args);
    void setMemModel(QString model);
    QString getMemModel();
    int ensureOutputDirectory();
    void appendLoaderParameters(QString copts, QString projfile, QStringList *args);

    bool isOutdated(QStringList srclist, QString srcpath, QString target);
    QStringList getLocalSourceList(QStringList &LLlist);
    QStringList getLibraryList(QStringList &ILlist, QString projectFile);
    QString findInclude(QString projdir, QString libdir, QString include);

private:
    QString findIncludePath(QString projdir, QString libdir, QString include);

private:
    QString projName;
    QString model;
    QString outputPath;
    QString exePath;
    QString exeName;
    QString memModel;
};

#endif // BUILDC_H
