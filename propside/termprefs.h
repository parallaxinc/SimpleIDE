#ifndef TERMPREFS_H
#define TERMPREFS_H

#include <QtGui>
#include "console.h"
#include "properties.h"

#define appNameKey  ASideGuiKey

#define enableKeyClearScreen       appNameKey "_enableClearScreen"
#define enableKeyHomeCursor        appNameKey "_enableHomeCursor"
#define enableKeyPosXYCursor       appNameKey "_enablePosXYCursor"
#define enableKeyMoveCursorLeft    appNameKey "_enableMoveCursorLeft"
#define enableKeyMoveCursorRight   appNameKey "_enableMoveCursorRight"
#define enableKeyMoveCursorUp      appNameKey "_enableMoveCursorUp"
#define enableKeyMoveCursorDown    appNameKey "_enableMoveCursorDown"
#define enableKeyBeepSpeaker       appNameKey "_enableBeepSpeaker"
#define enableKeyBackspace         appNameKey "_enableBackspace"
#define enableKeyTab               appNameKey "_enableTab"
#define enableKeyCReturn           appNameKey "_enableCReturn"
#define enableKeyClearToEOL        appNameKey "_enableClearToEOL"
#define enableKeyClearLinesBelow   appNameKey "_enableClearLinesBelow"
#define enableKeyNewLine           appNameKey "_enableNewLine"
#define enableKeyPosCursorX        appNameKey "_enablePosCursorX"
#define enableKeyPosCursorY        appNameKey "_enablePosCursorY"
#define enableKeySwapNLCR          appNameKey "_enableSwapNLCR"

namespace Ui {
    class TermPrefs;
}

class TermPrefs : public QDialog
{
    Q_OBJECT
public:
    TermPrefs(Console *con);
    virtual ~TermPrefs();
    void clearAll();
    void saveSettings();
    void readSettings();

    void showDialog();

public slots:
    void accept();
    void reject();

private:
    Ui::TermPrefs  *ui;
    Console        *serialConsole;
    QSettings      *settings;
    QStringList     settingNames;
};

#endif // TERMPREFS_H
