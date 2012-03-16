#ifndef GDB_H
#define GDB_H

#include <QtCore>
#include "terminal.h"

class GDB : public QObject
{
    Q_OBJECT
public:
    explicit GDB(QPlainTextEdit *terminal, QObject *parent = 0);
    ~GDB();

    void load(QString gdbprog, QString path, QString target, QString image, QString port);
    void setRunning(bool running);
    void setReady(bool ready);
    void sendCommand(QString command);

    bool enabled();
    void stop();
    bool parseResponse(QString resp);

    QString getResponseFile();
    int     getResponseLine();

    void kill();
    void backtrace();
    void runProgram();
    void next();
    void step();
    void interrupt();
    void finish();
    void until();

signals:
    void breakEvent();

public slots:
    void procStarted();
    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();

private:
    QPlainTextEdit  *status;
    QProcess        *process;
    QMutex          mutex;
    bool            gdbRunning;
    bool            gdbReady;

    bool            programRunning;

    QString         fileName;
    int             lineNumber;
};

#endif // GDB_H
