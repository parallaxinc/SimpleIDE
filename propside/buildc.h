#ifndef BUILDC_H
#define BUILDC_H

#include <QtGui>
#include "build.h"

class BuildC : public Build
{
    Q_OBJECT
public:
    BuildC(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb);

    int  runBuild(QString option, QString projfile, QString compiler);
    int  makeDebugFiles(QString fileName, QString projfile, QString compiler);

    int  runCOGC(QString filename, QString outext);
    int  runBstc(QString spinfile);
    int  runCogObjCopy(QString datfile, QString tarfile);
    int  runObjCopyRedefineSym(QString oldsym, QString newsym, QString file);
    int  runObjCopy(QString datfile);
    int  runGAS(QString datfile);
    int  runPexMake(QString fileName);
    int  runCompiler(QStringList copts);

    QStringList getCompilerParameters(QStringList copts);
    int  getCompilerParameters(QStringList copts, QStringList *args);
};

#endif // BUILDC_H
