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
    sbuff = NULL;
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

int  Console::eventKey(QKeyEvent* event)
{
    int key = event->key();
    switch(key)
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        key = getEnter();
        break;
    case Qt::Key_Backspace:
        key = '\b';
        break;
    case Qt::Key_Alt:
        return 0;
    case Qt::Key_Control:
        return 0;
    case Qt::Key_Shift:
        return 0;
    default:
        if(QApplication::keyboardModifiers() & Qt::CTRL) {
            key &= ~0xe0;
        }
        else {
            if(event->text().length() > 0) {
                QChar c = event->text().at(0);
                key = (int)c.toAscii();
            }
        }
        break;
    }
    return key;
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
        int key = eventKey(event);
        if(key < 1)
            return;
        if(this->enableEchoOn) {
            if(isascii(key) != 0)
                this->insertPlainText(QChar(key));
        }
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

void Console::setEnableClearScreen(bool value)
{
     enableClearScreen = value;
}
void Console::setEnableHomeCursor(bool value)
{
     enableHomeCursor = value;
}
void Console::setEnablePosXYCursor(bool value)
{
     enablePosXYCursor = value;
}
void Console::setEnableMoveCursorLeft(bool value)
{
     enableMoveCursorLeft = value;
}
void Console::setEnableMoveCursorRight(bool value)
{
     enableMoveCursorRight = value;
}
void Console::setEnableMoveCursorUp(bool value)
{
     enableMoveCursorUp = value;
}
void Console::setEnableMoveCursorDown(bool value)
{
     enableMoveCursorDown = value;
}
void Console::setEnableBeepSpeaker(bool value)
{
     enableBeepSpeaker = value;
}
void Console::setEnableBackspace(bool value)
{
     enableBackspace = value;
}
void Console::setEnableTab(bool value)
{
     enableTab = value;
}
void Console::setEnableCReturn(bool value)
{
     enableCReturn = value;
}
void Console::setEnableClearToEOL(bool value)
{
     enableClearToEOL = value;
}
void Console::setEnableClearLinesBelow(bool value)
{
     enableClearLinesBelow = value;
}
void Console::setEnableNewLine(bool value)
{
     enableNewLine = value;
}
void Console::setEnablePosCursorX(bool value)
{
     enablePosCursorX = value;
}
void Console::setEnablePosCursorY(bool value)
{
     enablePosCursorY = value;
}
void Console::setEnableClearScreen16(bool value)
{
     enableClearScreen16 = value;
}
void Console::setEnableEchoOn(bool value)
{
     enableEchoOn = value;
}
void Console::setEnableEnterIsNL(bool value)
{
     enableEnterIsNL = value;
}


void Console::setEnableSwapNLCR(bool value)
{
     enableSwapNLCR = value;
     if(enableSwapNLCR) {
         newline = 10;
         creturn = 13;
     }
     else {
         newline = 13;
         creturn = 10;
     }
}

int Console::getEnter()
{
    if(enableEnterIsNL)
        return newline;
    return creturn;
}

void Console::setWrapMode(int mode)
{
    wrapMode = mode;
    if(mode == 0) {
        this->setWordWrapMode(QTextOption::WordWrap);
    }
    else {
        this->setWordWrapMode(QTextOption::WrapAnywhere);
    }
}

void Console::setTabSize(int size)
{
    tabsize = size;
}

void Console::setHexMode(bool enable)
{
    hexmode = enable;
}

void Console::setHexDump(bool enable)
{
    hexdump = enable;
}

#ifdef EVENT_DRIVEN
enum { BUFFERSIZE = 2048 };
#else
#ifdef Q_WS_WIN32
enum { BUFFERSIZE = 16 };
#else
enum { BUFFERSIZE = 64 };
#endif
#endif

#if 1

void Console::updateReady(QextSerialPort* port)
{
    char buf[BUFFERSIZE];
    if(isEnabled == false)
        return;

    if(port->bytesAvailable() < 1)
        return;

    int length = port->readLine(buf,BUFFERSIZE);
    //int length = ba.length();

    if(length < 1)
        return;

    if(hexmode != false) {
        for(int n = 0; n < length; n++)
            dumphex((int)buf[n]);
    }
    else {
#ifdef EVENT_DRIVEN
        while (length > 0) {
            for(int n = 0; n < length; n++)
                update(buf[n]);
            length = port->readLine(buf,BUFFERSIZE);
        }
#else
        for(int n = 0; n < length; n++)
            update(buf[n]);
#endif
    }
}

void Console::dumphex(int ch)
{
    unsigned char c = ch;
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

        cur.insertText(QString(" %1").arg(c,2,16,QChar('0')));
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

        cur.insertText(QString(" %1").arg(c,2,16,QChar('0')));
    }
}

void Console::update(char ch)
{
    int _x, _y;

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

    _x = cur.columnNumber();
    _y = cur.blockNumber();


    //qDebug() << QString("%1 %2 %3 %4 %5").arg(pcmd,2,10,QChar('0')).arg(pcmdx,2,10,QChar('0')).arg(pcmdy,2,10,QChar('0')).arg(ch,2,10,QChar('0')).arg(ch,1,QChar('0'));
    //qDebug() << QString("P %1,%2 X,Y %3,%4 %5").arg(pcmdx,2,10,QChar('0')).arg(pcmdy,2,10,QChar('0')).arg(cur.columnNumber(),2,10,QChar('0')).arg(cur.blockNumber(),2,10,QChar('0')).arg(ch,2,10,QChar('0'));
    switch(pcmd)
    {
        case PCMD_CURPOS_X: {
                pcmdx = ch;
#if 0
                cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,pcmdy);
                setTextCursor(cur);
#endif
#if 1
                int j = cur.block().length();
                for(; j < pcmdx; j++) {
                    cur.movePosition(QTextCursor::EndOfLine,QTextCursor::MoveAnchor);
                    cur.insertText(" ");
                }
#endif
                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                if(pcmdx > 0) cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pcmdx);
                setTextCursor(cur);
                pcmd = PCMD_NONE;
            }
            break;

        case PCMD_CURPOS_Y: {
                pcmdy = ch;
                int j = this->blockCount();
                for(; j <= pcmdy; j++) {
                    cur.movePosition(QTextCursor::End);
                    cur.insertBlock(cur.blockFormat(),cur.charFormat());
                }
                cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                if(pcmdy > 0) cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,pcmdy);
                setTextCursor(cur);
#if 0
                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pcmdx);
                setTextCursor(cur);
#endif
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
                    for(; j <= pcmdy; j++) {
                        cur.movePosition(QTextCursor::End);
                        cur.insertBlock();
                    }
                    cur.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
                    if(pcmdy > 0)
                        cur.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pcmdy);

                    j = cur.block().length();
                    for(; j < pcmdx; j++) {
                        cur.movePosition(QTextCursor::EndOfLine,QTextCursor::MoveAnchor);
                        cur.insertText(" ");
                    }
                    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                    if(pcmdx > 0)
                        cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pcmdx);
                    setTextCursor(cur);
                }
                pcmdlen--;
                if(pcmdlen < 1) {
                    pcmd = PCMD_NONE;
                }
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
                return;
            }

            switch(ch)
            {
            case EN_ClearScreen: {
                    if(this->enableClearScreen) {
                        setPlainText("");
                    }
                }
                break;
            case EN_ClearScreen2: {
                    if(this->enableClearScreen16) {
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
                        if(cur.columnNumber() > 0) {
                            cur.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor);
                            setTextCursor(cur);
                        }
                    }
                }
                break;

            case EN_MoveCursorRight: {
                    if(this->enableMoveCursorRight) {
                        if(cur.columnNumber() < cur.block().length())
                            cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor);
                        else
                            cur.insertText(" ");
                        setTextCursor(cur);
                    }
                }
                break;

            case EN_MoveCursorUp: {
                    if(this->enableMoveCursorUp) {
                        int col = cur.columnNumber();
                        if(cur.blockNumber() > 0) {
                            cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                            cur.movePosition(QTextCursor::Up,QTextCursor::MoveAnchor);
                            int j = cur.columnNumber();
                            if(j < col) {
                                for(;j <= col; j++) {
                                    cur.movePosition(QTextCursor::EndOfBlock,QTextCursor::MoveAnchor);
                                    cur.insertText(" ");
                                }
                                cur.movePosition(QTextCursor::Up,QTextCursor::MoveAnchor);
                                cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,col);
                            }
                            setTextCursor(cur);
                        }
                    }
                }
                break;

            case EN_MoveCursorDown: {
                    if(this->enableMoveCursorDown) {
                        if(cur.blockNumber()+1 < this->blockCount())
                            cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor);
                        else
                            cur.insertBlock();
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
                            moveCursor(QTextCursor::Right,QTextCursor::MoveAnchor);
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
                            int row = cur.blockNumber();
                            int max = this->blockCount()-1;
                            if(row < max) {
                                // insert a newline
                                cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
                                text = cur.selectedText();
                                cur.removeSelectedText();
                                cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor);
                                cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
                                cur.insertText(text);
                            }
                            else {
                                cur.insertBlock();
                            }
#if 0
                            if(this->enableAddCRtoNL == false) {
                                while(col-- > -1) {
                                    cur.insertText(" ");
                                    cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor, 1);
                                }
                            }
#endif
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
                }
                break;

            case EN_ClearToEOL: {
                    if(this->enableClearToEOL) {
                        int end = cur.block().length();
                        int col = cur.columnNumber();
                        if(end > col) {
                            cur.clearSelection();
                            cur.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,end-col-1);
                            text = cur.selectedText();
                            col = cur.columnNumber();
                            if(text.length()) {
                                cur.removeSelectedText();
                                col = cur.columnNumber();
                                text = cur.selectedText();
                                col = cur.columnNumber();
                                setTextCursor(cur);
                                col = cur.columnNumber();
                            }
                        }
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
                    if(cur.block().length()-1 > cur.columnNumber())
                        cur.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
                    cur.insertText(QString(ch));
                    setTextCursor(cur);
                }
                break;
            }
            break;

        } // end pcmd default
    } // end pcmd switch
    return;
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

    if(maxrow != this->maximumBlockCount()) {
        maxrow  = this->maximumBlockCount();
        if(sbuff != NULL) {
            delete sbuff;
        }
        sbuff = new char[maxrow*256];
    }

    if(hexmode != false) {
        for(int n = 0; n < length; n++)
            dumphex((int)ba.at(n));
        return;
    }
    for(int n = 0; n < length; n++)
        update(ba.at(n));

    this->setPlainText(sbuff);
}

void Console::dumphex(int ch)
{
}


void Console::update(char ch)
{
    int pos = pcmdy*maxcol+pcmdx;
    int maxchars = maxrow*maxcol;

    qDebug() << QString("%1 %2,%3 %4 %5").arg(pcmd,2,10,QChar('0')).arg(pcmdx,2,10,QChar('0')).arg(pcmdy,2,10,QChar('0')).arg(ch,2,10,QChar('0')).arg(ch,1,QChar('0'));
    switch(pcmd)
    {
        case PCMD_CURPOS_X:
                pcmdx = ch;
            break;

        case PCMD_CURPOS_Y:
                pcmdy = ch;
            break;

        case PCMD_CURPOS_XY: {
                if(pcmdlen == 2) {
                    pcmdx = ch;
                }
                else if(pcmdlen == 1) {
                    pcmdy = ch;
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
                        sbuff[pos] = utf8;
                        //cur.insertText(QChar(utf8));
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
                return;
            }

            switch(ch)
            {
            case EN_ClearScreen: {
                    if(this->enableClearScreen) {
                        memset(sbuff,' ',maxchars);
                    }
                }
                break;

            case EN_ClearScreen2: {
                    if(this->enableClearScreen16) {
                        memset(sbuff,' ',maxchars);
                    }
                }
                break;

            case EN_HomeCursor: {
                    if(this->enableHomeCursor) {
                        pcmdx = pcmdy = 0;
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
                        pos = pcmdy*maxcol+pcmdx;
                        pos--;
                        if(pos > 0) {
                            pcmdx = pos % maxcol;
                            pcmdy = pos / maxcol;
                        }
                    }
                }
                break;

            case EN_MoveCursorRight: {
                    if(this->enableMoveCursorRight) {
                        pos = pcmdy*maxcol+pcmdx;
                        pos++;
                        if(pos < maxchars) {
                            pcmdx = pos % maxcol;
                            pcmdy = pos / maxcol;
                        }
                    }
                }
                break;

            case EN_MoveCursorUp: {
                    if(this->enableMoveCursorUp) {
                        pos = pcmdy*maxcol+pcmdx;
                        pos-= maxcol;
                        if(pos > 0) {
                            pcmdx = pos % maxcol;
                            pcmdy = pos / maxcol;
                        }
                    }
                }
                break;

            case EN_MoveCursorDown: {
                    if(this->enableMoveCursorDown) {
                        pos = pcmdy*maxcol+pcmdx;
                        pos+= maxcol;
                        if(pos < maxchars) {
                            pcmdx = pos % maxcol;
                            pcmdy = pos / maxcol;
                        }
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
                        pos = pcmdy*maxcol+pcmdx;
                        sbuff[pos] = ' ';
                        pos--;
                        if(pos > 0) {
                            pcmdx = pos % maxcol;
                            pcmdy = pos / maxcol;
                        }
                    }
                }
                break;

            case EN_Tab: {
                    if(this->enableTab) {
                        int n = pos = pcmdy*maxcol+pcmdx;
                        pos += tabsize;
                        if(pos < maxchars) {
                            while(n++ < pos)
                                sbuff[n] = ' ';
                            pcmdx = pos % maxcol;
                            pcmdy = pos / maxcol;
                        }
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
                            pos = pcmdy*maxcol+pcmdx;
                            pos += maxcol;
                            if(pos < maxchars) {
                                pcmdx = pos % maxcol;
                                pcmdy = pos / maxcol;
                            }
                        }
                    }
                    else if(ch == creturn) {
                        if(enableCReturn) {
                            pos = pcmdy*maxcol+pcmdx;
                            if(pos < maxchars) {
                                pcmdx = 0;
                                pcmdy = pos / maxcol;
                            }
                        }
                    }
                }
                break;

            case EN_ClearToEOL: {
                    if(this->enableClearToEOL) {
                        int n = pos = pcmdy*maxcol+pcmdx;
                        pos+= (maxcol-pcmdx);
                        if(pos < maxchars) {
                            while(n < pos)
                                sbuff[n++] = ' ';
                        }
                    }
                }
                break;

            case EN_ClearLinesBelow: {
                    if(this->enableClearLinesBelow) {
                        int n = pcmdy*maxcol+pcmdx;
                        pos   = maxchars;
                        memset(&sbuff[n], ' ', pos-n);
                    }
                }
                break;

            default: {
                    pos = pcmdy*maxcol+pcmdx+1;
                    sbuff[pos] = ch;
                }
                break;
            }

            break;

        } // end pcmd default
    } // end pcmd switch
}

#endif
