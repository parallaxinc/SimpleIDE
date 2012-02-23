#include "console.h"
#include "mainwindow.h"
#include "terminal.h"

Console::Console(QWidget *parent) : QPlainTextEdit(parent)
{
    setFont(QFont("courier"));
}

void Console::keyPressEvent(QKeyEvent *event)
{
    // qDebug() << "keyPressEvent";
    MainWindow *parentMain = (MainWindow *)this->parentWidget()->parentWidget();

    Terminal *term = (Terminal*) this->parentWidget();
    if(event->matches((QKeySequence::Copy))) {
        term->copyFromFile();
    }
    if(event->matches((QKeySequence::Paste))) {
        QClipboard *clip = QApplication::clipboard();
        parentMain->sendPortMessage(clip->text());
    }
    else {
        parentMain->keyHandler(event);
    }
}

#if defined(EVENT_DRIVEN)
enum { BUFFERSIZE = 128 };
#else
enum { BUFFERSIZE = 32 };
#endif

void Console::updateReady(QextSerialPort* port)
{
    char buffer[BUFFERSIZE+1];
    int length = port->bytesAvailable();
    if(length > BUFFERSIZE) length = BUFFERSIZE;
    length = port->readData(buffer, length);
    updateReady(buffer, length);
}

void Console::updateReady(char *buff, int length)
{
    for(int n = 0; n < length; n++)
    {
        switch(buff[n])
        {
            case 0: {
                break;
            }
            case '\b': {
                QString text;
                text = toPlainText();
                setPlainText(text.mid(0,text.length()-1));
                n+=2;
                break;
            }
            case 0xA: {
                insertPlainText(QString(buff[n]));
                break;
            }
            case 0xD: {
                break;
            }
            default: {
                insertPlainText(QString(buff[n]));
                break;
            }
        }
    }
    moveCursor(QTextCursor::End);
}
