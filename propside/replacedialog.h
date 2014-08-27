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

/*
 * Replace is similar to Find.
 */

#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include "qtversion.h"

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    ReplaceDialog(QWidget *parent = 0);
    QTextDocument::FindFlag getFlags(int prev = 0);

    QString getFindText();
    void    clearFindText();
    QString getReplaceText();
    void    clearReplaceText();
    void    setFindText(QString text);

    bool showBeginMessage(QString type);
    bool showEndMessage(QString type);

    void setEditor(QPlainTextEdit *ed);

public slots:
    void findChanged(QString text);
    void findClicked();
    void findDirection();
    void findNextClicked();
    void findPrevClicked();
    void replaceClicked();
    void replaceNextClicked();
    void replacePrevClicked();
    void replaceAllClicked();

private:
    QPlainTextEdit *editor;

    QToolButton *findNextButton;
    QToolButton *findPrevButton;
    QToolButton *replaceNextButton;
    QToolButton *replacePrevButton;
    QPushButton *replaceAllButton;
    QLineEdit   *findEdit;
    QString     findText;
    QLineEdit   *replaceEdit;
    QString     replaceText;

    bool        findForward;
    QPushButton *okButton;

    QToolButton *caseSensitiveButton;
    QToolButton *wholeWordButton;
    QToolButton *regexButton;

    QGridLayout *layout;

    /* last find position */
    int         findPosition;
    int         wasClicked;
};

#endif
// REPLACEDIALOG_H
