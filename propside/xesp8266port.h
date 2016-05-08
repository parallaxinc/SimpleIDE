#ifndef WIFIESP8266_H
#define WIFIESP8266_H

#include <QObject.h>
#include <QHostAddress.h>
#include <QTcpSocket>
#include <QSocketNotifier>

struct XEspInfo {
    qint32 ipAddr;
    qint32 macAddrHigh;
    qint32 macAddrLow;
    qint32 firmwareVersion;
    qint32 cfgChecksum;
    QString nodeID;
};

class XEsp8266port : public QObject
{
    Q_OBJECT
public:
    XEsp8266port(QObject *parent = 0);

    bool open(QHostAddress addr, qint64 baudrate);
    bool isOpen();
    bool isSequential() const;
    void close();
    void flush();

    qint64 bytesAvailable() const;
    QByteArray readAll();
    int read(char *buf, qint64 len);
    int write(QByteArray barry, int len);

    void setBaudRate(qint64 baudrate);
    qint64 getBaudRate() const;
    void setPortName(QString name);
    QString getPortName() const;
    void setIpAddress(QString ipaddr);
    QString getIpAddress() const;

    bool waitForReadyRead(int ms);
    bool waitForBytesWritten(int ms);

private:
    enum { SER_PORT = 23 };

private slots:

    void socketConnected();
    void socketConnectionClosed();
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError error);
    void socketException(int);

signals:
    void sockConnected();
    void sockDisconnected();
    void updateEvent(XEsp8266port*);

public slots:
    void readyRead();

private:
    QString port;
    QString ipaddr;
    int  baud;
    bool isopen;
    bool connected;
    bool signalsConnected;

    QByteArray rxdata;

    QTcpSocket socket;
    QSocketNotifier *notifier;
};

#endif // WIFIESP8266_H
