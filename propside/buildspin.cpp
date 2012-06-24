#include "buildspin.h"
#include "Sleeper.h"

#include "properties.h"
#include "asideconfig.h"

BuildSpin::BuildSpin(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb, Properties *p)
    : Build(projopts, compstat, stat, progsize, progbar, cb, p)
{
}

int  BuildSpin::runBuild(QString option, QString projfile, QString compiler)
{
    int rc = -1;

    projectFile = projfile;
    aSideCompiler = compiler;
    aSideCompilerPath = sourcePath(compiler);

    QFile file(projfile);
    if(file.exists() == false)
        return rc;
    char buffer[80];
    if(file.open(QFile::ReadOnly)) {
        file.readLine(buffer,80);
        file.close();
    }
    QString spinfile(buffer);

    spinfile = spinfile.trimmed();

    progress->show();
    programSize->setText("");

    compileStatus->setPlainText(tr("Project Directory: ")+sourcePath(projectFile)+"\r\n");
    compileStatus->moveCursor(QTextCursor::End);
    status->setText(tr("Building ...")+" "+spinfile);

    return runBstc(spinfile);
}

int  BuildSpin::makeDebugFiles(QString fileName, QString projfile, QString compiler)
{
    projectFile = projfile;
    aSideCompiler = compiler;
    aSideCompilerPath = sourcePath(compiler);

    return 0;
}


int  BuildSpin::runBstc(QString spinfile)
{
    int rc = 0;

    //getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args;

    /* run the bstc program */
    QString spin = properties->getSpinCompilerStr();
    QString comp = spin.mid(spin.lastIndexOf("/")+1);

    /* other compiler options */
    if(projectOptions->getCompOptions().length()) {
        QStringList complist = projectOptions->getSpinCompOptions().split(" ",QString::SkipEmptyParts);
        foreach(QString compopt, complist) {
            args.append(compopt);
        }
    }

    if(comp.compare("spin",Qt::CaseInsensitive) == 0) {
        // Roy's compiler always makes a .binary
        args.append("-I");
    }
    else {
        // BSTC needs to be told to make a .binary
        args.append("-b");
        args.append("-L");
    }
    args.append(properties->getSpinLibraryStr());

    args.append(spinfile); // using shortname limits us to files in the project directory.

    rc = startProgram(spin, sourcePath(projectFile), args);

    /*
     * Report program size
     * Use the projectFile instead of the current tab file
     */
    QString ssize;
    if(codeSize != 0)
        ssize = QString("Code Size %L1 bytes").arg(codeSize);
    programSize->setText(ssize);

    return rc;
}

void BuildSpin::appendLoaderParameters(QString copts, QString projfile, QStringList *args)
{
    QString filename = projfile.mid(0,projectFile.lastIndexOf("."));
    args->append(copts);
    args->append(filename+".binary");

    qDebug() << args;
}
