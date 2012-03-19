#include "console.h"
#include "mainwindow.h"
#include "terminal.h"

Console::Console(QWidget *parent) : QPlainTextEdit(parent)
{
    setFont(QFont("courier"));
    isEnabled = true;
}

void Console::setPortEnable(bool value)
{
    isEnabled = value;
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
enum { BUFFERSIZE = 16 };
#else
enum { BUFFERSIZE = 32 };
#endif

void Console::updateReady(QextSerialPort* port)
{
    char buffer[BUFFERSIZE+1];
    int length = port->bytesAvailable();
    if(length > BUFFERSIZE) length = BUFFERSIZE;
    length = port->readData(buffer, length);

    if(length < 1)
        return;

    QTextCursor cur = this->textCursor();
    if(isEnabled) {
        // always start at the end just in case someone clicked the window
        moveCursor(QTextCursor::End);
        for(int n = 0; n < length; n++)
        {
            char ch = buffer[n];
            switch(ch)
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
                case 0xD: {
                    cur.insertText(QString(ch));
                    break;
                }
                case 0xA: {
                        cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                    break;
                }
                default: {
                    cur.insertText(QString(ch));
                    break;
                }
            }
        }
        moveCursor(QTextCursor::End);
    }
    //updateReady(buffer, length);
}

void Console::updateReady(char *buff, int length)
{
    // always start at the end just in case someone clicked the window
    moveCursor(QTextCursor::End);
    for(int n = 0; n < length; n++)
    {
        char ch = buff[n];
        switch(ch)
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
                insertPlainText(QString(ch));
                break;
            }
            case 0xD: {
                break;
            }
            default: {
                insertPlainText(QString(ch));
                break;
            }
        }
    }
    moveCursor(QTextCursor::End);
}
