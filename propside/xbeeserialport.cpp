#include "console.h"
#include "xbeeserialport.h"

XBeeSerialPort::XBeeSerialPort(QObject *parent) : QObject(parent)
{
    isopen = false;
}

bool XBeeSerialPort::open(QHostAddress addr, qint64 baudrate)
{
    QUdpSocket sock;

    sock.connectToHost(addr, XBEE_APP_PORT);
    QHostAddress localAddress = sock.localAddress();
    myhostaddr = localAddress.toIPv4Address();
    sock.close();

    remoteaddr = addr.toIPv4Address();
    baud = baudrate;

    setItem(xbSerialIP, serialUDP);             // Ensure XBee's Serial Service uses UDP packets [WRITE DISABLED DUE TO FIRMWARE BUG]
    setItem(xbIPDestination, myhostaddr);       // Ensure Serial-to-IP destination is us (our IP)
    setItem(xbIPPort, XBEE_SER_PORT);           // Ensure Serial-to-IP port is proper (default, in this case)
    //setItem(xbDestPort, XBEE_SER_PORT);
    setItem(xbOutputMask, 0x7FFF);              // Ensure output mask is proper (default, in this case)
    setItem(xbRTSFlow, pinEnabled);             // Ensure RTS flow pin is enabled (input)
    setItem(xbIO4Mode, pinOutLow);              // Ensure serial hold pin is set to output low
    setItem(xbIO2Mode, pinOutHigh);             // Ensure reset pin is set to output high
    setItem(xbIO4Timer, 2);                     // Ensure serial hold pin's timer is set to 200 ms
    setItem(xbIO2Timer, 1);                     // Ensure reset pin's timer is set to 100 ms
    setItem(xbSerialMode, transparentMode);     // Ensure Serial Mode is transparent [WRITE DISABLED DUE TO FIRMWARE BUG]
    setItem(xbSerialBaud, this->baudRate());    // Ensure baud rate is set to initial speed
    setItem(xbSerialParity, parityNone);        // Ensure parity is none
    setItem(xbSerialStopBits, stopBits1);       // Ensure stop bits is 1
    setItem(xbPacketingTimeout, 3);             // Ensure packetization timout is 3 character times

    if (isOpen()) close();

    /*
     * We will use socket datagram functions in this driver.
     * Only socket.bind is used for connections.
     * socket.connectToHost is not necessary with hasPendingDatagrams(),
     * pendingDatagramSize(), readDatagram(), and writeDatagram().
     *
     * If we decide to use QIODevice read(), readAll(), readLine(), etc...
     * socket.connectToHost(addr, XBEE_SER_PORT) needs to be called.
     */
    if (!socket.bind(XBEE_SER_PORT, QAbstractSocket::ShareAddress))
        return -1;

    /*
     * We will poll for incoming data the same way we do for the serial driver.
     * If we go event driven, then we need to connect readyRead();
     * connect(&socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
     */

    isopen = true;
    return 0;
}

bool XBeeSerialPort::isOpen()
{
    bool rc = isopen;
    return rc;
}

bool XBeeSerialPort::isSequential() const
{
    return true;
}

void XBeeSerialPort::close()
{
    if (isopen || socket.isOpen()) {
        socket.close();
    }
    isopen = false;
}

void XBeeSerialPort::flush()
{
    char buf[BUFSIZ];
    int rc = 0;
    if (socket.state() != QUdpSocket::BoundState) return;
    while (socket.hasPendingDatagrams()) {
        rc = socket.pendingDatagramSize();
        rc = (rc < BUFSIZ) ? rc : BUFSIZ-1;
        socket.readDatagram(buf, rc);
    }
}

/*
 * not used
 */
void XBeeSerialPort::readyRead()
{
    char buf[BUFSIZ];
    if (socket.state() != QUdpSocket::BoundState) return;
    int rc = socket.pendingDatagramSize();
    rc = (rc < BUFSIZ) ? rc : BUFSIZ-1;
    socket.readDatagram(buf, rc);
    buf[rc] = 0;
    rxdata += QByteArray(buf);
}

qint64 XBeeSerialPort::bytesAvailable() const
{
    if (socket.state() != QUdpSocket::BoundState) return 0;
    return socket.hasPendingDatagrams() ? 1 : 0;
}

QByteArray XBeeSerialPort::readAll()
{
    QByteArray ba;
    char buf[BUFSIZ];

    while (socket.state() == QUdpSocket::BoundState && socket.hasPendingDatagrams()) {
        int len = socket.pendingDatagramSize();
        int size = len < BUFSIZ ? len : BUFSIZ-1;
        socket.readDatagram(buf, size);
        ba += QByteArray(buf, size);
    }
    return ba;
}

qint64 XBeeSerialPort::write(QByteArray barry, int len)
{
    qint64 rc = 0;
    quint16 serport = XBEE_SER_PORT;
    rc = socket.writeDatagram(barry.constData(), (qint64)len, QHostAddress(remoteaddr), serport);
    return rc;
}

void XBeeSerialPort::setBaudRate(qint64 baudrate)
{
    baud = baudrate;
}

qint64 XBeeSerialPort::baudRate() const
{
    return baud;
}

void XBeeSerialPort::setPortName(QString name)
{
    port = name;
}

QString XBeeSerialPort::portName() const
{
    return port;
}

/*
 * functions taken from David's file
 */
int XBeeSerialPort::setItem(xbCommand cmd, const QString value)
{
    const int MYBUFSIZE = 2048;
    uint8_t txBuf[MYBUFSIZE];
    uint8_t rxBuf[MYBUFSIZE];
    txPacket *tx = (txPacket *)txBuf;
    rxPacket *rx = (rxPacket *)rxBuf;
    int cnt;

    for (int i = 0; i < value.size(); ++i)
        txBuf[sizeof(txPacket) + i] = value[i].toLatin1();

    if ((cnt = sendRemoteCommand(cmd, tx, sizeof(txPacket) + value.size(), rx, sizeof(rxBuf))) == -1)
        return -1;

    return 0;
}

int XBeeSerialPort::setItem(xbCommand cmd, int value)
{
    const int MYBUFSIZE = 2048;
    uint8_t txBuf[MYBUFSIZE];
    uint8_t rxBuf[MYBUFSIZE];
    txPacket *tx = (txPacket *)txBuf;
    rxPacket *rx = (rxPacket *)rxBuf;
    int cnt, i;

    for (i = 0; i < (int)sizeof(int); ++i)
        txBuf[sizeof(txPacket) + i] = value >> ((sizeof(int) - i - 1) * 8);

    if ((cnt = sendRemoteCommand(cmd, tx, sizeof(txPacket) + sizeof(int), rx, sizeof(rxBuf))) == -1)
        return -1;

    return 0;
}

int XBeeSerialPort::sendRemoteCommand(xbCommand cmd, txPacket *tx, int txCnt, rxPacket *rx, int rxSize)
{
    int retries, timeout, cnt;

    QUdpSocket sock;
    QHostAddress rhaddr(remoteaddr);

    if(!sock.bind(XBEE_APP_PORT, QAbstractSocket::ShareAddress))
        return -1;

    tx->hdr.packetID = 0;
    tx->hdr.encryptionPad = 0;
    tx->hdr.commandID = 0x02;
    tx->hdr.commandOptions = 0x00;
    tx->frameID = 0x01;
    tx->configOptions = 0x02;
    strncpy(tx->atCommand, atCmd[cmd], 2);

    retries = 3;
    timeout = 200;
    while (--retries >= 0) {
        qint16 number = rand();

        tx->hdr.number1 = number;
        tx->hdr.number2 = tx->hdr.number1 ^ 0x4242;

        if (sock.writeDatagram((char *)tx, txCnt, rhaddr, XBEE_APP_PORT) != txCnt) {
            sock.close();
            return -1;
        }

        if (sock.waitForReadyRead(timeout)) {
            if ((cnt = sock.readDatagram((char *)rx, rxSize)) >= (int)sizeof(rxPacket)
            &&  (rx->hdr.number1 ^ rx->hdr.number2) == 0x4242
            &&  rx->hdr.number1 == number
            &&  rx->status == 0x00) {
                sock.close();
                return cnt;
            }
        }
    }

    sock.close();

    return -1;
}

