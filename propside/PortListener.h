


#ifndef PORTLISTENER_H_
#define PORTLISTENER_H_

#include <QtGui>
#include "console.h"
#include "qextserialport.h"

class PortListener : public QThread
{
Q_OBJECT
public:
    PortListener(QObject *parent, Console *term);
    void init(const QString &portName, BaudRateType baud);
    void setDtr(bool enable);
    void setRts(bool enable);
    bool open();
    void close();
    bool isOpen();
    void setTerminalWindow(QPlainTextEdit *editor);
    void send(QByteArray &data);
    int  readData(char *buff, int length);
    virtual void run();

private:

    Console         *terminal;
    QextSerialPort  *port;
    QPlainTextEdit  *textEditor;

private slots:
    void onDsrChanged(bool status);
    void updateReady();
    void updateReady(QextSerialPort*);

signals:
    void readyRead(int length);
    void updateEvent(QextSerialPort*);
};


#endif /*PORTLISTENER_H_*/
