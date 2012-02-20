
#include "PortListener.h"
#include <QtDebug>


enum { READSIZE = 64 };

PortListener::PortListener()
{
    port = NULL;
}

void PortListener::init(const QString & portName, BaudRateType baud)
{
    if(port != NULL) {
         // don't reinitialize port
        if(port->portName() == portName)
            return;
        delete port;
    }
    this->port = new QextSerialPort(portName, QextSerialPort::EventDriven);
    port->setBaudRate(baud);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(port, SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)));
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

void PortListener::onReadyRead()
{
    char buff[READSIZE+1];
    int ret = 0;

    if(port->isOpen() == false)
        return;

    ret = port->readData(buff, READSIZE);
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
}

void PortListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}
