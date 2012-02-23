


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
    bool open();
    void close();
    bool isOpen();
    void setTerminalWindow(QPlainTextEdit *editor);
    void send(QByteArray &data);
    int  readData(char *buff, int length);
    virtual void run();
    void enable(bool value);

private:

    Console         *terminal;
    QextSerialPort  *port;
    QPlainTextEdit  *textEditor;

    bool isEnabled;

private slots:
    void onDsrChanged(bool status);
    void updateReady();
    void updateReady(QextSerialPort*);
    void updateReady(char *buff, int length);

signals:
    void readyRead(int length);
    void updateEvent(QextSerialPort*);
    void updateEvent(char *buff, int length);
};


#endif /*PORTLISTENER_H_*/
