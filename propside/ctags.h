#ifndef CTAGS_H
#define CTAGS_H

#include <QtGui>

class CTags : public QObject
{
    Q_OBJECT
public:
    explicit CTags(QString path, QObject *parent = 0);

    int     runCtags(QString path);
    int     runSpinCtags(QString path, QString libpath);
    bool    enabled();
    QString findTag(QString symbol);
    QString getFile(QString line);
    int     getLine(QString line);

    int     tagPush(QString tagline);
    QString tagPop();
    void    tagClear();
    int     tagCount();

private:
    int     spintags(QStringList files);
    int     spintags(QString file);
    int     makeSpinTagMap(QString file, QMap<QString,QString> &map);

private slots:
    void    procError(QProcess::ProcessError);
    void    procReadyRead();
    void    procFinished(int,QProcess::ExitStatus);

private:
    bool        ctagsFound;
    QString     compilerPath;
    QString     ctagsProgram;
    QString     projectPath;
    QString     libraryPath;

    QProcess    *process;
    bool        procDone;
    QMutex      mutex;

    QString     tagFile;
    int         tagLine;
    QStringList tagStack;
};

#endif // CTAGS_H
