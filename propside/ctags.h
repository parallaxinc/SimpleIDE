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

#ifndef CTAGS_H
#define CTAGS_H

#include "qtversion.h"

class CTags : public QObject
{
    Q_OBJECT
public:
    explicit CTags(QString path, QObject *parent = 0);

    int     runCtags(QString path);
    int     runSpinCtags(QString path, QString libpath);
    bool    enabled();
    QString findTag(QString symbol);
    QString getFile(QString line);
    int     getLine(QString line);

    int     tagPush(QString tagline);
    QString tagPop();
    void    tagClear();
    int     tagCount();

private:
    int     spintags(QStringList files);
    int     spintags(QString file);
    int     makeSpinTagMap(QString file, QMap<QString,QString> &map);

private slots:
    void    procError(QProcess::ProcessError);
    void    procReadyRead();
    void    procFinished(int,QProcess::ExitStatus);

private:
    bool        ctagsFound;
    QString     compilerPath;
    QString     ctagsProgram;
    QString     projectPath;
    QString     libraryPath;

    QProcess    *process;
    bool        procDone;
    QMutex      mutex;

    QString     tagFile;
    int         tagLine;
    QStringList tagStack;
};

#endif // CTAGS_H
