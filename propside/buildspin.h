#ifndef BUILDSPIN_H
#define BUILDSPIN_H

#include <QtGui>
#include "build.h"

class BuildSpin : public Build
{
    Q_OBJECT
public:
    BuildSpin(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb);

    int  runBuild(QString option, QString projfile, QString compiler);
    int  makeDebugFiles(QString fileName, QString projfile, QString compiler);

    int  runBstc(QString spinfile);

};

#endif // BUILDSPIN_H
