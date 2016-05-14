#include "console.h"
#include "xesp8266port.h"

XEsp8266port::XEsp8266port(QObject *parent) : QObject(parent), socket(0), notifier(0), connected(false), signalsConnected(false)
{
    isopen = false;
}

bool XEsp8266port::open(QHostAddress addr, qint64 baudrate)
{
    if (isOpen()) close();

    connected = false;
    if (!signalsConnected)
    {
        connect(&socket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(&socket, SIGNAL(disconnected()), this, SLOT(socketConnectionClosed()));
        connect(&socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
        connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
        signalsConnected = true;
    }

    socket.connectToHost(addr, XEsp8266port::SER_PORT,QTcpSocket::ReadWrite);

    baud = baudrate;

    isopen = true;
    return 0;
}

bool XEsp8266port::isOpen()
{
    bool rc = isopen;
    return rc;
}

bool XEsp8266port::isSequential() const
{
    return true;
}

void XEsp8266port::close()
{
    if (isopen || socket.isOpen()) {
        socket.close();
    }
    isopen = false;
}

void XEsp8266port::flush()
{
    char buf[BUFSIZ];
    int rc = 0;

    qDebug() << "flush Socket.State" << socket.state();

    if (socket.state() != QTcpSocket::ConnectedState) return;
    while (socket.bytesAvailable()) {
        rc = socket.bytesAvailable();
        rc = (rc < BUFSIZ) ? rc : BUFSIZ-1;
        socket.readLine(buf, rc);
    }
}

/*
 * not used
 */
void XEsp8266port::readyRead()
{
    qDebug() << "readyRead Socket.State" << socket.state();

    if (socket.state() != QTcpSocket::ConnectedState) return;
    rxdata = socket.readAll();
}

qint64 XEsp8266port::bytesAvailable() const
{
    //qDebug() << "bytesAvailable Socket.State" << socket.state();

    if (socket.state() != QTcpSocket::ConnectedState) return 0;
    return socket.bytesAvailable() ? 1 : 0;
}

QByteArray XEsp8266port::readAll()
{
    QByteArray ba = socket.readAll();
    //qDebug() << "readAll Socket.State" << socket.state();
    return ba;
}

int XEsp8266port::read(char *buf, qint64 len)
{
    //char *buffer = new char[len+1];
    int rlen = socket.read(buf, len);
    buf[rlen] = '\0';
    return rlen;
}

int XEsp8266port::write(QByteArray barry, int len)
{
    qint64 rc = 0;

    qDebug() << "write Socket.State" << socket.state();

    if(socket.state() != QTcpSocket::ConnectedState)
        return rc;
    rc = socket.write(barry.constData(), (qint64)len);
    return rc;
}

void XEsp8266port::setBaudRate(qint64 baudrate)
{
    baud = baudrate;
}

qint64 XEsp8266port::getBaudRate() const
{
    return baud;
}

void XEsp8266port::setPortName(QString name)
{
    port = name;
}

QString XEsp8266port::getPortName() const
{
    return port;
}

void XEsp8266port::setIpAddress(QString ip)
{
    ipaddr = ip;
}

QString XEsp8266port::getIpAddress() const
{
    return ipaddr;
}

void XEsp8266port::socketConnected()
{
#ifndef Q_OS_MAC
    delete notifier;
#endif
    qDebug() << "socketConnected";

    connected = true;
#ifndef Q_OS_MAC
    notifier = new QSocketNotifier(socket.socketDescriptor(), QSocketNotifier::Exception, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(socketException(int)));
#endif
    emit sockConnected();
}

void XEsp8266port::socketConnectionClosed()
{
    qDebug() << "socketConnectionClosed";
#ifndef Q_OS_MAC
    delete notifier;
    notifier = 0;
#endif
    connected = false;
    emit sockDisconnected();
}

void XEsp8266port::socketReadyRead()
{
    //qDebug() << "socketReadyRead" << socket.readAll();
    emit updateEvent(this);
}

void XEsp8266port::socketError(QAbstractSocket::SocketError error)
{
    qDebug() << "socketError" << error;
    //emit q->connectionError(error);
}

void XEsp8266port::socketException(int)
{
    qDebug("socketException :: out-of-band data received, should handle that here!");
}

bool XEsp8266port::waitForReadyRead(int ms)
{
    socket.waitForReadyRead(ms);
}

bool XEsp8266port::waitForBytesWritten(int ms)
{
    socket.waitForBytesWritten(ms);
}
