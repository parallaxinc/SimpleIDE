#ifndef CTAGS_H
#define CTAGS_H

#include <QtGui>

class CTags : public QObject
{
    Q_OBJECT
public:
    explicit CTags(QString path, QObject *parent = 0);

    int     runCtags(QString path);
    bool    enabled();
    QString findTag(QString symbol);
    QString getFile(QString line);
    int     getLine(QString line);

    int     tagPush(QString tagline);
    QString tagPop();
    void    tagClear();
    int     tagCount();

signals:

private slots:
    void    procError(QProcess::ProcessError);
    void    procReadyRead();
    void    procFinished(int,QProcess::ExitStatus);

private:
    bool        ctagsFound;
    QString     compilerPath;
    QString     ctagsProgram;
    QString     projectPath;

    QProcess    *process;
    bool        procDone;
    QMutex      mutex;

    QString     tagFile;
    int         tagLine;
    QStringList tagStack;
};

#endif // CTAGS_H
