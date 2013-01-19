#ifndef BUILD_H
#define BUILD_H

#include <QtGui>
#include "blinker.h"
#include "properties.h"
#include "projectoptions.h"

#define FILELINK " -> "
#define SHOW_ASM_EXTENTION ".asm"
#define SHOW_ASMC_EXTENTION ".asmc"
#define SHOW_MAP_EXTENTION ".map"

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
};

#endif // BUILD_H
