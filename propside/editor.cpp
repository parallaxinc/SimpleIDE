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

#include "linenumberarea.h"
#include "editor.h"
#include "properties.h"

#include "highlightc.h"
#include "spinhighlighter.h"

#ifdef SPINSIDE
#include "mainspinwindow.h"
#define MAINWINDOW MainSpinWindow
#else
#include "mainwindow.h"
#define MAINWINDOW MainWindow
#endif

Editor::Editor(GDB *gdebug, SpinParser *parser, QWidget *parent) : QPlainTextEdit(parent)
{
    mainwindow = parent;
    ctrlPressed = false;
    setMouseTracking(true);
    gdb = gdebug;
    spinParser = parser;
    isSpin = false;

    lineNumberArea = new LineNumberArea(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(parent, SIGNAL(highlightCurrentLine(QColor)), this, SLOT(highlightCurrentLine(QColor)));
    updateLineNumberAreaWidth(0);

    highlighter = NULL;
    setHighlights();
    setCenterOnScroll(true);
}

Editor::~Editor()
{
    delete highlighter;
    delete lineNumberArea;
}

void Editor::setHighlights(QString filename)
{
    fileName = filename;
    Properties *p = static_cast<MAINWINDOW*>(mainwindow)->propDialog;
    if(highlighter != NULL) {
        delete highlighter;
        highlighter = NULL;
    }
    if(filename.isEmpty() == false) {
        if(filename.contains(".spin",Qt::CaseInsensitive)) {
            highlighter = new SpinHighlighter(this->document(), p);
            isSpin = true;
        }
        else {
            highlighter = new HighlightC(this->document(), p);
            isSpin = false;
        }
    }
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
    if((QApplication::keyboardModifiers() & Qt::CTRL)) {
        if(e->key() == Qt::Key_S) {
            emit saveEditorFile();
            return;
        }
    }

    /* source browser */
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
#if 0   // highlighting like this causes editor trouble. don't do it.
        if(text.length() > 0) {
            //qDebug() << "keyPressEvent ctrlPressed " << text;
            if(static_cast<MAINWINDOW*>(mainwindow)->isTagged(text))
                this->setTextCursor(cur);
        }
#endif
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    /* if F1 do help.
     * if tab or shift tab, do block shift
     * if "." do auto complete
     */
    int key = e->key();


    if((key == Qt::Key_Enter) || (key == Qt::Key_Return)) {
        // temporarily remove auto indent for all platforms
#ifdef ENABLE_AUTO_ENTER
        if(autoEnterColumn() == 0)
            QPlainTextEdit::keyPressEvent(e);
#else
        // temporarily remove auto indent for other platforms
        QPlainTextEdit::keyPressEvent(e);
#endif
    }
#ifdef ENABLE_AUTO_ENTER
    else if(key == Qt::Key_BraceRight) {
        if(braceMatchColumn() == 0)
            QPlainTextEdit::keyPressEvent(e);;
    }
#endif
    /* if F1 get word under mouse and pass to findSymbolHelp. no word is ok too. */
    else if(key == Qt::Key_F1) {
        contextHelp();
        QPlainTextEdit::keyPressEvent(e);
    }
#if defined(SPIN_AUTOCOMPLETE)
    /* #-auto complete */
    else if(key == Qt::Key_NumberSign) {
        if(isSpin) {
            if(spinAutoCompleteCON() == 0)
                QPlainTextEdit::keyPressEvent(e);
        }
        else {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
    /* dot-auto complete */
    else if(key == Qt::Key_Period) {
        if(isSpin) {
            if(spinAutoComplete() == 0)
                QPlainTextEdit::keyPressEvent(e);
        }
        else {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
#endif
    /* if TAB key do block move */
    else if(key == Qt::Key_Tab || key == Qt::Key_Backtab) {
        tabBlockShift();
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

void Editor::clearCtrlPressed()
{
    ctrlPressed = false;
}

void Editor::mouseMoveEvent (QMouseEvent *e)
{
    mousepos = e->pos();
    //qDebug() << "mouseMoveEvent " << mousepos.x() << "," << mousepos.y();
    QPlainTextEdit::mouseMoveEvent(e);
}

void Editor::mousePressEvent (QMouseEvent *e)
{
    if(ctrlPressed || (QApplication::keyboardModifiers() & Qt::CTRL) != 0) {
        static_cast<MAINWINDOW*>(mainwindow)->findDeclaration(e->pos());
        qDebug() << "mousePressEvent ctrlPressed" << ctrlPressed << (QApplication::keyboardModifiers() & Qt::CTRL);
        ctrlPressed = false;
    }
    QPlainTextEdit::mousePressEvent(e);
}


int Editor::autoEnterColumn()
{
    if(fileName.endsWith(".spin", Qt::CaseInsensitive)) {
        return autoEnterColumnSpin();
    }
    else if(fileName.endsWith(".c", Qt::CaseInsensitive) ||
            fileName.endsWith(".cpp", Qt::CaseInsensitive) ||
            fileName.endsWith(".h", Qt::CaseInsensitive)
            ) {
        return autoEnterColumnC();
    }
    return 0;
}

int Editor::autoEnterColumnC()
{
    QTextCursor cur = this->textCursor();
    if(cur.selectedText().length() > 0) {
        return 0;
    }

    int line = cur.blockNumber();
    int col  = cur.columnNumber();
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,col);
    QString text = cur.selectedText();
    cur.clearSelection();
    if(text.length() == 0) return 0;

    int stop = -1;
    int indent = -1;
    int star = -1;
    int slcm = -1; // disable single line comment
    // int slcm = text.indexOf("//");

    // don't indent under closed comment
    if(text.indexOf("*/") > -1) {
        return 0;
    }
    if(isCommentOpen(line)) {
        star = stop = text.indexOf("*");
    }
    if(stop < 0 && slcm > -1) {
        stop = slcm;
    }
    if(stop < 0 && text.lastIndexOf("{") == text.length()-1) {
        indent = this->tabStopWidth()/10;
    }

    qDebug() << text;
    /* start a single undo/redo operation */
    cur.beginEditBlock();

    cur.insertBlock();

    for(int n = 0; n <= stop || isspace(text[n].toLatin1()); n++) {
        if(n == star) {
            cur.insertText("*");
        }
        else if(n == slcm) {
            cur.insertText("// ");
        }
        else {
            cur.insertText(" ");
        }
    }

    if(indent > 0) {
        for(int n = 0; n < indent; n++) {
            cur.insertText(" ");
        }
    }

    this->setTextCursor(cur);
    /* end a single undo/redo operation */
    cur.endEditBlock();

    return 1;
}

int Editor::autoEnterColumnSpin()
{
    QTextCursor cur = this->textCursor();
    if(cur.selectedText().length() > 0) {
        return 0;
    }

    int col = cur.columnNumber();

    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,col);
    QString text = cur.selectedText();
    cur.clearSelection();
    if(text.length() == 0) return 0;

    // handle indent for spin
    int stop = -1;
    int indent = -1;
    int slcm = text.indexOf("'");

    if(slcm > -1) {
        stop = slcm;
    }

    //qDebug() << text;
    /* start a single undo/redo operation */
    cur.beginEditBlock();

    cur.insertBlock();
    //cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);

    for(int n = 0; n <= stop || isspace(text[n].toLatin1()); n++) {
        if(n == slcm) {
            if(text.indexOf("''") > -1)
                cur.insertText("''");
            else
                cur.insertText("'");
        }
        else {
            cur.insertText(" ");
        }
    }

    if(indent > 0) {
        for(int n = 0; n < indent; n++) {
            cur.insertText(" ");
        }
    }

    this->setTextCursor(cur);
    /* end a single undo/redo operation */
    cur.endEditBlock();

    return 1;
}


bool Editor::isCommentOpen(int line)
{
    // find out if there is a brace mismatch
    QString s = this->toPlainText();
    QStringList sl = s.split("\n",QString::SkipEmptyParts);
    int length = sl.length();

    int como = 0;
    int comc = 0;

    int lpos = -1;
    int cpos = -1;

    for(int n = 0; n < length; n++) {
        s = sl.at(n);
        if(s.contains("/*")) {
            como++;
        }
        if(s.contains("*/")) {
            comc++;
        }
        cpos = s.lastIndexOf("*");
        if(line == n && como > comc) {
            if(lpos > -1 && cpos == lpos) {
                return true;
            }
        }
        lpos = cpos;

    }
    return !(como == comc);
}

int Editor::braceMatchColumn()
{
    int position = this->textCursor().columnNumber();
    int indent = this->tabStopWidth()/10;

    if(position < indent)
        return 0;

    QTextCursor cur = this->textCursor();

    /* if this line has a brace, ignore */
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    QString text = cur.selectedText();

    cur.clearSelection();
    if(text.contains("{"))
        return 0;

    // find out if there is a brace mismatch
    QString s = this->toPlainText();

    //QStringList sl = this->toPlainText().split(QRegExp("/\\*.*\\*/"));
    //qDebug() << s;
    //s = sl.join("\n");
    //qDebug() << s;
    QStringList sl = s.split("\n",QString::SkipEmptyParts);
    int length = sl.length();

    int braceo = 0;
    int bracec = 0;

    bool blockComment = false;

    for(int n = 0; n < length; n++) {
        QString s = sl.at(n);
        if(s.contains("/*") && s.contains("*/")) {
            QString t = s.mid(s.indexOf("*/")+2);
            s = s.mid(0, s.indexOf("/*"));
            s += t;
        }
        if(blockComment && s.contains("*/")) {
            s = s.mid(s.indexOf("*/")+2);
            blockComment = false;
        }
        if(blockComment)
            continue;
        if(s.contains("/*") && !s.contains("*/")) {
            s = s.mid(0, s.indexOf("/*"));
            blockComment = true;
        }
        //qDebug() << "BMATCH" << s;

        if(s.contains("{")) {
            braceo++;
        }
        if(s.contains("}")) {
            bracec++;
        }
    }

    // if all braces match exit
    if(braceo == bracec) {
        return 0;
    }
    qDebug() << "Brace Mismatch";

    // if brace mismatch, set closing brace
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Up,QTextCursor::MoveAnchor,1);
    cur.movePosition(QTextCursor::Down,QTextCursor::KeepAnchor,1);

    text = cur.selectedText();
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    if(text.length() == 0)
        return 0;

    //cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,text.length());
    setTextCursor(cur);
    cur.clearSelection();

    /* start a single undo/redo operation */
    cur.beginEditBlock();
    //qDebug() << "POS" << (position-indent);
    for(int n = 0; n < position-indent; n++) {
        cur.insertText(" ");
    }
    cur.insertText("}");

    setTextCursor(cur);

    /* end a single undo/redo operation */
    cur.endEditBlock();

    return 1;
}

QString Editor::spinPrune(QString s)
{
    QRegExp re("\\b(byte|long|word)\\b");
    re.setCaseSensitivity(Qt::CaseInsensitive);
    s = s.mid(s.indexOf("\t")+1);
    if(s.lastIndexOf(")") > 0)
        s = s.mid(0,s.lastIndexOf(")")+1);
    if(s.lastIndexOf(":") > 0)
        s = s.mid(0,s.lastIndexOf(":"));
    if(s.lastIndexOf("|") > 0)
        s = s.mid(0,s.lastIndexOf("|"));
    if(s.lastIndexOf("[") > 0)
        s = s.mid(0,s.lastIndexOf("["));
    if(s.lastIndexOf("=") > 0)
        s = s.mid(0,s.lastIndexOf("="));
    if(s.indexOf(re) > 0)
        s = s.mid(0,s.indexOf(re));

    return s.trimmed();
}

QString Editor::deletePrefix(QString s)
{
    QRegExp re("\\b(byte|long|word)\\b");
    re.setCaseSensitivity(Qt::CaseInsensitive);

    if(s.indexOf("con ",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("con",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("con\t",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("con",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("dat ",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("dat",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("dat\t",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("dat",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("pub ",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("pub",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("pub\t",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("pub",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("pri ",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("pri",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("pri\t",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("pri",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("obj ",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("obj",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("obj\t",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("obj",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("var ",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("var",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf("var\t",0,Qt::CaseInsensitive) == 0)
        s = s.mid(s.indexOf("var",0,Qt::CaseInsensitive)+4);
    else if(s.indexOf(re) == 0)
        s = s.mid(s.indexOf(re)+5);

    return s;
}

void Editor::addAutoItem(QString type, QString s)
{
    QIcon icon;

    if(type.mid(0,1).compare("c") == 0)
        icon.addFile(":/images/const.png");
    else if(type.mid(0,1).compare("e") == 0)
        icon.addFile(":/images/const.png");
    else if(type.mid(0,1).compare("o") == 0)
        icon.addFile(":/images/obj.png");
    else if(type.mid(0,1).compare("p") == 0)
        icon.addFile(":/images/pri.png");
    else if(type.mid(0,1).compare("f") == 0)
        icon.addFile(":/images/pub.png");
    else if(type.mid(0,1).compare("v") == 0)
        icon.addFile(":/images/var.png");
    else if(type.mid(0,1).compare("x") == 0)
        icon.addFile(":/images/dat.png");

    if(deletePrefix(s).length() == 0)
        cbAuto.addItem(icon,s);
    else
        cbAuto.addItem(icon,spinPrune(deletePrefix(s)));
}

void Editor::spinAutoShow(int width)
{
    MAINWINDOW *win = static_cast<MAINWINDOW*>(mainwindow);
    int psize = this->font().pointSize();
    width *= psize;
    cbAuto.setFrame(false);
    cbAuto.setEditable(false);
    cbAuto.setAutoCompletion(true);
    cbAuto.setGeometry(win->x()+350, win->y()+100, width, 20);
    cbAuto.showPopup();
}

bool Editor::isNotAutoComplete()
{
    QTextCursor cur = this->textCursor();
    cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    if(cur.selectedText().contains("'")) {
        return true;
    }

    while(cur.atStart() == false) {
        if(cur.selectedText().contains("{")) {
            if(cur.selectedText().contains("}") != true)
                return true;
            else
                return false;
        }
        cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    }

    QTextCursor qcur = this->textCursor();
    int column = qcur.columnNumber();
    qcur.select(QTextCursor::LineUnderCursor);
    QString text = qcur.selectedText();
    if(text.contains("\"")) {
        int instring = 0;
        for(int n = 0; n < text.length(); n++) {
            if(text.at(n) == '\"')
                instring ^= 1;
            if(instring && (n == column))
                return true;
        }
    }
    return false;
}

QString Editor::selectAutoComplete()
{
    QTextCursor cur = this->textCursor();
    int col;
    char ch;
    QString text = cur.selectedText();
    cur.removeSelectedText();
    while((col = cur.columnNumber()) > 0) {
        cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,1);
        text = cur.selectedText();
        ch = text.at(0).toLatin1();
        if(isspace(ch))
            break;
    }
    return cur.selectedText().trimmed();
}


int Editor::spinAutoComplete()
{
    if(isNotAutoComplete())
        return 0;

    QString text = selectAutoComplete();
    // if(text.compare("#") == 0) // would like to autocomplete on previous object

    /*
     * we have an object name. get object info
     */
    if(text.length() > 0) {
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected(int)));
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected0insert(int)));
        connect(&cbAuto, SIGNAL(activated(int)), this, SLOT(cbAutoSelected0insert(int)));
        qDebug() << "keyPressEvent object dot pressed" << text;
        QStringList list = spinParser->spinSymbols(fileName,text);
        if(list.count() == 0)
            return 0;
        cbAuto.clear();
        // we depend on index item 0 to be the auto-start key
        cbAuto.addItem(".");
        if(list.count() > 0) {
            int width = 0;
            list.sort();
            for(int j = 0; j < list.count(); j++) {
                QString s = list[j];
                QString type = s;

#ifdef AUTOCON
                if(type.at(0) == 'c') // don't show con for object. - # shows con
                    continue;
                if(type.at(0) == 'e') // don't show enum for object. - # shows enums
                    continue;
#endif
                if(type.at(0) == 'o') // don't show obj for object.
                    continue;
                if(type.at(0) == 'p') // don't show pri for object.
                    continue;
                if(type.at(0) == 'v') // don't show var for object.
                    continue;
                if(type.at(0) == 'x') // don't show dat for object.
                    continue;

                s = spinPrune(s);
                if(s.length() > width)
                    width = s.length();
                addAutoItem(type, s);
            }
            spinAutoShow(width);
        }
        return 1;
    }
    /*
     * no object name. get local info
     */
    else {
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected(int)));
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected0insert(int)));
        connect(&cbAuto, SIGNAL(activated(int)), this, SLOT(cbAutoSelected(int)));
        qDebug() << "keyPressEvent local dot pressed";
        QStringList list = spinParser->spinSymbols(fileName,"");
        if(list.count() == 0)
            return 0;
        cbAuto.clear();
        cbAuto.addItem(".");
        if(list.count() > 0) {
            int width = 0;
            list.sort();
            // always put objects on top
            for(int j = 0; j < list.count(); j++) {
                QString s = list[j];
                QString type = s;
                if(type.at(0) == 'o') {
                    list.removeAt(j);
                    list.insert(0,s);
                }
            }
            // add all elements
            for(int j = 0; j < list.count(); j++) {
                QString s = list[j];
                QString type = s;
#ifdef AUTOCON
                if(type.at(0) == 'c')
                    continue;
                if(type.at(0) == 'e')
                    continue;
#endif
                s = spinPrune(s);
                if(s.length() > width)
                    width = s.length();
                addAutoItem(type, s);
            }
            spinAutoShow(width);
        }
        return 1;
    }
    return 0;
}

int  Editor::spinAutoCompleteCON()
{
#ifdef AUTOCON
    QString text = selectAutoComplete();

    if(text.length() > 0) {
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected(int)));
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected0insert(int)));
        connect(&cbAuto, SIGNAL(activated(int)), this, SLOT(cbAutoSelected0insert(int)));
        qDebug() << "keyPressEvent # pressed" << text;
        QStringList list = spinParser->spinConstants(fileName,text);
        if(list.count() == 0)
            return 0;
        cbAuto.clear();
        // we depend on index item 0 to be the auto-start key
        cbAuto.addItem(QString("#"));
        if(list.count() > 0) {
            int width = 0;
            list.sort();
            for(int j = 0; j < list.count(); j++) {
                QString s = list[j];
                QString type = s;
                s = spinPrune(s);
                if(s.length() > width)
                    width = s.length();
                addAutoItem(type, s);
            }
            spinAutoShow(width);
        }
        return 1;
    }
    /*
     * no object name. get local info
     */
    else {
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected(int)));
        disconnect(&cbAuto,SIGNAL(activated(int)),this,SLOT(cbAutoSelected0insert(int)));
        connect(&cbAuto, SIGNAL(activated(int)), this, SLOT(cbAutoSelected(int)));
        qDebug() << "keyPressEvent local # pressed";
        QStringList list = spinParser->spinConstants(fileName,"");
        if(list.count() == 0)
            return 0;
        cbAuto.clear();
        cbAuto.addItem(QString("#"));
        if(list.count() > 0) {
            int width = 0;
            list.sort();
            for(int j = 0; j < list.count(); j++) {
                QString s = list[j];
                QString type = s;
                s = spinPrune(s);
                if(s.length() > width)
                    width = s.length();
                addAutoItem(type, s);
            }
            spinAutoShow(width);
        }
        return 1;
    }
#endif
    return 0;
}

void Editor::cbAutoSelected0insert(int index)
{
    QString s = cbAuto.itemText(index);
    QTextCursor cur = this->textCursor();
    s = deletePrefix(s);
    if(index != 0)
        // we depend on index item 0 to be the auto-start key
        cur.insertText(cbAuto.itemText(0)+s.trimmed());
    else
        cur.insertText(cbAuto.itemText(0));
    cbAuto.hide();
}

void Editor::cbAutoSelected(int index)
{
    QString s = cbAuto.itemText(index);
    QTextCursor cur = this->textCursor();
    s = deletePrefix(s);
    cur.insertText(s.trimmed());

    cbAuto.hide();
}

int Editor::contextHelp()
{
    QTextCursor cur = this->cursorForPosition(mousepos);
    cur.select(QTextCursor::WordUnderCursor);
    QString text = cur.selectedText();
    qDebug() << "keyPressEvent F1 " << text;
    static_cast<MAINWINDOW*>(mainwindow)->findSymbolHelp(text);
    return 1;
}

int Editor::tabBlockShift()
{
    int tabSpaces = this->tabStopWidth()/10;

    QTextCursor cur = this->textCursor();

    int curbeg = cur.selectionStart();
    //int curend = cur.selectionEnd();
    //int curpos = cur.position();

    /* do we have shift ? */
    bool shift = false;
    if((QApplication::keyboardModifiers() & Qt::SHIFT))
        shift = true;

    /* if a block is selected */
    if(cur.selectedText().length() > 0 && cur.selectedText().contains(QChar::ParagraphSeparator) == true) {

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
            return 0;

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
    return 1;
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


//![cursorPositionChanged]

/*
 * variation of highlighCurrent line
 */
void Editor::highlightCurrentLine(QColor lineColor)
{
    if(lineColor.isValid() == false) {
        lineColor.setRgb(255,255,255);
    }
    // for gdb use: QColor lineColor = QColor(Qt::yellow).lighter(160);
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

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
