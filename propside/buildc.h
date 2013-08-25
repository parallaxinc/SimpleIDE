#ifndef BUILDC_H
#define BUILDC_H

#include <QtGui>
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
    int ensureOutputDirectory();
    void appendLoaderParameters(QString copts, QString projfile, QStringList *args);

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

    QHash<QString, QString> incHash;
};

#endif // BUILDC_H
