
#include "console.h"
#include "PortListener.h"
#include <QtDebug>

/*
 * We use Polling for the port because events are not
 * well behaved in the QextSerialPort library on windows.
 */
PortListener::PortListener(QObject *parent, Console *term) : QThread(parent)
{
    terminal = term;
#if defined(EVENT_DRIVEN)
    port = new QextSerialPort(QextSerialPort::EventDriven);
    connect(port, SIGNAL(readyRead()), this, SLOT(updateReady()));
#else
    port = new QextSerialPort(QextSerialPort::Polling);
    connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
#endif
}

void PortListener::init(const QString & portName, BaudRateType baud)
{
    port->setPortName(portName);
    port->setBaudRate(baud);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    port->setTimeout(10);
}

QString PortListener::getPortName()
{
    return port->portName();
}

BaudRateType PortListener::getBaudRate()
{
    return port->baudRate();
}

void PortListener::setDtr(bool enable)
{
    this->port->setDtr(enable);
}

void PortListener::setRts(bool enable)
{
    this->port->setRts(enable);
}

bool PortListener::open()
{
    if(!textEditor) // no text editor, no open
        return false;

    if(terminal == NULL)
        return false;

    if(port == NULL)
        return false;

    if(port->isOpen() == true)
        return false;

    port->open(QIODevice::ReadWrite);

#if !defined(EVENT_DRIVEN)
    connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
    this->start();
#else
    connect(port, SIGNAL(readyRead()), this, SLOT(updateReady()));
#endif
    return true;
}

void PortListener::close()
{
    if(port == NULL)
        return;

#if !defined(EVENT_DRIVEN)
    disconnect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
#else
    disconnect(port, SIGNAL(readyRead()), this, SLOT(updateReady()));
#endif

    port->close();
}

bool PortListener::isOpen()
{
    return port->isOpen();
}

void PortListener::setTerminalWindow(QPlainTextEdit *editor)
{
    textEditor = editor;
}

void PortListener::send(QByteArray &data)
{
    port->write(data.constData(),1);
}

void PortListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}

void PortListener::updateReady()
{
    if(terminal != NULL)
        if(terminal->enabled())
            terminal->updateReady(port);
}

void PortListener::updateReady(QextSerialPort* port)
{
    if(terminal != NULL)
        if(terminal->enabled())
            terminal->updateReady(port);
}

#if defined(Q_OS_WIN32)
// delay less than 25ms here is dangerous for windows
#define POLL_DELAY 25
#else
// delay for MAC/Linux
#define POLL_DELAY 10
#endif

/*
 * This is the port listener thread.
 * We have to use polling so that we can share the loader port.
 */
void PortListener::run()
{
    while(port->isOpen()) {
        msleep(POLL_DELAY);
        QApplication::processEvents();
        if(terminal->enabled())
            emit updateEvent(port);
    }
}
