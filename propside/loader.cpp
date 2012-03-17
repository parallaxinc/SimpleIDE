#include "loader.h"
#include "properties.h"
#include "Sleeper.h"

Loader::Loader(QLabel *mainstatus, QPlainTextEdit *compileStatus, QWidget *parent) :
    QPlainTextEdit(parent)
{
    setFont(QFont("courier"));
    status   = mainstatus;
    compiler = compileStatus;
    setRunning(false);
    setDisableIO(true);
}

Loader::~Loader()
{
    stop();
}

int Loader::load(QString program, QString workpath, QStringList args)
{
    this->program = program;
    this->workpath = workpath;

    process = new QProcess();

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
        QVariant incv = settings.value(includesKey);
        if(incv.canConvert(QVariant::String)) {
            QString s = incv.toString();
            s = QDir::fromNativeSeparators(s);
            this->program = s + "propeller-load";
#if defined(Q_WS_WIN32)
            this->program+=".exe";
#endif
        }
    }

    process = new QProcess();

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
        delete process;
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
        key = (int)c.toAscii();
        break;
    }
    QByteArray barry;
    barry.append((char)key);
    process->write(barry);

    QPlainTextEdit::keyPressEvent(e);

#if 0
    if(e->matches((QKeySequence::Copy))) {
        copy();
    }
    if(e->matches((QKeySequence::Paste))) {
        QClipboard *clip = QApplication::clipboard();
        QString command = clip->text();
        this->insertPlainText(command);
        QByteArray barry(command.toAscii());
        process->write(barry);
        return;
    }
    else {
        if(QApplication::keyboardModifiers() == 0) {
            //qDebug() << "keyPressEvent" << key;
            QByteArray barry;
            if(key == Qt::Key_Enter || key == Qt::Key_Return)
                barry.append('\n');
            else
                barry.append((char)key);
            process->write(barry);
        }
    }
#endif
}

void Loader::mousePressEvent(QMouseEvent* e)
{

}

void Loader::mouseMoveEvent(QMouseEvent* e)
{

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
    QTextCursor cur;
    QString s = process->readAllStandardOutput();

    if(this->disableIO)
        return;

    if(s.contains("[ Entering terminal mode.",Qt::CaseInsensitive)) {
        setReady(true);
        QStringList list = s.split("\n");
        if(list.count() > 0)
            compiler->insertPlainText(list.at(0));
        if(list.count() > 1)
            s = list.at(1);
    }

    if(ready) {
        this->insertPlainText(s);
        cur = this->textCursor();
        cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        this->setTextCursor(cur);
    }
    else {
        compiler->insertPlainText(s);
        cur = compiler->textCursor();
        cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        compiler->setTextCursor(cur);
    }
}

