#include "gdb.h"
#include "Sleeper.h"

#define GDBPROMPT "(gdb)"

GDB::GDB(QPlainTextEdit *terminal, QObject *parent) :
    QObject(parent)
{
    status = terminal;
    gdbRunning = false;
    process = new QProcess(this);
}

GDB::~GDB()
{
    stop();
}

void GDB::load(QString program, QString workpath, QString target, QString image, QString port)
{
    /*
     * this is the asynchronous method.
     */
    process->setProperty("Name", QVariant(program));
    process->setProperty("IsLoader", QVariant(false));

    stop();
    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(started()),this,SLOT(procStarted()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(workpath);

    QStringList args;
    args.append(image);

    status->setPlainText("");
    status->insertPlainText(tr("Starting gdb ... "));
    process->start(program,args);

    while(gdbRunning == false)
        QApplication::processEvents();

    //sendCommand("set listsize 1");
    //sendCommand("target remote | " + target); // + " -v -l gdblog.txt");
    sendCommand("break main");
    sendCommand("continue");
}

void GDB::setRunning(bool running)
{
    mutex.lock();
    gdbRunning = running;
    mutex.unlock();
}

void GDB::setReady(bool ready)
{
    mutex.lock();
    gdbReady = ready;
    mutex.unlock();
}

void GDB::sendCommand(QString command)
{
    if(gdbRunning == false) {
        qDebug() << "GDBsend:" << command << ". GDB is not running.";
        return;
    }

    while(gdbReady == false)
        QApplication::processEvents();

    setReady(false);
    command += "\n";
    status->insertPlainText(command);
    QByteArray barry(command.toAscii());
    process->write(barry);

}

bool GDB::enabled()
{
    return gdbRunning;
}

void GDB::stop()
{
    if(gdbRunning) {
        process->close();
    }
    setRunning(false);
    setReady(false);
}

bool GDB::parseResponse(QString resp)
{
    bool ok;
    QString file;
    QString line;
    QString fileStr(": file ");
    QString lineStr(", line ");
    QString brkStr("Breakpoint");
    QString atStr(") at ");

    if(resp.length() == 0)
        return false;
    if(resp.contains("warning:",Qt::CaseInsensitive))
        return false;
    if(resp.contains("packet error",Qt::CaseInsensitive))
        return false;
    if(resp.contains("failure reply:",Qt::CaseInsensitive))
        return false;

    QStringList list = resp.split("\n",QString::SkipEmptyParts);

    /* if we get one line + (gdb)
     * just use the last filename if the number parses ok
     */
    if(list.length() == 2) {
        QString s = list.at(1);
        if(s.contains(GDBPROMPT)) {
            s = list.at(0);
            s = s.mid(0,s.indexOf(' '));
            lineNumber = s.toInt(&ok);
            if(fileName.length() > 0)
                return ok;
        }
    }

    for(int n = 0; n < list.length(); n++) {
        QString s = list.at(n);

        /* look for pattern "Breakpoint.*) at "*/
        if(s.contains(brkStr,Qt::CaseInsensitive) && s.contains(atStr)) {
            s = s.mid(s.indexOf(atStr)+atStr.length());
            file = s.mid(0,s.lastIndexOf(":"));
            line = s.mid(s.lastIndexOf(":")+1);
            if(file.length() == 0 || line.length() == 0)
                return false;
            fileName = file;
            lineNumber = line.toInt(&ok);
            return ok;
        }
        /* look for pattern ": file " and ", line " */
        else
        if(s.indexOf(fileStr) > -1) {
            s = s.mid(s.indexOf(fileStr)+fileStr.length());
            file = s.mid(0,s.lastIndexOf(","));
            s = s.mid(s.indexOf(lineStr)+lineStr.length());
            line = s.mid(0, s.lastIndexOf("."));
            if(file.length() == 0 || line.length() == 0)
                return false;
            fileName = file;
            lineNumber = line.toInt(&ok);
            return ok;
        }
    }
    return false;
}

QString GDB::getResponseFile()
{
    return fileName;
}

int     GDB::getResponseLine()
{
    return lineNumber;
}

void GDB::kill()
{
    stop();
    status->insertPlainText(tr("\n\nDebugger Done ....\n"));
}

void GDB::backtrace()
{
    sendCommand("backtrace");
}

void GDB::runProgram()
{
    sendCommand("continue");
}

void GDB::next()
{
    sendCommand("next");
}

void GDB::step()
{
    sendCommand("step");
}

void GDB::finish()
{
    sendCommand("finish");
}

/* TODO probably broken */
void GDB::interrupt()
{
    sendCommand("\3");
}

void GDB::until()
{
    sendCommand("until");
}

void GDB::procStarted()
{
    qDebug() << "GDBprocStarted";
    setRunning(true);
}

void GDB::procError(QProcess::ProcessError error)
{
    qDebug() << "GDBprocError" << error;
}

void GDB::procFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    setRunning(false);
    qDebug() << "GDBprocFinished" << exitCode << exitStatus;
}

void GDB::procReadyRead()
{
    QString s = process->readAllStandardOutput();
    if(gdbRunning) {
        if(parseResponse(s)) {
            emit breakEvent();
        }
    }
    if(s.indexOf(GDBPROMPT) > -1) {
        setReady(true);
    }
    status->insertPlainText(s);
    QTextCursor cur = status->textCursor();
    cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    status->setTextCursor(cur);
}

