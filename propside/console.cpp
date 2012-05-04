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
    else
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
enum { BUFFERSIZE = 64 };
#endif

void Console::updateReady(QextSerialPort* port)
{
    char buffer[BUFFERSIZE+1];
    int length = port->bytesAvailable();
    if(length > BUFFERSIZE) length = BUFFERSIZE;
    length = port->read(buffer, length);

    if(length < 1)
        return;

    if(isEnabled == false)
        return;

    QString text = "";

    QTextCursor cur = this->textCursor();
    if(cur.block().length() > 200)
        cur.insertBlock();
    // always start at the end just in case someone clicked the window
    moveCursor(QTextCursor::End);
    for(int n = 0; n < length; n++)
    {
        char ch = buffer[n];
        //insertPlainText(QString(" %1").arg(ch, 2, 16, QChar('0')));
        switch(ch)
        {
            case 0: {
                setPlainText("");
                moveCursor(QTextCursor::End);
                break;
            }
            case '\b': {
                text = toPlainText();
                setPlainText(text.mid(0,text.length()-1));
                moveCursor(QTextCursor::End);
                break;
            }
            case '\n': {
                cur.insertText(QString(ch));
                break;
            }
            case '\r': {
                char nc;
                text = toPlainText();

                /* handle corner cases for terminal because \r can come after \n
                 */
                if(n+1 < length) {
                    nc = buffer[n+1];
                }
                else if(n+1 <= BUFFERSIZE) {
                    char bufft[1];
                    if(port->bytesAvailable() > 0) {
                        if(port->read(bufft,1) > -1) {
                            buffer[n+1] = bufft[0];
                            nc = buffer[n+1];
                            length++;
                        }
                    }
                }
                else {
                    length = port->bytesAvailable();
                    if(length > BUFFERSIZE/4) length = BUFFERSIZE/4;
                    length = port->read(buffer, length);
                    n = 0;
                    nc = buffer[n];
                    // for loop incrs back to 0 for next round
                    // we need to process nc == '\n' and other chars there
                    n--;
                }

                if(nc == '\n')
                    continue;

                int tlen = text.length();
                int tcol = cur.block().length()-1;

                tlen -= tcol;

                // should never be < 0
                if(tlen > -1) {
                    text = text.mid(0,tlen);
                    setPlainText(text);
                    moveCursor(QTextCursor::End);
                }

                break;
            }
            default: {
                cur.insertText(QString(ch));
                break;
            }
        }
    }
}
