/*
 * mainwindow.h defines main window class
 */

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
#include "replacedialog.h"
#include "aboutdialog.h"
#include "ctags.h"
#include "newproject.h"
#include "editor.h"

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
    void printFile(const QString &path = QString());    // not implemented
    void zipFile(const QString &path = QString());      // not implemented
    void closeFile();
    void closeAll();
    void newProject();
    void newProjectAccepted();
    void openProject(const QString &path = QString());
    void saveProject();
    void closeProject();
    void aboutShow();
    void helpShow();
    void projectTreeClicked(QModelIndex index);
    void closeTab(int index = 0);
    void changeTab(bool trig);
    void setCurrentBoard(int index);
    void setCurrentPort(int index);
    void connectButton();
    void portResetButton();
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
    void quitProgram();

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
    void replaceInFile();
    void redoChange();
    void undoChange();
    void findDeclaration();
    void findDeclaration(QPoint point);
    void findDeclaration(QTextCursor cur);
    void prevDeclaration();
    int  showDeclaration(QString tagline);
    bool isTagged(QString text);
    void findDeclarationInfo();

    void compileStatusClicked();

    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();

    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    void openRecentFile();

    void setCurrentProject(const QString &fileName);
    void updateRecentProjectActions();
    void openRecentProject();

    void editorMenu(QPoint point);
    void fontDialog();
    void fontBigger();
    void fontSmaller();

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
    int  runPexMake(QString fileName);
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
    void updateProjectTree(QString fileName);
    void setEditorTab(int num, QString shortName, QString fileName, QString text);
    QString shortFileName(QString fileName);
    QString sourcePath(QString file);

    AboutDialog     *aboutDialog;

    QSettings       *settings;
    QString         aSideLoader;
    QString         aSideCompiler;
    QString         aSideCompilerPath;
    QString         aSideIncludes;
    QString         aSideCfgFile;
    QString         aSideSeparator;

    Highlighter     *highlighter;

    QToolBar        *fileToolBar;
    QToolBar        *projToolBar;
    QToolBar        *propToolBar;
    QToolBar        *browseToolBar;
    QToolBar        *programToolBar;
    QToolBar        *ctrlToolBar;

    QSplitter       *leftSplit;
    QSplitter       *rightSplit;

    QTabWidget      *editorTabs;
    QVector<Editor*> *editors;
    QFont           editorFont;
    bool            fileChangeDisable;
    QMenu           *edpopup;

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

    QFileDialog     fileDialog;

    QComboBox       *cbBoard;
    QComboBox       *cbPort;
    QStringList     friendlyPortName;
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
    bool            procDone;
    QMutex          procMutex;

    Hardware        *hardwareDialog;
    QLabel          *status;
    QLabel          *programSize;

    QMenu           *projectMenu;
    QModelIndex     projectIndex;
    QString         projectHome;

    ProjectOptions  *projectOptions;

    // find and replace
    ReplaceDialog   *replaceDialog;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
    QAction *separatorFileAct;

    enum { MaxRecentProjects = 5 };
    QAction *recentProjectActs[MaxRecentProjects];
    QAction *separatorProjectAct;

    CTags           *ctags;
    QToolButton     *btnBrowseBack;
    QToolButton     *btnFindDef;
    NewProject      *newProjDialog;

    QString         lastPath;
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
