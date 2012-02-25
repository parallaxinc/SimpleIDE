#include "ctags.h"

CTags::CTags(QString path, QObject *parent) : QObject(parent)
{
    QString ctags("ctags");

    compilerPath = path;
    ctagsProgram = path+ctags;

#if defined(Q_WS_WIN32)
    ctagsProgram = ctagsProgram+".exe";
#endif

    if(QFile::exists(ctagsProgram))
        ctagsFound = true;
    else
        ctagsFound = false;

    process = new QProcess();
}

int CTags::runCtags(QString path)
{
    int rc = -1;
    QStringList args;

    if(ctagsFound == false)
        return rc;

    projectPath = path;

    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(path);

    args.append("--format=1");

    QDir dir(path);
    dir.setFilter(QDir::Files);
    QFileInfoList fileList = dir.entryInfoList();
    foreach(QFileInfo file, fileList) {
        if(file.fileName().contains(".out"))
            continue;
        args.append(path+file.fileName());
    }

    procDone = false;
    process->start(ctagsProgram,args);

    /* process Qt application events until procDone
     */
    while(procDone == false)
        QApplication::processEvents();

    rc = process->exitCode();
    return rc;
}

void CTags::procError(QProcess::ProcessError code)
{
    qDebug() << "procError " << code;
}

void CTags::procReadyRead()
{
    qDebug() << "procReadyRead :" << process->readAllStandardOutput();
}

void CTags::procFinished(int code, QProcess::ExitStatus status)
{
    if(procDone)
        return;

    mutex.lock();
    procDone = true;
    mutex.unlock();

    qDebug() << "procFinished " << code << " " << status << ":" << process->readAllStandardOutput();
}

bool CTags::enabled()
{
    return ctagsFound;
}

QString CTags::findTag(QString symbol)
{
    QString rets("");
    if(ctagsFound == false)
        return rets;

    QFile file(projectPath+"tags");
    if(file.exists() == false)
        return rets;

    QString fileString;
    if(file.open(QFile::ReadOnly)) {
        fileString = file.readAll();
        file.close();
    }
    if(fileString.contains(symbol)) {
        QStringList list = fileString.split("\n");
        QRegExp rx("^"+symbol+"[ \t].*");
        for(int n = list.length()-1; n >= 0; n--) {
            QString line = list.at(n);
            if(line.length() == 0)
                continue;
            if(line.at(0) == '!')
                continue;
            int pos = rx.indexIn(line);
            if(pos > -1) {
                rets = line;
                break;
            }
        }
    }
    return rets;
}

QString CTags::getFile(QString line)
{
    QStringList item = line.split("\t");
    return item.at(1);
}

int CTags::getLine(QString line)
{
    QStringList item = line.split("\t");
    QString rspec = item.at(2);
    if(rspec.indexOf('^') > -1)
        rspec = rspec.mid(rspec.indexOf('^')+1);
    if(rspec.lastIndexOf('$') > 0)
        rspec = rspec.mid(0,rspec.lastIndexOf('$'));
    QRegExp rx(rspec,Qt::CaseSensitive, QRegExp::RegExp);
    QRegExp rx2(rspec,Qt::CaseSensitive, QRegExp::RegExp2);
    QRegExp rxwc(rspec,Qt::CaseSensitive, QRegExp::Wildcard);
    QRegExp rxwcu(rspec,Qt::CaseSensitive, QRegExp::WildcardUnix);
    QRegExp rxfs(rspec,Qt::CaseSensitive, QRegExp::FixedString);
    QRegExp rxw3(rspec,Qt::CaseSensitive, QRegExp::W3CXmlSchema11);

    QFile file(item.at(1));
    QString filestr;
    if(file.open(QFile::ReadOnly)) {
        filestr = file.readAll();
        file.close();
#if 0
        /* could return the file position of the search string */
        int pos = rxwc.indexIn(filestr);
        if(pos > -1)
            return pos;
#else
        QStringList list = filestr.split("\n");
        for(int n = 0; n < list.length(); n++) {
            QString line = list.at(n)+"\n";
            int pos = rx.indexIn(line);
            pos &= rx2.indexIn(line);
            pos &= rxwc.indexIn(line);
            pos &= rxwcu.indexIn(line);
            pos &= rxfs.indexIn(line);
            pos &= rxw3.indexIn(line);
            if(pos > -1)
                return n;
        }
#endif
    }
    return -1;
}

int CTags::tagPush(QString tagline)
{
    tagStack.append(tagline);
    return tagStack.count();
}

QString CTags::tagPop()
{
    QString tagline;
    tagline = tagStack.at(tagStack.count()-1);
    tagStack.removeLast();
    return tagline;
}

void CTags::tagClear()
{
    tagStack.clear();
}

int CTags::tagCount()
{
    return tagStack.count();
}

