
#include "PortListener.h"
#include <QtDebug>


enum { READSIZE = 512 };

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
    connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead())); // never disconnect
    connect(port, SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)));
    buffer.resize(READSIZE+1);
}

void PortListener::setDtr(bool enable)
{
    this->port->setDtr(enable);
}

void PortListener::open()
{
    if(!textEditor) // no text editor, no open
        return;

    if(port == NULL)
        return;

    port->open(QIODevice::ReadWrite);
    onReadyRead(); // see what's in the buffer?
}

void PortListener::close()
{
    if(port == NULL)
        return;

    if(port->isOpen()) {
        port->close();
    }
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
    QString text;
    int ret;
    int len;
    len = port->bytesAvailable();
    if(len == 0)
        return;

    buffer = port->read(READSIZE);
    ret = buffer.length();
    if(ret == 0)
        return;

    for(int n = 0; n < ret; n++)
    {
        switch(buffer[n])
        {
            case 0:
                break;
            case '\b':
                text = textEditor->toPlainText();
                textEditor->setPlainText(text.mid(0,text.length()-1));
                n+=2;
                break;
            case 0xA:
                textEditor->insertPlainText(QString(buffer.at(n)));
                break;
            case 0xD:
                break;
            default:
                textEditor->insertPlainText(QString(buffer.at(n)));
                break;
        }
    }
    textEditor->moveCursor(QTextCursor::End);

#if 0
    if(buffer.indexOf('\b') < 0)
    { // no backspace
        for(int n = 0; n < ret; n++) {
            switch(buffer[n]) {
            case 0:
                break;
            case 0xA:
                textEditor->insertPlainText(QString(buffer.at(n)));
                break;
            case 0xD:
                break;
            default:
                textEditor->insertPlainText(QString(buffer.at(n)));
                break;
            }
        }
    }
    else // handle backspaces.
    {
        for(int n = 0; n < ret; n++)
        {
            QString text = textEditor->toPlainText();
            int tlen = text.length();
            if(buffer[n] == '\b') {
                textEditor->setPlainText(text.mid(0,tlen-1));
            } else {
                textEditor->setPlainText(text + buffer[n]);
            }
        }
    }
    textEditor->moveCursor(QTextCursor::End);
#endif
}

void PortListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}
