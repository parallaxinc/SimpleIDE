


#ifndef PORTLISTENER_H_
#define PORTLISTENER_H_

#include <QtGui>
#include <QObject>
#include "qextserialport.h"

class PortListener : public QObject
{
Q_OBJECT
public:
    PortListener();
    void init(const QString &portName, BaudRateType baud);
    void setDtr(bool enable);
    bool open();
    void close();
    bool isOpen();
    void setTerminalWindow(QPlainTextEdit *editor);
    void send(QByteArray &data);

private:
    QextSerialPort  *port;
    QPlainTextEdit  *textEditor;

    QByteArray buffer;

private slots:
    void onReadyRead();
    void onDsrChanged(bool status);

};


#endif /*PORTLISTENER_H_*/
