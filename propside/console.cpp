#ifdef SPINSIDE
#include "mainspinwindow.h"
#define MAINWINDOW MainSpinWindow
#else
#include "mainwindow"
#define MAINWINDOW MainWindow
#endif

#include "terminal.h"
#include "console.h"

Console::Console(QWidget *parent) : QPlainTextEdit(parent)
{
    setFont(QFont("courier"));
    isEnabled = true;
}

void Console::setPortEnable(bool value)
{
    utfparse = false;
    utfbytes = 0;
    utf8 = 0;

    isEnabled = value;
}

bool Console::enabled()
{
    return isEnabled;
}

void Console::clear()
{
    utfparse = false;
    utfbytes = 0;
    utf8 = 0;

    setPlainText("");
}

void Console::keyPressEvent(QKeyEvent *event)
{
    // qDebug() << "keyPressEvent";
    MAINWINDOW *parentMain = (MAINWINDOW *)this->parentWidget()->parentWidget();

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

#ifdef EVENT_DRIVEN
enum { BUFFERSIZE = 2048 };
#else
#ifdef Q_WS_WIN32
enum { BUFFERSIZE = 32 };
#else
enum { BUFFERSIZE = 64 };
#endif
#endif

#if 0
void Console::updateReady(QextSerialPort* port)
{
    if(isEnabled == false)
        return;

    if(port->bytesAvailable() < 1)
        return;

    QByteArray ba = port->read(BUFFERSIZE);
    int length = ba.length();

    if(length < 1)
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
        char ch = ba[n];

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
                char nc = ba[n+1];
                if(n >= length-1) {
                    ba = port->read(BUFFERSIZE);
                    length = ba.length();
                    n = 0;
                    nc = ba[n];
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
#else

void Console::updateReady(QextSerialPort* port)
{
    if(isEnabled == false)
        return;

    if(port->bytesAvailable() < 1)
        return;

    QByteArray ba = port->read(BUFFERSIZE);
    int length = ba.length();

    if(length < 1)
        return;

    for(int n = 0; n < length; n++)
        update(ba.at(n));
}
#endif


void Console::update(char ch)
{
    QString text = "";
    QTextCursor cur = this->textCursor();

    if(cur.block().length() > 120)
        cur.insertBlock();

    // always start at the end just in case someone clicked the window
    moveCursor(QTextCursor::End);

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
        //continue;
        return;
    }

    switch(pcmd)
    {
        case PCMD_CURPOS_X: {
                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,ch);
                setTextCursor(cur);
                pcmd = PCMD_NONE;
            }
            break;

        case PCMD_CURPOS_Y: {
                cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,ch);
                setTextCursor(cur);
                pcmd = PCMD_NONE;
            }
            break;

        case PCMD_CURPOS_XY: {
                if(pcmdlen == 2) {
                    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                    cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,ch);
                    setTextCursor(cur);
                }
                else if(pcmdlen == 1) {
                    cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                    cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,ch);
                    setTextCursor(cur);
                }
                pcmdlen--;
                if(pcmdlen < 1)
                    pcmd = PCMD_NONE;
            }
            break;

        default: {

            switch(ch)
            {
            case EN_ClearScreen:
            case EN_ClearScreen2: {
                    if(this->enableClearScreen) {
                        setPlainText("");
                    }
                }
                break;

            case EN_HomeCursor: {
                    if(this->enableHomeCursor) {
                        cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_PosXYCursor: {
                    if(this->enablePosXYCursor) {
                        pcmd = PCMD_CURPOS_XY;
                        pcmdlen  = 2;
                    }
                }
                break;

            case EN_MoveCursorLeft: {
                    if(this->enableMoveCursorLeft) {
                        cur.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor);
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_MoveCursorRight: {
                    if(this->enableMoveCursorRight) {
                        cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor);
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_MoveCursorUp: {
                    if(this->enableMoveCursorUp) {
                        cur.movePosition(QTextCursor::Up,QTextCursor::MoveAnchor);
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_MoveCursorDown: {
                    if(this->enableMoveCursorDown) {
                        cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor);
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_BeepSpeaker: {
                    if(this->enableBeepSpeaker) {
                        QApplication::beep();
                    }
                }
                break;

            case EN_Backspace: {
                    if(this->enableBackspace) {
                        text = toPlainText();
                        setPlainText(text.mid(0,text.length()-1));
                        moveCursor(QTextCursor::End);
                    }
                }
                break;

            case EN_Tab: {
                    if(this->enableTab) {
                        int column = cur.columnNumber() % 8;
                        while(column++ < 8)
                            cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor);
                        setTextCursor(cur);
                    }
                }
                break;

            /* fall through case because newline and creturn can change roles
             * the switch is just to speed things up
             */
            case EN_NewLine: // fall through
            case EN_CReturn: {
                    if(ch == newline) {
                        if(enableNewLine) {
                            cur.insertBlock();
                            setTextCursor(cur);
                        }
                    }
                    else if(ch == creturn) {
                        if(enableCReturn) {
                            text = toPlainText();
                            cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                            setTextCursor(cur);
                        }
                    }
                    lastchar = ch; // later check for appendCR and add it
                }
                break;

            case EN_ClearToEOL: {
                    if(this->enableClearToEOL) {
                        cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
                        cur.removeSelectedText();
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_ClearLinesBelow: {
                    if(this->enableClearLinesBelow) {
                        int row = cur.blockNumber();
                        int col = cur.columnNumber();
                        cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                        cur.movePosition(QTextCursor::Down,QTextCursor::KeepAnchor,row);
                        cur.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,col);
                        QString s = cur.selectedText();
                        this->setPlainText(s);
                    }
                }
                break;

            default: {
                    cur.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
                    cur.removeSelectedText();
                    cur.insertText(QString(ch));
                    setTextCursor(cur);
                }
                break;
            }
            break;

        } // end pcmd default
    } // end pcmd switch
}
