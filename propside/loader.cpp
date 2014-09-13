/*
 * This file is part of the Parallax Propeller SimpleIDE development environment.
 *
 * Copyright (C) 2014 Parallax Incorporated
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "loader.h"
#include "properties.h"
#include "Sleeper.h"

Loader::Loader(QLabel *mainstatus, QPlainTextEdit *compileStatus, QProgressBar *progressBar, QWidget *parent) :
    QPlainTextEdit(parent)
{
    setFont(QFont("courier"));
    status   = mainstatus;
    compiler = compileStatus;
    progress = progressBar;
    setRunning(false);
    setDisableIO(true);
    setReadOnly(false);

    process = new QProcess();
}

Loader::~Loader()
{
    stop();
}

int Loader::load(QString program, QString workpath, QStringList args)
{
    this->program = program;
    this->workpath = workpath;

    stop();
    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(started()),this,SLOT(procStarted()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(this->workpath);
#if 0
    this->port = "";
    for(int n = 0; n < args.length(); n++) {
        QString arg = args.at(n);
        if(arg.contains("-p", Qt::CaseInsensitive)){
            this->port = args.at(n+1);
            break;
        }
    }
#endif
    this->setPlainText("");
    setReady(false);
    setDisableIO(false);
    process->start(this->program,args);

    while(running == false)
        QApplication::processEvents();

    return process->exitCode();
}

int Loader::reload(QString port)
{
    QStringList args;

    if(this->program.length() == 0) {
        QSettings settings(publisherKey, ASideGuiKey,this);
        QVariant incv = settings.value(propLoaderKey);
        if(incv.canConvert(QVariant::String)) {
            QString s = incv.toString();
            s = QDir::fromNativeSeparators(s);
            this->program = s + "propeller-load";
#if defined(Q_OS_WIN32)
            this->program+=".exe";
#endif
        }
    }

    stop();
    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(started()),this,SLOT(procStarted()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(this->workpath);

    if(port.length() > 0) {
        args.append("-p");
        args.append(port);
        args.append("-t");
    }

    setReady(false);
    setDisableIO(false);
    process->start(this->program,args);

    while(running == false)
        QApplication::processEvents();

    return process->exitCode();
}

void Loader::setPortEnable(bool value)
{
    if(value)
        setDisableIO(false);
    else
        setDisableIO(true);
}

void Loader::setDisableIO(bool value)
{
    mutex.lock();
    disableIO = value;
    mutex.unlock();
}

void Loader::setRunning(bool value)
{
    mutex.lock();
    running = value;
    mutex.unlock();
}

void Loader::setReady(bool value)
{
    mutex.lock();
    ready = value;
    mutex.unlock();
}

bool Loader::enabled()
{
    return running;
}

void Loader::stop()
{
    if(running) {
        process->kill();
        //delete process; // can't just delete on Mac
    }
    setRunning(false);
    setReady(false);
    setDisableIO(true);
}

void Loader::kill()
{
    stop();
    this->insertPlainText(tr("\n\nLoader Done ....\n"));
}

void Loader::keyPressEvent(QKeyEvent* e)
{
    int key = e->key();
    if(this->running == false)
        return;

    if(this->disableIO)
        return;

    if(e->matches((QKeySequence::Copy))) {
        copy();
        return;
    }
    if(e->matches((QKeySequence::Paste))) {
        QClipboard *clip = QApplication::clipboard();
        QString command = clip->text();
        this->insertPlainText(command);
        QByteArray barry(command.toLatin1());
        process->write(barry);
        return;
    }

    switch(key)
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            key = '\n';
            break;
        case Qt::Key_Backspace:
            key = '\b';
            break;
        default:
            if(key & Qt::Key_Escape)
                return;
            QChar c = e->text().at(0);
            key = (int)c.toLatin1();
            break;
    }
    QByteArray barry;
    barry.append((char)key);
    process->write(barry);
}

void Loader::mousePressEvent(QMouseEvent* e)
{
    QPlainTextEdit::mousePressEvent(e);
}

void Loader::mouseMoveEvent(QMouseEvent* e)
{
    QPlainTextEdit::mouseMoveEvent(e);
}

void Loader::procStarted()
{
    qDebug() << "Loader::procStarted";
    setRunning(true);
}

void Loader::procError(QProcess::ProcessError error)
{
    qDebug() << "Loader::procError" << error;
}

void Loader::procFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    setRunning(false);
    qDebug() << "Loader::procFinished" << exitCode << exitStatus;
}


void Loader::procReadyRead()
{
    if(this->disableIO)
        return;

    QTextCursor cur;
    QByteArray s = process->readAll();

    if(ready != true) {
        /* propeller-load -t message:
         * [ Entering terminal mode. Type ESC or Control-C to exit. ]\r\n
         */
        if(QString(s).contains(". ]", Qt::CaseInsensitive)) {
            setReady(true);
            status->setText(status->text()+" Loader done.");
            progress->setValue(100);
            Sleeper::ms(250);
            progress->hide();
            s = s.mid(s.indexOf("]")+1);
        }
    }

    if(ready) {
        /* Just doing insertPlainText(s) don't get it.
         * Also we need to add character enable filters simiar to PST
         */
        for(int n = 0; n < s.length();n++) {
            char ch = QChar(s.at(n)).toLatin1();
            //this->insertPlainText(QString(" %1").arg(ch, 2, 16, QChar('0')));
            if(ch == '\0')
                continue; // for now ignore 0's
            if(ch == '\r')
                continue; // for now ignore \r
            if(ch == '\b') {
                QString text = this->toPlainText();
                this->setPlainText(text.mid(0,text.length()-1));
                n+=2;
                continue;
            }
            this->insertPlainText(QString(ch));
        }
        cur = this->textCursor();
        cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        this->setTextCursor(cur);
    }
    else {
        /* insertPlainText OK here - it's not too critical
         */
        compiler->insertPlainText(s);
        cur = compiler->textCursor();
        cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        compiler->setTextCursor(cur);
    }
}

