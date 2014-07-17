#ifndef PORTCONNECTIONMONITOR_H
#define PORTCONNECTIONMONITOR_H

#include <QtCore>

class PortConnectionMonitor : public QThread
{
    Q_OBJECT
public:
    explicit PortConnectionMonitor(QObject *parent = 0);

    QStringList enumeratePorts();
    void stop();
    void run();

signals:
    void portChanged();

public slots:

private:
    QString pathPrefix;
    QStringList portList;
    bool running;

};

#endif // PORTCONNECTIONMONITOR_H
