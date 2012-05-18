#include "editor.h"
#include "properties.h"
#include "mainwindow.h"

Editor::Editor(GDB *gdebug, QWidget *parent) : QPlainTextEdit(parent)
{
    mainwindow = parent;
    ctrlPressed = false;
    setMouseTracking(true);
    gdb = gdebug;

    lineNumberArea = new LineNumberArea(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    updateLineNumberAreaWidth(0);

    highlighter = NULL;
    setHighlights();
    setCenterOnScroll(true);
}

Editor::~Editor()
{
    delete highlighter;
}

void Editor::setHighlights()
{
    Properties *p = static_cast<MainWindow*>(mainwindow)->propDialog;
    if(highlighter != NULL)
        delete highlighter;
    highlighter = new Highlighter(this->document(), p);
}

void Editor::setLineNumber(int num)
{
    QTextCursor cur = textCursor();
    cur.setPosition(0);
    cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,num-1);
    setTextCursor(cur);
}

void Editor::keyPressEvent (QKeyEvent *e)
{
    if((QApplication::keyboardModifiers() & Qt::CTRL) && ctrlPressed == false) {
        ctrlPressed = true;
        QTextCursor tcur = this->textCursor();
        QTextCursor cur = this->cursorForPosition(mousepos);
        tcur.select(QTextCursor::WordUnderCursor);
        cur.select(QTextCursor::WordUnderCursor);
        QString text = cur.selectedText();
        if(tcur.selectedText().compare(text)) {
            QPlainTextEdit::keyPressEvent(e);
            return;
        }
        if(text.length() > 0) {
            //qDebug() << "keyPressEvent ctrlPressed " << text;
            if(static_cast<MainWindow*>(mainwindow)->isTagged(text)) {
                this->setTextCursor(cur);
            }
        }
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    int tabSpaces = this->tabStopWidth()/10;

    /* if tab or shift tab, do block shift
     */
    int key = e->key();

    if(key == Qt::Key_F1) {
        QTextCursor cur = this->textCursor();
        cur.select(QTextCursor::WordUnderCursor);
        QString text = cur.selectedText();
        if(text.length() > 0) {
            qDebug() << "keyPressEvent F1 " << text;
            static_cast<MainWindow*>(mainwindow)->findSymbolHelp(text);
        }
        else {
            cur = this->cursorForPosition(mousepos);
            cur.select(QTextCursor::WordUnderCursor);
            text = cur.selectedText();
            if(text.length() > 0) {
                qDebug() << "keyPressEvent F1 " << text;
                static_cast<MainWindow*>(mainwindow)->findSymbolHelp(text);
            }
        }
        QPlainTextEdit::keyPressEvent(e);
        return;
    }
    else
    if(key == Qt::Key_Tab || key == Qt::Key_Backtab) {

        QTextCursor cur = this->textCursor();

        int curbeg = cur.selectionStart();
        //int curend = cur.selectionEnd();
        //int curpos = cur.position();

        /* do we have shift ? */
        bool shift = false;
        if((QApplication::keyboardModifiers() & Qt::SHIFT))
            shift = true;

        /* if a block is selected */
        if(cur.selectedText().length() > 0) {

            QTextBlock blocktext = cur.block();
            QStringList mylist;

            /* make tabs based on user preference - set by mainwindow */
            QString tab = "";
            for(int n = tabSpaces; n > 0; n--) tab+=" ";

            /* highlight block from beginning of the first line to the last line */
            QString text = cur.selectedText();
            int column = cur.columnNumber();
            if(column > 0) {
                cur.setPosition(curbeg-column,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor, text.length()+column);
                text = cur.selectedText();
            }
            if(text.length() == 0)
                return;

            /* get a list of the selected block. keep empty lines */
            mylist = text.split(QChar::ParagraphSeparator);

            /* start a single undo/redo operation */
            cur.beginEditBlock();

            /* get rid of old block */
            cur.removeSelectedText();

            /* indent list */
            text = "";
            for(int n = 0; n < mylist.length(); n++) {
                QString s = mylist.at(n);
                if(s.length() == 0 && n+1 == mylist.length())
                    break;
                if(shift == false) {
                    for(int n = tabSpaces; n > 0; n--) s = " " + s;
                }
                else {
                    if(s.indexOf(tab) == 0 || (n == 0 && column >= tabSpaces))
                        s = s.mid(tabSpaces);
                }
                text += s;
                if(n+1 < mylist.length())
                    text += "\n";
            }

            /* insert new block */
            cur.insertText(text);
            this->setTextCursor(cur);

            /* end single undo/redo operation */
            cur.endEditBlock();
        }
        /* no block selected */
        else {
            int cpos = cur.position();
            if(cpos > -1) {
                int n = cur.columnNumber() % tabSpaces;
                if (shift == false) {
                    for(; n < tabSpaces; n++)
                        insertPlainText(" ");
                }
                else if(cur.columnNumber() != 0) {
                    QString st;
                    for(; n < tabSpaces; n++) {
                        cur.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
                        st = cur.selectedText();
                        if(st.length() == 1 && st.at(0) == ' ') {
                            cur.removeSelectedText();
                            if(cur.columnNumber() % tabSpaces == 0)
                                break;
                        }
                        if(st.at(0) != ' ')
                            cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor);
                    }
                }
            }
            this->setTextCursor(cur);
        }
    }
    else {
        QPlainTextEdit::keyPressEvent(e);
    }
}

void Editor::keyReleaseEvent (QKeyEvent *e)
{
    QTextCursor cur = this->textCursor();
    if(ctrlPressed) {
        ctrlPressed = false;
        cur.setPosition(cur.anchor());
        //qDebug() << "keyReleaseEvent ctrlReleased";
    }
    QPlainTextEdit::keyReleaseEvent(e);
}

void Editor::mouseMoveEvent (QMouseEvent *e)
{
    mousepos = e->pos();
    //qDebug() << "mouseMoveEvent " << mousepos.x() << "," << mousepos.y();
    QPlainTextEdit::mouseMoveEvent(e);
}

void Editor::mousePressEvent (QMouseEvent *e)
{
    if(ctrlPressed) {
        static_cast<MainWindow*>(mainwindow)->findDeclaration(e->pos());
        //qDebug() << "mousePressEvent ctrlPressed";
        ctrlPressed = false;
    }
    QPlainTextEdit::mousePressEvent(e);
}



/* Code below here is taken from the Nokia CodeEditor example */

/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//![extraAreaWidth]

int Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

//![extraAreaWidth]

//![slotUpdateExtraAreaWidth]

void Editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void Editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

//![slotUpdateRequest]

//![resizeEvent]

void Editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

//![resizeEvent]

//![cursorPositionChanged]

void Editor::highlightCurrentLine()
{
#if defined(GDBENABLE)
    if(gdb->enabled() == false)
        return;

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
#endif
}

//![cursorPositionChanged]

//![extraAreaPaintEvent_0]

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(120));

//![extraAreaPaintEvent_0]

//![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
//![extraAreaPaintEvent_1]

//![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
//![extraAreaPaintEvent_2]
