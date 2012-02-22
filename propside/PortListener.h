


#ifndef PORTLISTENER_H_
#define PORTLISTENER_H_

#include <QtGui>
#include "qextserialport.h"

class PortListener : public QThread
{
Q_OBJECT
public:
    PortListener(QObject *parent);
    void init(const QString &portName, BaudRateType baud);
    void setDtr(bool enable);
    bool open();
    void close();
    bool isOpen();
    void setTerminalWindow(QPlainTextEdit *editor);
    void send(QByteArray &data);
    virtual void run();
    void enable(bool value);

private:
    QextSerialPort  *port;
    QPlainTextEdit  *textEditor;

    bool isEnabled;

private slots:
    void onReadyRead(int length);
    void onDsrChanged(bool status);

signals:
    void readyRead(int length);
};


#endif /*PORTLISTENER_H_*/
