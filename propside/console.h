#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>
#include "qextserialport.h"

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);
    void setPortEnable(bool value);
    bool enabled();
    void clear();

    void setEnableClearScreen(bool value) {
         enableClearScreen = value;
    }
    void setEnableHomeCursor(bool value) {
         enableHomeCursor = value;
    }
    void setEnablePosXYCursor(bool value) {
         enablePosXYCursor = value;
    }
    void setEnableMoveCursorLeft(bool value) {
         enableMoveCursorLeft = value;
    }
    void setEnableMoveCursorRight(bool value) {
         enableMoveCursorRight = value;
    }
    void setEnableMoveCursorUp(bool value) {
         enableMoveCursorUp = value;
    }
    void setEnableMoveCursorDown(bool value) {
         enableMoveCursorDown = value;
    }
    void setEnableBeepSpeaker(bool value) {
         enableBeepSpeaker = value;
    }
    void setEnableBackspace(bool value) {
         enableBackspace = value;
    }
    void setEnableTab(bool value) {
         enableTab = value;
    }
    void setEnableCReturn(bool value) {
         enableCReturn = value;
    }
    void setEnableClearToEOL(bool value) {
         enableClearToEOL = value;
    }
    void setEnableClearLinesBelow(bool value) {
         enableClearLinesBelow = value;
    }
    void setEnableNewLine(bool value) {
         enableNewLine = value;
    }
    void setEnablePosCursorX(bool value) {
         enablePosCursorX = value;
    }
    void setEnablePosCursorY(bool value) {
         enablePosCursorY = value;
    }

    void setEnableSwapNLCR(bool value) {
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

    void setRnableAddCRtoNL(bool value) {
         enableAddCRtoNL = value;
    }

private:

    typedef enum {
        PCMD_NONE = 0,
        PCMD_CURPOS_X,
        PCMD_CURPOS_Y,
        PCMD_CURPOS_XY
    } PCmdEn;

    PCmdEn  pcmd;
    int     pcmdlen;

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
        EN_ClearScreen2 //16
    } EnableEn;

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

    bool enableSwapNLCR;
    bool enableAddCRtoNL;

    bool enableANSI;

    char newline;
    char creturn;
    char lastchar;

    bool isEnabled;
    bool utfparse;
    int  utfbytes;
    int  utf8;

protected:
    void keyPressEvent(QKeyEvent* event);

public slots:
    void updateReady(QextSerialPort*);
    void update(char ch);
};

#endif // CONSOLE_H
