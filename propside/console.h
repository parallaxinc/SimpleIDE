/*
 * This file is part of the Parallax Propeller SimpleIDE development environment.
 *
 * Copyright (C) 2014 Parallax Incorporated
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include "qtversion.h"
#include "qextserialport.h"
#include "xesp8266port.h"

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);
    void setPortEnable(bool value);
    bool enabled();
    void setSerialPollEnable(bool value);
    bool serialPollEnabled();
    void clear();
    QString eventKey(QKeyEvent* event);

    void setCursorMode();

    void setEnableClearScreen(bool value);
    void setEnableHomeCursor(bool value);
    void setEnablePosXYCursor(bool value);
    void setEnableMoveCursorLeft(bool value);
    void setEnableMoveCursorRight(bool value);
    void setEnableMoveCursorUp(bool value);
    void setEnableMoveCursorDown(bool value);
    void setEnableBeepSpeaker(bool value);
    void setEnableBackspace(bool value);
    void setEnableTab(bool value);
    void setEnableCReturn(bool value);
    void setEnableClearToEOL(bool value);
    void setEnableClearLinesBelow(bool value);
    void setEnableNewLine(bool value);
    void setEnablePosCursorX(bool value);
    void setEnablePosCursorY(bool value);
    void setEnableClearScreen16(bool value);
    void setEnableEchoOn(bool value);
    void setEnableEnterIsNL(bool value);
    void setEnableSwapNLCR(bool value);

    int  getEnter();
    void setWrapMode(int mode);
    void setTabSize(int size);
    void setHexMode(bool enable);
    void setHexDump(bool enable);

public:

    typedef enum {
        EN_ClearScreen = 0,
        EN_HomeCursor,
        EN_PosXYCursor,
        EN_MoveCursorLeft,
        EN_MoveCursorRight, // 4
        EN_MoveCursorUp,
        EN_MoveCursorDown,
        EN_BeepSpeaker,
        EN_Backspace, // 8
        EN_Tab,
        EN_CReturn,
        EN_ClearToEOL,
        EN_ClearLinesBelow, //12
        EN_NewLine,
        EN_PosCursorX,
        EN_PosCursorY,
        EN_ClearScreen2, //16
         // everything from here down is to make save/restore settings easier
        EN_AddCRtoNL,
        EN_EnterIsNL,
        EN_LAST
    } EnableEn;

private:

    typedef enum {
        PCMD_NONE = 0,
        PCMD_CURPOS_XY = 2,
        PCMD_CURPOS_X = 14,
        PCMD_CURPOS_Y = 15
    } PCmdEn;

    PCmdEn  pcmd;
    int     pcmdlen;
    int     pcmdx;
    int     pcmdy;

    bool enableClearScreen;
    bool enableHomeCursor;
    bool enablePosXYCursor;
    bool enableMoveCursorLeft;
    bool enableMoveCursorRight;
    bool enableMoveCursorUp;
    bool enableMoveCursorDown;
    bool enableBeepSpeaker;
    bool enableBackspace;
    bool enableTab;
    bool enableCReturn;
    bool enableClearToEOL;
    bool enableClearLinesBelow;
    bool enableNewLine;
    bool enablePosCursorX;
    bool enablePosCursorY;
    bool enableClearScreen16;
    bool enableEchoOn;
    bool enableEnterIsNL;

    bool enableSwapNLCR;

    bool enableANSI;

    char newline;
    char creturn;
    char lastchar;

    bool isSerialPollEnabled;
    bool isEnabled;
    bool utfparse;
    int  utfbytes;
    int  utf8;

    int  maxcol;
    int  maxrow;

    int  wrapMode;
    int  tabsize;

    bool hexmode;
    int  hexbytes;
    int  maxhex;
    int  hexbyte[17];
    bool hexdump;

    // screen buffer
    char *sbuff;

protected:
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent *e);

public slots:
    void updateReady(QextSerialPort*);
    void updateReady(XEsp8266port *);
    void dumphex(int ch);
    void update(char ch);

};

#endif // CONSOLE_H
