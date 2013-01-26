#ifndef BLINKER_H
#define BLINKER_H

#include <QtGui>
#include <QThread>

class Blinker : public QThread
{
Q_OBJECT;
public:
    Blinker(QLabel *status);
    void run();

private:
    QLabel *status;

signals:
    void statusFailed();
    void statusNone();
};

#endif // BLINKER_H
