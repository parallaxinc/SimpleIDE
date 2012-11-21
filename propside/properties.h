/*
 * Copyright (c) 2012 Parallax, Inc.
 * Initial Code by John Steven Denson
 *
 * All Rights for this file are MIT Licensed.
 *
 +--------------------------------------------------------------------
 TERMS OF USE: MIT License
 +--------------------------------------------------------------------
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 +--------------------------------------------------------------------
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "propertycolor.h"

#define publisherKey        "ParallaxInc"
#define publisherComKey     "Parallax.com"
#define ASideGuiKey         "SimpleIDE"

#define simpleViewKey       "SimpleIDE_SimpleViewType"
#define oldViewBoxKey       "SimpleIDE_OldViewBoxReminder"
#define ASideGuiGeometry    "SimpleIDE_WindowGeometry"
#define helpStartupKey      "SimpleIDE_ShowHelpStart"
#define compilerKey         "SimpleIDE_Compiler"
#define includesKey         "SimpleIDE_Includes"
#define workspaceKey        "SimpleIDE_Workspace"
#define spinCompilerKey     "SimpleIDE_SpinCompiler"
#define spinLibraryKey      "SimpleIDE_SpinLibrary"
#define spinWorkspaceKey    "SimpleIDE_SpinWorkspace"
#define cloneSrcKey         "SimpleIDE_CloneSrc"
#define cloneDestKey        "SimpleIDE_CloneDest"
#define separatorKey        "SimpleIDE_PathSeparator"
#define configFileKey       "SimpleIDE_ConfigFile"
#define editorFontKey       "SimpleIDE_EditorFont"
#define fontSizeKey         "SimpleIDE_FontSize"
#define lastFileNameKey     "SimpleIDE_LastFileName"
#define lastBoardNameKey    "SimpleIDE_LastBoardName"
#define lastPortNameKey     "SimpleIDE_LastPortName"
#define termGeometryKey     "SimpleIDE_TermGeometry"
#define recentFilesKey      "SimpleIDE_recentFileList"
#define recentProjectsKey   "SimpleIDE_recentProjectsList"
#define tabSpacesKey        "SimpleIDE_TabSpacesCount"
#define loadDelayKey        "SimpleIDE_LoadDelay_us"
#define resetTypeKey        "SimpleIDE_ResetType"
#define spinCompilerKey     "SimpleIDE_SpinCompiler"
#define altTerminalKey      "SimpleIDE_AltTerminal"
#define hlEnableKey         "SimpleIDE_HighlightEnable"
#define hlNumStyleKey       "SimpleIDE_HighlightNumberStyle"
#define hlNumWeightKey      "SimpleIDE_HighlightNumberWeight"
#define hlNumColorKey       "SimpleIDE_HighlightNumberColor"
#define hlFuncStyleKey      "SimpleIDE_HighlightFunctionStyle"
#define hlFuncWeightKey     "SimpleIDE_HighlightFunctionWeight"
#define hlFuncColorKey      "SimpleIDE_HighlightFunctionColor"
#define hlKeyWordStyleKey   "SimpleIDE_HighlightKeyWordTypeStyle"
#define hlKeyWordWeightKey  "SimpleIDE_HighlightKeyWordTypeWeight"
#define hlKeyWordColorKey   "SimpleIDE_HighlightKeyWordTypeColor"
#define hlPreProcStyleKey   "SimpleIDE_HighlightPreProcTypeStyle"
#define hlPreProcWeightKey  "SimpleIDE_HighlightPreProcTypeWeight"
#define hlPreProcColorKey   "SimpleIDE_HighlightPreProcTypeColor"
#define hlQuoteStyleKey     "SimpleIDE_HighlightQuoteTypeStyle"
#define hlQuoteWeightKey    "SimpleIDE_HighlightQuoteTypeWeight"
#define hlQuoteColorKey     "SimpleIDE_HighlightQuoteTypeColor"
#define hlLineComStyleKey   "SimpleIDE_HighlightLineCommmentTypeStyle"
#define hlLineComWeightKey  "SimpleIDE_HighlightLineCommmentTypeWeight"
#define hlLineComColorKey   "SimpleIDE_HighlightLineCommmentTypeColor"
#define hlBlockComStyleKey  "SimpleIDE_HighlightBlockCommentTypeStyle"
#define hlBlockComWeightKey "SimpleIDE_HighlightBlockCommentTypeWeight"
#define hlBlockComColorKey  "SimpleIDE_HighlightBlockCommentTypeColor"
#define HintKeyPrefix       "SimpleIDE_Hint_"

#include <QtGui>

class Properties : public QDialog
{
    Q_OBJECT
public:
    explicit Properties(QWidget *parent = 0);

    int getTabSpaces();
    int getLoadDelay();
    int setComboIndexByValue(QComboBox *combo, QString value);

    Qt::GlobalColor getQtColor(int index);

    enum Reset { DTR=0, RTS, CFG, AUTO };
    Reset getResetType();

signals:

public slots:
    void cleanSettings();
    void browseCompiler();
    void browseIncludes();
    void browseWorkspace();

    void accept();
    void reject();
    void showProperties();
    void browseSpinCompiler();
    void browseSpinLibrary();
    void browseSpinWorkspace();

    QString getCompilerStr() { return leditCompiler->text(); }
    QString getIncludesStr() { return leditIncludes->text(); }
    QString getWorkspaceStr() { return leditWorkspace->text(); }
    QString getSpinCompilerStr() { return leditSpinCompiler->text(); }
    QString getSpinLibraryStr()  { return leditSpinLibrary->text(); }
    QString getSpinWorkspaceStr() { return leditSpinWorkspace->text(); }

private:

    void setupFolders();
    void setupSpinFolders();
    void setupGeneral();
    void setupOptional();
    void addHighlights(QComboBox *box, QVector<PColor*> p);
    void setupHighlight();

    void fileStringProperty(QVariant *var, QLineEdit *ledit, const char *key, QString *value);

    QTabWidget  tabWidget;
    QString     mypath;

    QString     compilerstr;
    QString     includesstr;
    QString     workspacestr;
    QString     spinCompilerStr;
    QString     spinLibraryStr;
    QString     spinWorkspaceStr;
    
    QString     tabSpacesStr;
    QString     loadDelayStr;
    Reset       resetTypeEnum;

    bool         hlNumStyleBool;
    bool         hlNumWeightBool;
    int          hlNumColorIndex;
    bool         hlFuncStyleBool;
    bool         hlFuncWeightBool;
    int          hlFuncColorIndex;
    bool         hlKeyWordStyleBool;
    bool         hlKeyWordWeightBool;
    int          hlKeyWordColorIndex;
    bool         hlPreProcStyleBool;
    bool         hlPreProcWeightBool;
    int          hlPreProcColorIndex;
    bool         hlQuoteStyleBool;
    bool         hlQuoteWeightBool;
    int          hlQuoteColorIndex;
    bool         hlLineComStyleBool;
    bool         hlLineComWeightBool;
    int          hlLineComColorIndex;
    bool         hlBlockComStyleBool;
    bool         hlBlockComWeightBool;
    int          hlBlockComColorIndex;

    QLineEdit   *leditCompiler;
    QLineEdit   *leditIncludes;
    QLineEdit   *leditWorkspace;

    QLineEdit   *leditSpinCompiler;
    QLineEdit   *leditSpinLibrary;
    QLineEdit   *leditSpinWorkspace;

    QLineEdit   tabSpaces;
    QLineEdit   loadDelay;
    QComboBox   resetType;

    QLineEdit   leditAltTerminal;

    QToolButton hlNumButton;

    QCheckBox   hlEnable;

    QCheckBox   hlNumStyle;
    QCheckBox   hlNumWeight;
    QComboBox   hlNumColor;
    QCheckBox   hlFuncStyle;
    QCheckBox   hlFuncWeight;
    QComboBox   hlFuncColor;
    QCheckBox   hlKeyWordStyle;
    QCheckBox   hlKeyWordWeight;
    QComboBox   hlKeyWordColor;
    QCheckBox   hlPreProcStyle;
    QCheckBox   hlPreProcWeight;
    QComboBox   hlPreProcColor;
    QCheckBox   hlQuoteStyle;
    QCheckBox   hlQuoteWeight;
    QComboBox   hlQuoteColor;
    QCheckBox   hlLineComStyle;
    QCheckBox   hlLineComWeight;
    QComboBox   hlLineComColor;
    QCheckBox   hlBlockComStyle;
    QCheckBox   hlBlockComWeight;
    QComboBox   hlBlockComColor;

    QVector<PColor*> propertyColors;
};




#endif // PROPERTIES_H
