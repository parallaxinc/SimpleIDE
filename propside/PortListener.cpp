
#include "PortListener.h"
#include <QtDebug>

/*
 * if READSIZE is much bigger than this, the screen is not very responsive.
 */
enum { READSIZE = 32 };

/*
 * We use Polling for the port because events are not
 * well behaved in the QextSerialPort library on windows.
 */
PortListener::PortListener(QObject *parent) : QThread(parent)
{
    port = new QextSerialPort(QextSerialPort::Polling);
    connect(this, SIGNAL(readyRead(int)), this, SLOT(onReadyRead(int)));
    isEnabled = true;
}

void PortListener::init(const QString & portName, BaudRateType baud)
{
    port->setPortName(portName);
    port->setBaudRate(baud);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
}

void PortListener::setDtr(bool enable)
{
    this->port->setDtr(enable);
}

bool PortListener::open()
{
    if(!textEditor) // no text editor, no open
        return false;

    if(port == NULL)
        return false;

    if(port->isOpen() == true)
        return false;

    port->open(QIODevice::ReadWrite);
    this->start();
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

void PortListener::onReadyRead(int length)
{
    char buff[READSIZE+1];
    int ret;
    if(length > READSIZE)
        length = READSIZE;

    ret = port->readData(buff, length);
    for(int n = 0; n < ret; n++)
    {
        switch(buff[n])
        {
            case 0: {
                break;
            }
            case '\b': {
                QString text;
                text = textEditor->toPlainText();
                textEditor->setPlainText(text.mid(0,text.length()-1));
                n+=2;
                break;
            }
            case 0xA: {
                textEditor->insertPlainText(QString(buff[n]));
                break;
            }
            case 0xD: {
                break;
            }
            default: {
                textEditor->insertPlainText(QString(buff[n]));
                break;
            }
        }
    }
    textEditor->moveCursor(QTextCursor::End);
    this->yieldCurrentThread();
}

void PortListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}

/*
 * This is the port listener thread.
 * We have to use polling - see constructor.
 */
void PortListener::run()
{
    int len = 0;
    while(port->isOpen()) {
        QApplication::processEvents();
        msleep(20);
        if(isEnabled == false)
            continue;
        len = port->bytesAvailable();
        if(len > 0) {
            emit readyRead(len);
        }
    }
}

void PortListener::enable(bool value)
{
    isEnabled = value;
}
