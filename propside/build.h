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

#ifndef BUILD_H
#define BUILD_H

#include "blinker.h"
#include "properties.h"
#include "projectoptions.h"

#define FILELINK " -> "
#define SHOW_ASM_EXTENTION ".asm"
#define SHOW_ASMC_EXTENTION ".asmc"
#define SHOW_MAP_EXTENTION ".map"
#define BUILDALL_MEMTYPE "buildalltype"

class Build : public QWidget
{
    Q_OBJECT
public:
    Build(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb, Properties *p);

    virtual int  runBuild(QString option, QString projfile, QString compiler);
    virtual int  makeDebugFiles(QString fileName, QString projfile, QString compiler);
    virtual void appendLoaderParameters(QString copts, QString projfile, QStringList *args);
    virtual QString getOutputPath(QString projfile);

    enum DumpType { DumpNormal, DumpReadSizes, DumpCat, DumpOff };
    int  startProgram(QString program, QString workpath, QStringList args, DumpType dump = DumpOff);

public slots:
    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyReadCat();
    void procReadyRead();
    void procReadyReadSizes();

    void statusNone();
    void statusFailed();
    void statusPassed();

public:
    void abortProcess();
    int  checkBuildStart(QProcess *proc, QString progName);
    void showBuildStart(QString progName, QStringList args);
    int  buildResult(int exitStatus, int exitCode, QString progName, QString result);
    void compilerError(QProcess::ProcessError error);
    void compilerFinished(int exitCode, QProcess::ExitStatus status);

    int  checkCompilerInfo();
    QString sourcePath(QString srcpath);
    QString shortFileName(QString fileName);
    void removeArg(QStringList &list, QString arg);

    void clearIncludeHash() {
        if(incHash.count() > 0)
            incHash.clear();
    }

signals:
    void showCompileStatusError();

private:
    Blinker *blinker;

protected:
    QString         aSideCompiler;
    QString         aSideCompilerPath;
    QString         separator;

    QString         projectFile;
    QPlainTextEdit  *compileStatus;
    QLabel          *status;
    QLabel          *programSize;
    QProgressBar    *progress;
    QComboBox       *cbBoard;

    QProcess        *process;
    int             codeSize;
    int             memorySize;

    int             progMax;
    int             progCount;
    bool            procDone;
    bool            procResultError;
    QMutex          procMutex;

    ProjectOptions  *projectOptions;
    Properties      *properties;

    QString         outputFile;

    QHash<QString, QString> incHash;
    QHash<QString, QString> filesHash;
};

#endif // BUILD_H
