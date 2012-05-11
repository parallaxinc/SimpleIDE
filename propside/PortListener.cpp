
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
#if defined(Q_WS_WIN32)
    port->setPortName(port->fullPortNameWin(portName));
#else
    port->setPortName(portName);
#endif
    port->setBaudRate(baud);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    port->setTimeout(10);
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
    this->start();
#endif
    return true;
}

void PortListener::close()
{
    if(port == NULL)
        return;

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
    port->writeData(data.constData(),1);
}
#if 0
int PortListener::readData(char *buffer, int length)
{
    int ret = port->readData(buffer,length);
    return ret;
}
#endif

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
        terminal->updateReady(port);
}

void PortListener::updateReady(QextSerialPort* port)
{
    if(terminal != NULL)
        terminal->updateReady(port);
}

/*
 * This is the port listener thread.
 * We have to use polling - see constructor.
 */
void PortListener::run()
{
    while(port->isOpen()) {
        msleep(25); // less than 25ms here is dangerous for windows
        QApplication::processEvents();
        emit updateEvent(port);
    }
}
