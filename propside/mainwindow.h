/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QIcon>
#include <QMainWindow>
#include <iostream>
#include <exception>
#include "stdio.h"
#include "highlighter.h"
#include "treemodel.h"
#include "PortListener.h"
#include "qextserialport.h"
#include "terminal.h"
#include "properties.h"
#include "asideconfig.h"
#include "asideboard.h"
#include "console.h"
#include "hardware.h"
#include "projectoptions.h"
#include "cbuildtree.h"
#include "finddialog.h"

#define untitledstr "Untitled"

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

public slots:
    void terminalEditorTextChanged();
    void newFile();
    void openFile(const QString &path = QString());
    void saveFile(const QString &path = QString());
    void saveFileByTabIndex(int tab);
    void saveAsFile(const QString &path = QString());
    void openRecentProject(const QString &path = QString());
    void printFile(const QString &path = QString());    // not implemented
    void zipFile(const QString &path = QString());      // not implemented
    void about();
    void projectTreeClicked(QModelIndex index);
    void referenceTreeClicked(QModelIndex index);
    void closeTab(int index = 0);
    void changeTab(int index);
    void setCurrentBoard(int index);
    void setCurrentPort(int index);
    void connectButton();
    void terminalClosed();
    void setProject();
    void hardware();
    void properties();
    void propertiesAccepted();
    void programBuild();
    void programBurnEE();
    void programRun();
    void programDebug();
    void compilerError(QProcess::ProcessError error);
    void compilerFinished(int exitCode, QProcess::ExitStatus status);
    void closeEvent(QCloseEvent *event);

    void fileChanged();
    void keyHandler(QKeyEvent* event);
    void sendPortMessage(QString s);
    void enumeratePorts();
    void initBoardTypes();

    void addProjectFile();
    void deleteProjectFile();
    void showProjectFile();
    void saveProjectOptions();

    void copyFromFile();
    void cutFromFile();
    void pasteToFile();
    void editCommand();
    void systemCommand();
    void findInFile();
    void findNextInFile();
    void findPrevInFile();
    void redoChange();
    void undoChange();

    void compileStatusClicked();

    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();

private:
    void exitSave();
    void getApplicationSettings();
    int  checkCompilerInfo();
    int  runBuild(void);
    int  runCOGC(QString filename);
    int  runBstc(QString spinfile);
    int  runCogObjCopy(QString datfile);
    int  runObjCopy(QString datfile);
    int  runGAS(QString datfile);
    QStringList getCompilerParameters(QStringList options);
    int  runCompiler(QStringList options);
    QStringList getLoaderParameters(QString options);
    int  runLoader(QString options);
    int  startProgram(QString program, QString workpath, QStringList args);
    int  checkBuildStart(QProcess *proc, QString progName);
    void showBuildStart(QString progName, QStringList args);
    int  buildResult(int exitStatus, int exitCode, QString progName, QString result);

    void openFileName(QString fileName);
    void checkAndSaveFiles();
    void setupEditor();
    void setupFileMenu();
    void setupHelpMenu();
    void setupToolBars();
    void setupProjectTools(QSplitter *vsplit);
    void addToolButton(QToolBar *bar, QToolButton *btn, QString imgfile);
    void updateProjectTree(QString fileName, QString text);
    void updateReferenceTree(QString fileName, QString text);
    void setEditorTab(int num, QString shortName, QString fileName, QString text);
    QString shortFileName(QString fileName);
    QString sourcePath(QString file);

    void setCurrentProject(const QString &fileName);
    void updateRecentProjectActions();

    QSettings       *settings;
    QString         aSideLoader;
    QString         aSideCompiler;
    QString         aSideCompilerPath;
    QString         aSideIncludes;
    QString         aSideCfgFile;
    QString         aSideSeparator;

    Highlighter     *highlighter;

    QToolBar        *fileToolBar;
    QToolBar        *propToolBar;
    QToolBar        *debugToolBar;
    QToolBar        *ctrlToolBar;

    QSplitter       *leftSplit;
    QSplitter       *rightSplit;

    QTabWidget      *editorTabs;
    QVector<QPlainTextEdit*> *editors;
    bool            fileChangeDisable;

    QPlainTextEdit  *compileStatus;

    QString         projectFile;
    CBuildTree      *projectModel;
    QTreeView       *projectTree;
    CBuildTree      *referenceModel;
#ifdef REFERENCE_TREE
    QTreeView       *referenceTree;
#endif
    QString         basicPath;
    QString         includePath;

    QComboBox       *cbBoard;
    QComboBox       *cbPort;
    QToolButton     *btnConnected;
    Console         *termEditor;
    PortListener     *portListener;
    Terminal        *term;
    int             termXpos;
    int             termYpos;

    Properties      *propDialog;

    ASideConfig     *aSideConfig;
    QString         portName;
    QString         boardName;

    QProcess        *process;
    QProgressBar    *progress;
    int             progMax;
    int             progCount;

    Hardware        *hardwareDialog;
    QLabel          *status;
    QLabel          *programSize;

    QMenu           *projectMenu;
    QModelIndex     projectIndex;
    QString         projectHome;

    ProjectOptions  *projectOptions;

    FindDialog      *findDialog;

    enum { MaxRecentProjects = 5 };
    QAction *recentProjectActs[MaxRecentProjects];
    QAction *separatorAct;
};

//! [0]

// Macro which connects a signal to a slot, and which causes application to
// abort if the connection fails.  This is intended to catch programming errors
// such as mis-typing a signal or slot name.  It is necessary to write our own
// macro to do this - the following idiom
//     Q_ASSERT(connect(source, signal, receiver, slot));
// will not work because Q_ASSERT compiles to a no-op in release builds.

#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if(!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);

#endif
