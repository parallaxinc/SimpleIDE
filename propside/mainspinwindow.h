/*
 * mainspinwindow.h defines main window class
 */

#ifndef MAINSPINWINDOW_H
#define MAINSPINWINDOW_H

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
#include "gdb.h"
#include "loader.h"
#include "projecttree.h"
#include "help.h"
#include "build.h"
#include "buildc.h"
#include "buildspin.h"
#include "spinparser.h"

#define untitledstr "Untitled"

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

//! [0]
class MainSpinWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainSpinWindow(QWidget *parent = 0);
    Properties *propDialog;

    enum DumpType { DumpNormal, DumpReadSizes, DumpCat, DumpOff };

public slots:
    void terminalEditorTextChanged();
    void addTab();
    void newFile();
    void openFile(const QString &path = QString());
    void saveFile();
    void saveFileByTabIndex(int tab);
    void saveAsFile(const QString &path = QString());
    void savePexFile();
    void downloadSdCard();
    void printFile();
    void closeFile();
    void closeAll();
    void newProject();
    void newProjectAccepted();
    void openProject(const QString &path = QString());
    void saveAsProject(const QString &path = QString());
    void cloneProject();
    void closeProject();
    void zipProject();
    void aboutShow();
    void creditShow();
    void helpShow();
    void libraryShow();
    void userguideShow();
    void projectTreeClicked(QModelIndex index);
    void closeTab(int index = 0);
    void saveTab(int index = 0);
    void editorTabMenu(QPoint);
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

    void programStopBuild();
    void programBuild();
    void programBurnEE();
    void programRun();
    void programDebug();

    void debugCompileLoad();
    void gdbShowLine();
    void gdbKill();
    void gdbBacktrace();
    void gdbContinue();
    void gdbNext();
    void gdbStep();
    void gdbFinish();
    void gdbUntil();
    void gdbInterrupt();
    void gdbBreak();

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
    void addProjectLibFile();
    void addProjectLink();
    void addProjectIncPath();
    void addProjectLibPath();
    void deleteProjectFile();
    void showProjectFile();

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
    void findSymbolHelp(QString text);

    void compileStatusClicked();
    void compilerChanged();
    void showCompileStatusError();

    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();
    void procReadyReadCat();

    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    void openRecentFile();

    void setCurrentProject(const QString &fileName);
    void updateRecentProjectActions();
    void openRecentProject();

    void fontDialog();
    void fontBigger();
    void fontSmaller();

    void showAssemblyFile();
    void showMapFile();
    int  makeDebugFiles(QString fileName);

    QPlainTextEdit* getDebugEditor();
    void toggleSimpleView();

    void showProjectPane(bool show);
    void showStatusPane(bool show);
    void splitterChanged();

private:
    QString pathDiff(QString s2, QString s1);
    QString saveAsProjectLinkFix(QString srcPath, QString dstPath, QString link);
    void exitSave();
    void getApplicationSettings();
    int  checkCompilerInfo();
    bool isSpinProject();
    bool isCProject();
    void selectBuilder();
    int  runBuild(QString option);
#ifdef KEEP_CTOOLS
    int  runCOGC(QString filename, QString outext);
    int  runBstc(QString spinfile);
    int  runCogObjCopy(QString datfile, QString tarfile);
    int  runObjCopyRedefineSym(QString oldsym, QString newsym, QString file);
    int  runObjCopy(QString datfile);
    int  runGAS(QString datfile);
    int  runPexMake(QString fileName);
    void removeArg(QStringList &list, QString arg);
    QStringList getCompilerParameters(QStringList options);
    int  getCompilerParameters(QStringList copts, QStringList *args);
    int  runCompiler(QStringList options);
#endif
    QStringList getLoaderParameters(QString options);
    int  runLoader(QString options);
#ifdef KEEP_CTOOLS
    int  startProgram(QString program, QString workpath, QStringList args, DumpType dump = DumpOff);
#endif
    int  startProgramTool(QString program, QString workpath, QStringList args);
    int  checkBuildStart(QProcess *proc, QString progName);
    void showBuildStart(QString progName, QStringList args);
    int  buildResult(int exitStatus, int exitCode, QString progName, QString result);

    bool isFileUTF16(QFile *file);
    void openFileName(QString fileName);
    void checkAndSaveFiles();
    void setupEditor();
    void setupFileMenu();
    void setupHelpMenu();
    void setupToolBars();
    void setupProjectTools(QSplitter *vsplit);
    void addToolButton(QToolBar *bar, QToolButton *btn, QString imgfile);
    bool isOutputFile(QString file);
    void addProjectListFile(QString fileName);

    void saveProject();
    void saveProjectOptions();
    void saveSpinProjectOptions();
    void saveManagedProjectOptions();

    void updateProjectTree(QString fileName);
    void updateManagedProjectTree(QString fileName, QString projName);
    void updateSpinProjectTree(QString fileName, QString projName);

    void setEditorTab(int num, QString shortName, QString fileName, QString text);
    QString shortFileName(QString fileName);
    QString sourcePath(QString file);

    void cStatusClicked(QString line);
    void spinStatusClicked(QString line);

    void showSimpleView(bool simple);

    AboutDialog     *aboutDialog;
    QString         aboutLanding;

    Help            *helpDialog;

    QSettings       *settings;
    QString         aSideLoader;
    QString         aSideCompiler;
    QString         aSideCompilerPath;
    QString         aSideIncludes;
    QString         aSideCfgFile;
    QString         aSideSeparator;
    QString         aSideDocPath;
    bool            simpleViewType;

    QToolBar        *fileToolBar;
    QToolBar        *projToolBar;
    QToolBar        *propToolBar;
    QToolBar        *browseToolBar;
    QToolBar        *programToolBar;
    QToolBar        *ctrlToolBar;

    QMenu           *fileMenu;
    QMenu           *projMenu;
    QMenu           *toolsMenu;

    QSplitter       *leftSplit;
    QSplitter       *rightSplit;

    QPushButton     *btnShowProjectPane;
    QPushButton     *btnShowStatusPane;

    QTabWidget      *editorTabs;
    QVector<Editor*> *editors;
    QFont           editorFont;
    bool            fileChangeDisable;
    QMenu           *edpopup;

    QPlainTextEdit  *compileStatus;
    QPlainTextEdit  *debugStatus;

    QString         projectFile;
    CBuildTree      *projectModel;
    ProjectTree     *projectTree;
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
    QToolButton     *btnDownloadSdCard;

#if defined(LOADER_TERMINAL)
    Loader          *termEditor;
#else
    Console         *termEditor;
#endif

    PortListener    *portListener;
    Terminal        *term;

    int             termXpos;
    int             termYpos;

    ASideConfig     *aSideConfig;
    QString         portName;
    QString         boardName;

    QProcess        *process;
    QProgressBar    *progress;
    int             progMax;
    int             progCount;
    bool            procDone;
    bool            procResultError;
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

    QTabWidget      *statusTabs;
    GDB             *gdb;
    QPlainTextEdit  *gdbStatus;
    QPlainTextEdit  *toolStatus;

    QToolButton *btnProgramDebugTerm;
    QToolButton *btnProgramRun;

    Build           *builder;
    BuildC          *buildC;
    BuildSpin       *buildSpin;
    SpinParser      spinParser;
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
