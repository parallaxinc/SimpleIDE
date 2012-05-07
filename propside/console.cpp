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

#ifdef Q_WS_MAC

void Console::updateReady(QextSerialPort* port)
{
    QString buffer = port->readAll();
    int length = buffer.length();

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
    //qDebug() << QByteArray(buffer,length);

    for(int n = 0; n < length; n++)
    {
        char ch = buffer[n].toAscii();
        //qDebug(QString(" %1 %2").arg(ch, 2, 16, QChar('0')).arg(QChar(ch)).toAscii());
        //insertPlainText(QString(" %1 ").arg(ch, 2, 16, QChar('0')));
        //insertPlainText(QChar(ch));

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
                //cur.insertBlock();
                break;
            }
            case '\r': {
                char nc = buffer[n+1].toAscii();
                if(n >= length-1) {
                    length = port->bytesAvailable();
                    buffer = port->readAll();
                    length = buffer.length();
                    n = 0;
                    nc = buffer[n].toAscii();
                    /* for loop incrs back to 0 for next round
                     * we need to process nc == '\n' and other chars there
                     */
                    n--;
                }
                if(nc != '\n') {
                    text = toPlainText();
                    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
                    cur.removeSelectedText();
                }
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

#else


#if defined(EVENT_DRIVEN)
// odd that on linux BUFFERSIZE > about 16 and lines get recieved but are not printed on the console.
enum { BUFFERSIZE = 16 };
#else
enum { BUFFERSIZE = 32 };
#endif

void Console::updateReady(QextSerialPort* port)
{
    // improve performance a little
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
    //qDebug() << QByteArray(buffer,length);

    for(int n = 0; n < length; n++)
    {
        char ch = buffer[n];
        //qDebug(QString(" %1 %2").arg(ch, 2, 16, QChar('0')).arg(QChar(ch)).toAscii());
        //insertPlainText(QString(" %1 ").arg(ch, 2, 16, QChar('0')));
        //insertPlainText(QChar(ch));

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
                //cur.insertBlock();
                break;
            }
            case '\r': {
                char nc = buffer[n+1];
                if(n >= length-1) {
                    length = port->bytesAvailable();
                    if(length > BUFFERSIZE) length = BUFFERSIZE;
                    length = port->read(buffer, length);
                    n = 0;
                    nc = buffer[n];
                    /* for loop incrs back to 0 for next round
                     * we need to process nc == '\n' and other chars there
                     */
                    n--;
                }
                if(nc != '\n') {
                    text = toPlainText();
                    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
                    cur.removeSelectedText();
                }
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

#endif
