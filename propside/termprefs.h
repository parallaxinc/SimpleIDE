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

#ifndef TERMPREFS_H
#define TERMPREFS_H

class TermPrefs;

#include "qtversion.h"

#include "terminal.h"
#include "console.h"
#include "properties.h"
#include "propertycolor.h"

#define appNameKey  ASideGuiKey

#define enableKeyClearScreen        appNameKey "_enableClearScreen"
#define enableKeyClearScreen16      appNameKey "_enableClearScreen16"
#define enableKeyHomeCursor         appNameKey "_enableHomeCursor"
#define enableKeyPosXYCursor        appNameKey "_enablePosXYCursor"
#define enableKeyMoveCursorLeft     appNameKey "_enableMoveCursorLeft"
#define enableKeyMoveCursorRight    appNameKey "_enableMoveCursorRight"
#define enableKeyMoveCursorUp       appNameKey "_enableMoveCursorUp"
#define enableKeyMoveCursorDown     appNameKey "_enableMoveCursorDown"
#define enableKeyBeepSpeaker        appNameKey "_enableBeepSpeaker"
#define enableKeyBackspace          appNameKey "_enableBackspace"
#define enableKeyTab                appNameKey "_enableTab"
#define enableKeyCReturn            appNameKey "_enableCReturn"
#define enableKeyClearToEOL         appNameKey "_enableClearToEOL"
#define enableKeyClearLinesBelow    appNameKey "_enableClearLinesBelow"
#define enableKeyNewLine            appNameKey "_enableNewLine"
#define enableKeyPosCursorX         appNameKey "_enablePosCursorX"
#define enableKeyPosCursorY         appNameKey "_enablePosCursorY"
#define enableKeySwapNLCR           appNameKey "_enableSwapNLCR"
#define enableKeyAddNLtoCR          appNameKey "_enableAddNLtoCR"
#define enableKeyEnterIsNL          appNameKey "_enableEnterIsNL"

#define termKeyForeground           appNameKey "_termForeground"
#define termKeyBackground           appNameKey "_termBackground"
#define termKeyFontFamily           appNameKey "_termFontFamily"
#define termKeyFontSize             appNameKey "_termFontSize"
#define termKeyFontWeight           appNameKey "_termFontWeight"
#define termKeyFontItalic           appNameKey "_termFontItalic"
#define termKeyWrapMode             appNameKey "_termWrapMode"
#define termKeyPageLineSize         appNameKey "_termPageLineSize"
#define termKeyBufferLines          appNameKey "_termBufferLines"
#define termKeyTabSize              appNameKey "_termTabSize"
#define termKeyHexMode              appNameKey "_termHexMode"
#define termKeyHexDump              appNameKey "_termHexDumpMode"

#define termKeyEchoOn               appNameKey "_termEchoOn"
#define termKeyBaudRate             appNameKey "_termBaudRate"

namespace Ui {
    class TermPrefs;
}

class TermPrefs : public QDialog
{
    Q_OBJECT
public:
    TermPrefs(Terminal *term);
    virtual ~TermPrefs();
    void addColors(QComboBox *box, QVector<PColor*> pcolor);
    void saveSettings();
    void readSettings();

    int  getBaudRate();
    void saveBaudRate(int baud);
    bool  getEchoOn();
    void saveEchoOn(bool echoOn);

    void showDialog();

public slots:
    void hexDump(bool hex);
    void chooseFont();
    void resetSettings();
    void accept();
    void reject();

private:
    Ui::TermPrefs  *ui;
    Terminal       *terminal;
    Console        *serialConsole;
    QSettings      *settings;
    QStringList     settingNames;

    QVector<PColor*> propertyColors;

    QString         styleText;
};

#endif // TERMPREFS_H
