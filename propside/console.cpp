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
// odd that on linux BUFFERSIZE > about 16 and lines get recieved but are not printed on the console.
enum { BUFFERSIZE = 16 };
#else
enum { BUFFERSIZE = 64 };
#endif

void Console::updateReady(QextSerialPort* port)
{
    /* Using QString buffer = port.readAll() ... doesn't work.
     * Clear screen 0's get lost. Use char buffer instead.
     */
    char buffer[BUFFERSIZE+1];
    int length = port->bytesAvailable();
    length = (length > BUFFERSIZE) ? BUFFERSIZE : length;
    port->readData(buffer, length);

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

    static bool utfparse = false;
    static int utfbytes = 0;
    static int utf8 = 0;

    for(int n = 0; n < length; n++)
    {
        char ch = buffer[n];

        //qDebug(QString(" %1 %2").arg(ch, 2, 16, QChar('0')).arg(QChar(ch)).toAscii());
        //insertPlainText(QString(" %1 ").arg(ch, 2, 16, QChar('0')));
        //insertPlainText(QChar(ch));

        if (ch & 0x80) {    //UTF-8 parsing and handling
            if (utfparse == true) {

                utf8 <<= 6;
                utf8 |= (ch & 0x3F);

                utfbytes--;

                if (utfbytes == 0) {
                    utfparse = false;
                    cur.insertText(QChar(utf8));
                }
            } else {
                utfparse = true;
                utf8 = 0;

                while (ch & 0x80) {
                    ch <<= 1;
                    utfbytes++;
                }

                ch >>= utfbytes;

                utf8 = (int)ch;

                utfbytes--;
            }
            continue;
        }

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
                moveCursor(QTextCursor::End);
                break;
            }
            case '\r': {
                char nc = buffer[n+1];
                if(n >= length-1) {
                    length = port->bytesAvailable();
                    length = (length > BUFFERSIZE) ? BUFFERSIZE : length;
                    port->readData(buffer, length);
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
                moveCursor(QTextCursor::End);
                break;
            }
        }
    }
}
