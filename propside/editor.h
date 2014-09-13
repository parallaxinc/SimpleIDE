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

#ifndef EDITOR_H
#define EDITOR_H

#include "gdb.h"
#include "highlighter.h"
#include "spinparser.h"

class LineNumberArea;

class Editor : public QPlainTextEdit
{
    Q_OBJECT
public:
    Editor(GDB *gdb, SpinParser *parser, QWidget *parent);
    virtual ~Editor();

    void setHighlights(QString filename = "");
    void setLineNumber(int num);

    void clearCtrlPressed();

private:
    int  autoEnterColumn();
    int  autoEnterColumnC();
    int  autoEnterColumnSpin();
    int  braceMatchColumn();
    bool isCommentOpen(int line);
    QString spinPrune(QString s);
    void addAutoItem(QString type, QString s);
    void spinAutoShow(int width);
    int  spinAutoComplete();
    int  spinAutoCompleteCON();
    int  contextHelp();
    int  tabBlockShift();
    bool isNotAutoComplete();
    QString selectAutoComplete();
    QString deletePrefix(QString s);

protected:
    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private:
    QWidget *mainwindow;
    QTextCursor lastCursor;
    QPoint  mousepos;
    bool    ctrlPressed;
    GDB     *gdb;
    SpinParser  *spinParser;
    bool    isSpin;
    Highlighter *highlighter;

    QComboBox cbAuto;

private slots:
    void cbAutoSelected(int index);
    void cbAutoSelected0insert(int index);

/* lineNumberArea support below this line: see Nokia Copyright below */
public:
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine(QColor lineColor = QColor::Invalid);
    void updateLineNumberArea(const QRect &, int);
    
private:
    QWidget *lineNumberArea;
    QString fileName;

signals:
    void saveEditorFile();
};


#endif // EDITOR_H
