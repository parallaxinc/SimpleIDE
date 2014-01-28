#include "build.h"
#include "Sleeper.h"

Build::Build(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb, Properties *p)
{
    projectOptions = projopts;
    compileStatus = compstat;
    status = stat;
    programSize = progsize;
    progress = progbar;
    cbBoard = cb;
    properties = p;

    process = new QProcess();
    blinker = new Blinker(status);

    connect(blinker, SIGNAL(statusNone()), this, SLOT(statusNone()));
    connect(blinker, SIGNAL(statusFailed()), this, SLOT(statusFailed()));

    separator = "/";
}


/*
 * virtual, must be overloaded.
 */
int Build::runBuild(QString options, QString projfile, QString compiler)
{
    if(options.isEmpty() || projfile.isEmpty() || compiler.isEmpty())
        return -1;
    return -1;
}

/*
 * virtual, must be overloaded.
 */
int Build::makeDebugFiles(QString fileName, QString projfile, QString compiler)
{
    if(fileName.isEmpty() || projfile.isEmpty() || compiler.isEmpty())
        return -1;
    return -1;
}

/*
 * virtual, must be overloaded.
 */
QString Build::getOutputPath(QString projfile)
{
    if(projfile.isEmpty())
        return "";
    return "";
}

void Build::abortProcess()
{
    if(procDone != true) {
        procMutex.lock();
        procDone = true;
        procMutex.unlock();
        QApplication::processEvents();
        //process->kill(); // don't kill here. let the user process that is waiting kill it.
    }
}

int  Build::startProgram(QString program, QString workpath, QStringList args, DumpType dump)
{
    /*
     * this is the asynchronous method.
     */
    showBuildStart(program,args);

#if !defined(Q_WS_WIN32)
    if(program.contains(aSideCompilerPath) == false)
        program = aSideCompilerPath + program;
#endif

    process->setProperty("Name", QVariant(program));
    process->setProperty("IsLoader", QVariant(false));

    if(dump == this->DumpReadSizes) {
        disconnect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
        connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyReadSizes()));
    }
    else {
        connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    }
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(workpath);

    procDone = false;
    procResultError = false;
    process->start(program,args);

    this->codeSize = 0;

    /* process Qt application events until procDone
     */
    while(procDone == false) {
        QApplication::processEvents();
        Sleeper::ms(5);
    }

    int killed = 0;
    if(process->state() == QProcess::Running) {
        process->kill();
        Sleeper::ms(500);
        compileStatus->appendPlainText(tr("Program killed by user."));
        status->setText(status->text() + tr(" Done."));
        killed = -1;
    }

    disconnect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyReadSizes()));

    //progress->hide();

    if(procResultError)
        return 1;
    return process->exitCode() | killed;
}

void Build::procError(QProcess::ProcessError error)
{
    if(procDone == true)
        return;

    QVariant name = process->property("Name");
    compileStatus->appendPlainText(name.toString() + tr(" error ... (%1)").arg(error));
    compileStatus->appendPlainText(process->readAllStandardOutput());

    procMutex.lock();
    procDone = true;
    procMutex.unlock();
}

void Build::procFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(procDone == true)
        return;

    procMutex.lock();
    procDone = true;
    procMutex.unlock();

    QVariant name = process->property("Name");
    buildResult(exitStatus, exitCode, name.toString(), process->readAllStandardOutput());

    int len = status->text().length();
    QString s = status->text().mid(len-8);
    /*
    if(s.contains("done.",Qt::CaseInsensitive) == false)
        status->setText(status->text()+" done.");
    */
}

/*
 * save for cat dumps
 */
void Build::procReadyReadCat()
{

}

/*
 * read program sizes from objdump -h
 */
void Build::procReadyReadSizes()
{
    int rc;
    bool ok;
    QByteArray bytes = process->readAllStandardOutput();
    if(bytes.length() == 0)
        return;

    this->codeSize = 0;
    this->memorySize = 0;
    QStringList lines = QString(bytes).split("\n",QString::SkipEmptyParts);
    int len = lines.length();
    for (int n = 0; n < len; n++) {
        QString line = lines[n];
        if(line.length() > 0) {
            QString ms = line.mid(line.indexOf("."));
            QRegExp regex("[ \t]");
            QStringList more = line.split(regex,QString::SkipEmptyParts);
            if(ms.contains(".bss",Qt::CaseInsensitive)) {
                if(more.length() > 2) {
                    rc = more.at(2).toInt(&ok,16);
                    if(ok) {
                        this->memorySize += rc;
                    }
                }
            }
            else
            if(ms.contains("heap",Qt::CaseInsensitive)) {
                //this->codeSize += 4;
                //this->memorySize += 4;
                break;
            }
            else if (n < len-1){
                if(QString(lines.at(n+1)).contains("load", Qt::CaseInsensitive)) {
                    if(more.length() > 2) {
                        rc = more.at(2).toInt(&ok,16);
                        if(ok) {
                            this->codeSize += rc;
                            this->memorySize += rc;
                        }
                    }
                }
            }
        }
    }
}

void Build::procReadyRead()
{
    QByteArray bytes = process->readAllStandardOutput();
    if(bytes.length() == 0)
        return;

#if defined(Q_WS_WIN32)
    QString eol("\r");
#else
    QString eol("\n");
#endif
    bytes = bytes.replace("\r\n","\n");

    // bstc doesn't return good exit status
    QString progname;
    QVariant pvar = process->property("Name");
    if(pvar.canConvert(QVariant::String)) {
        progname = pvar.toString();
    }
    bool isbstc = false;
    if(progname.contains("bstc",Qt::CaseInsensitive)) {
        isbstc = true;
        if(QString(bytes).contains("Error",Qt::CaseInsensitive)) {
            procResultError = true;
        }
        if(QString(bytes).contains("longs",Qt::CaseInsensitive)) {
            bytes = bytes.replace("longs", "bytes");
        }
    }
    if(progname.contains("propbasic",Qt::CaseInsensitive)) {
        isbstc = true;
        if(QString(bytes).contains("Error",Qt::CaseInsensitive) && !QString(bytes).contains("0 Error",Qt::CaseInsensitive)) {
            procResultError = true;
        }
        if(QString(bytes).contains("longs",Qt::CaseInsensitive)) {
            bytes = bytes.replace("longs", "bytes");
        }
    }

    if(progname.contains("propeller-elf-gcc") && bytes.contains("gcc version")) {
        QString line = QString(bytes);
        QStringList lines = line.split("gcc version",QString::SkipEmptyParts);
        if(lines.count() > 0) {
            compileStatus->insertPlainText(" GCC "+QString(lines[1]).trimmed());
            return;
        }
    }

    QStringList lines = QString(bytes).split("\n",QString::SkipEmptyParts);
    if(bytes.contains("bytes")) {
        for (int n = 0; n < lines.length(); n++) {
            QString line = lines[n];
            if(line.length() > 0) {
                if(line.indexOf("\r") > -1) {
                    QStringList more = line.split("\r",QString::SkipEmptyParts);
                    lines.removeAt(n);
                    for(int m = more.length()-1; m > -1; m--) {
                        QString ms = more.at(m);
                        if(ms.contains("bytes",Qt::CaseInsensitive))
                            lines.insert(n,more.at(m));
                        if(ms.contains("loading",Qt::CaseInsensitive))
                            lines.insert(n,more.at(m));
                    }
                }
            }
        }
    }

    for (int n = 0; n < lines.length(); n++) {
        QString line = lines[n];
        if(line.length() > 0) {
            compileStatus->moveCursor(QTextCursor::End);
            if(line.contains("Propeller Version",Qt::CaseInsensitive)) {
                compileStatus->insertPlainText(line+eol);
                progress->setValue(0);
            }
            else
            if(line.contains("loading",Qt::CaseInsensitive) && !isbstc) {
                progMax = 0;
                progress->setValue(0);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("writing",Qt::CaseInsensitive)) {
                progMax = 0;
                progress->setValue(0);
            }
            else
            if(line.contains("Download OK",Qt::CaseInsensitive)) {
                progress->setValue(100);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("sent",Qt::CaseInsensitive)) {
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("remaining",Qt::CaseInsensitive)) {
                if(progMax == 0) {
                    QString bs = line.mid(0,line.indexOf(" "));
                    progMax = bs.toInt();
                    progMax /= 1024;
                    progMax++;
                    progCount = 0;
                    if(progMax == 0) {
                        progress->setValue(100);
                    }
                }
                if(progMax != 0) {
                    progCount++;
                    progress->setValue(100*progCount/progMax);
                }
                compileStatus->moveCursor(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
                compileStatus->insertPlainText(line);
            }
            else
            if(line.contains("Program size",Qt::CaseInsensitive)) {
                // bstc reports program size is N longs
                compileStatus->insertPlainText(eol);
                compileStatus->insertPlainText(line);
                QString s = line.mid(line.lastIndexOf("is ")+3);
                s = s.mid(0,s.lastIndexOf(" "));
                bool ok = false;
                int size =  s.toInt(&ok);
                this->codeSize = ok ? size : 0;
            }
            else {
                if(n < lines.length())
                    compileStatus->insertPlainText(eol);
                compileStatus->insertPlainText(line);
            }
        }
    }

}

int  Build::checkBuildStart(QProcess *proc, QString progName)
{
    QMessageBox mbox;
    mbox.setStandardButtons(QMessageBox::Ok);
    //qDebug() << QDir::currentPath();
    if(!proc->waitForStarted()) {
        mbox.setInformativeText(progName+tr(" Could not start."));
        mbox.exec();
        return -1;
    }
    if(!proc->waitForFinished()) {
        mbox.setInformativeText(progName+tr(" Error waiting for program to finish."));
        mbox.exec();
        return -1;
    }
    return 0;
}

void Build::showBuildStart(QString progName, QStringList args)
{
    QString argstr = "";
    for(int n = 0; n < args.length(); n++)
        argstr += " "+args[n];
    //qDebug() << progName+argstr;
    compileStatus->appendPlainText(shortFileName(progName)+argstr);

    while(blinker->isRunning()) {
        QApplication::processEvents();
    }
    statusNone();
}

int  Build::buildResult(int exitStatus, int exitCode, QString progName, QString result)
{
    QMessageBox mbox;
    mbox.setStandardButtons(QMessageBox::Ok);
    mbox.setInformativeText(result);

    progress->setVisible(false);
    if(exitStatus == QProcess::CrashExit)
    {
        status->setText(status->text()+" "+shortFileName(progName)+" "+tr("Compiler Crashed"));
        mbox.setText(progName+" "+tr("Compiler Crashed"));
        mbox.exec();
    }
    else if(result.toLower().indexOf("error") > -1)
    { // just in case we get an error without exitCode
        status->setText(status->text()+" "+shortFileName(progName)+tr(" Error:")+result);
        if(progName.contains("load",Qt::CaseInsensitive))
            mbox.setText(tr("Load Error"));
        else {
            if(result.contains("port",Qt::CaseInsensitive))
                mbox.setText(tr("Serial Port Error"));
            else
                mbox.setText(tr("Build Error"));
        }
        mbox.exec();
    }
    else if(exitCode != 0)
    {
        if(compileStatus->toPlainText().contains("error")) {
            QString errstr;
            QStringList list = compileStatus->toPlainText().split("\n");
            for(int n = list.length()-1; n > -1; n--) {
                if(QString(list[n]).contains("error")) {
                    errstr = list[n];
                }
            }
            status->setText(status->text()+" "+errstr+". ");
        }
        else {
            status->setText(status->text()+" "+shortFileName(progName)+tr(" Error: ")+QString("%1").arg(exitCode));
        }
    }
    else if(result.toLower().indexOf("warning") > -1)
    {
        status->setText(status->text()+" "+shortFileName(progName)+tr(" Compiled OK with Warning(s)."));
        statusPassed();
        return 0;
    }
    else
    {
        /* we can show progress of individual build steps, but that makes status unreasonable. */
        statusPassed();
        return 0;
    }

    statusFailed();
    blinker->start(); // run status timer
    emit showCompileStatusError();

    return -1;
}

void Build::statusNone()
{
    status->setStyleSheet("QLabel { background-color: palette(window) }");
}

void Build::statusFailed()
{
    status->setStyleSheet("QLabel { background-color: rgb(255,0,0) }");
}

void Build::statusPassed()
{
    status->setStyleSheet("QLabel { background-color: rgb(0,200,0); }");
}

void Build::compilerError(QProcess::ProcessError error)
{
    qDebug() << error;
}

void Build::compilerFinished(int exitCode, QProcess::ExitStatus status)
{
    qDebug() << exitCode << status;
}


int  Build::checkCompilerInfo()
{
    QMessageBox mbox(QMessageBox::Critical,tr("Build Error"),"",QMessageBox::Ok);
    if(aSideCompiler.length() == 0) {
        mbox.setInformativeText(tr("Please specify compiler application in properties."));
        mbox.exec();
        return -1;
    }
#ifdef KEEP
    if(aSideIncludes.length() == 0) {
        mbox.setInformativeText(tr("Please specify loader folder in properties."));
        mbox.exec();
        return -1;
    }
#endif
    return 0;
}

QString Build::sourcePath(QString srcpath)
{
    srcpath = QDir::fromNativeSeparators(srcpath);
    srcpath = srcpath.mid(0,srcpath.lastIndexOf("/")+1);
    return srcpath;
}

QString Build::shortFileName(QString fileName)
{
    QString rets;
    if(fileName.indexOf('/') > -1)
        rets = fileName.mid(fileName.lastIndexOf('/')+1);
    else if(fileName.indexOf('\\') > -1)
        rets = fileName.mid(fileName.lastIndexOf('\\')+1);
    else
        rets = fileName.mid(fileName.lastIndexOf(separator)+1);
    return rets;
}


void Build::removeArg(QStringList &list, QString arg)
{
    int len = list.length();
    for(int n = 0; n < len; n++) {
        QString s = list.at(n);
        if(s.compare(arg) == 0) {
            list.removeAt(n);
            return;
        }
    }
}

void Build::appendLoaderParameters(QString copts, QString projfile, QStringList *args)
{
    if(copts.isEmpty() || projfile.isEmpty() || args->isEmpty())
        return;
}
