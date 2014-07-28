#ifndef LOADER_H
#define LOADER_H

#include "qtversion.h"

class Loader : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Loader(QLabel *mainstatus, QPlainTextEdit *compileStatus, QProgressBar *progressBar, QWidget *parent);
    ~Loader();

    int  load(QString prog, QString path, QStringList args);
    int  reload(QString port);
    void setDisableIO(bool value);
    void setPortEnable(bool value);
    void setRunning(bool running);
    void sendCommand(QString command);
    bool enabled();
    void stop();

    void kill();

protected:
    void keyPressEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

public slots:
    void procStarted();
    void procError(QProcess::ProcessError error);
    void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void procReadyRead();

private:
    void setReady(bool value);

    QString program;
    QString workpath;

    QLabel          *status;
    QPlainTextEdit  *compiler;
    QProgressBar    *progress;
    QPlainTextEdit  *console;
    QProcess        *process;
    QMutex          mutex;
    bool            running;
    bool            ready;
    bool            disableIO;
};

#endif // LOADER_H
