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
    pcmd = Console::PCMD_NONE;
    pcmdx = 0;
    pcmdy = 0;
    maxcol = 32;
    wrapMode = 0;
    tabsize = 8;
    hexmode = false;
    hexbytes = 0;
    maxhex = 16;
    for(int n = 0; n < maxhex; n++)
        hexbyte[n] = 0;
}

void Console::setPortEnable(bool value)
{
    utfparse = false;
    utfbytes = 0;
    utf8 = 0;
    hexbytes = 0;
    for(int n = 0; n < maxhex; n++)
        hexbyte[n] = 0;
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
    hexbytes = 0;
    for(int n = 0; n < maxhex; n++)
        hexbyte[n] = 0;
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

void Console::resizeEvent(QResizeEvent *e)
{
    QFontMetrics fm(this->font());
    maxcol = this->width()/8;
    if(fm.width("X") > 0) {
        maxcol = width()/fm.width("X")-3;
    }

    //qDebug() << maxcol << width() << fm.width("X");
    QPlainTextEdit::resizeEvent(e);
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

    if(hexmode != false) {
        for(int n = 0; n < length; n++)
            dumphex((int)ba.at(n));
        return;
    }
    for(int n = 0; n < length; n++)
        update(ba.at(n));
}

void Console::dumphex(int ch)
{
    QTextCursor cur = this->textCursor();
    // always start at the end just in case someone clicked the window
    moveCursor(QTextCursor::End);

    if(hexdump != true) {
        if(wrapMode > 0) {
            if(cur.block().length() > wrapMode)
                cur.insertBlock();
        }
        else if(cur.block().length() > maxcol-2) {
            cur.insertBlock();
        }

        cur.insertText(QString(" %1").arg(ch,2,16,QChar('0')));
    }
    else {
        int byte = hexbytes % maxhex;
        if(byte == 0) {
            cur.insertText(QString("  "));
            for(int n = 0; n < maxhex; n++) {
                if(isprint(hexbyte[n]))
                    cur.insertText(QString(hexbyte[n]));
                else
                    cur.insertText(QString("."));
            }
            cur.insertBlock();
        }
        hexbyte[byte] = ch;
        hexbytes++;

        cur.insertText(QString(" %1").arg(ch,2,16,QChar('0')));
    }
}

void Console::update(char ch)
{
    QString text = "";
    QTextCursor cur = this->textCursor();
    this->setWordWrapMode(QTextOption::WrapAnywhere);

    if(wrapMode > 0) {
        if(cur.block().length() > wrapMode)
            cur.insertBlock();
    }
    else if(cur.block().length() > maxcol) {
        cur.insertBlock();
    }

    // always start at the end just in case someone clicked the window
    //moveCursor(QTextCursor::End);
    // now that we have cursor positioning we can't always start at the end.

    //qDebug(QString(" %1 %2").arg(ch, 2, 16, QChar('0')).arg(QChar(ch)).toAscii());
    //insertPlainText(QString(" %1 ").arg(ch, 2, 16, QChar('0')));
    //insertPlainText(QChar(ch));

    qDebug() << QString("%1 %2,%3 %4,%5").arg(pcmd,2,10,QChar('0')).arg(cur.columnNumber(),2,10,QChar('0')).arg(cur.blockNumber(),2,10,QChar('0')).arg(ch,2,10,QChar('0')).arg(ch,1,QChar('0'));
    switch(pcmd)
    {
        case PCMD_CURPOS_X: {
                pcmdx = ch;
                cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,pcmdy);
                setTextCursor(cur);

                int j = cur.block().length();
                if(j < ch) {
                    for(; j <= ch; j++) {
                        cur.movePosition(QTextCursor::EndOfBlock,QTextCursor::MoveAnchor);
                        cur.insertText(" ");
                    }
                }
                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,ch);
                setTextCursor(cur);
                pcmd = PCMD_NONE;
            }
            break;

        case PCMD_CURPOS_Y: {
                pcmdy = ch;
                int j = this->blockCount();
                if(this->blockCount() < ch) {
                    for(; j <= ch; j++) {
                        cur.movePosition(QTextCursor::End);
                        cur.insertBlock(cur.blockFormat(),cur.charFormat());
                    }
                }
                cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,ch);
                setTextCursor(cur);

                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pcmdx);
                setTextCursor(cur);
                pcmd = PCMD_NONE;
            }
            break;

        case PCMD_CURPOS_XY: {
                if(pcmdlen == 2) {
                    pcmdx = ch;
                }
                else if(pcmdlen == 1) {
                    pcmdy = ch;
                    int j = this->blockCount();
                    if(this->blockCount() < ch) {
                        for(; j < ch; j++) {
                            cur.movePosition(QTextCursor::End);
                            cur.insertBlock(cur.blockFormat(),cur.charFormat());
                        }
                    }
                    cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                    cur.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, ch);
                    setTextCursor(cur);

                    j = cur.block().length();
                    if(j < pcmdx) {
                        for(; j < pcmdx; j++) {
                            cur.movePosition(QTextCursor::EndOfBlock,QTextCursor::MoveAnchor);
                            cur.insertText(" ");
                        }
                    }
                    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                    cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,ch);
                    setTextCursor(cur);
                }
                pcmdlen--;
                if(pcmdlen < 1)
                    pcmd = PCMD_NONE;
            }
            break;

        default: {

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

            case EN_PosCursorX: {
                    if(this->enablePosCursorX) {
                        pcmd = PCMD_CURPOS_X;
                        pcmdlen  = 1;
                    }
                }
                break;

            case EN_PosCursorY: {
                    if(this->enablePosCursorY) {
                        pcmd = PCMD_CURPOS_Y;
                        pcmdlen  = 1;
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
                        int column = cur.columnNumber() % tabsize;
                        while(column++ < tabsize) {
                            moveCursor(QTextCursor::End);
                            cur.insertText(" ");
                        }
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
                            int col = cur.columnNumber();
                            cur.insertBlock();
                            if(this->enableAddCRtoNL == false) {
                                while(col-- > -1) {
                                    cur.insertText(" ");
                                    cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor, 1);
                                }
                            }
                            setTextCursor(cur);
                        }
                    }
                    else if(ch == creturn) {
                        if(enableCReturn) {
                            text = toPlainText();
                            cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                            setTextCursor(cur);
#if 0
                            if(this->enableAddNLtoCR) {
                                cur.insertBlock();
                                setTextCursor(cur);
                            }
                            else {
                                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                                setTextCursor(cur);
                            }
#endif
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

            pcmdx = cur.columnNumber();
            pcmdy = cur.blockNumber();

        } // end pcmd default
    } // end pcmd switch
}
