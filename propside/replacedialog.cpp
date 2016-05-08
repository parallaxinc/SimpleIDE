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
 * TODO: put findNext in find and __findNext in replace and try replace up/down.
 */

#include "qtversion.h"

#include <QMargins>
#include "replacedialog.h"

#define USE_REGEX 0

ReplaceDialog::ReplaceDialog(QWidget *parent) : QDialog(parent)
{
    int row = 0;
    int col = 0;
    int span = 1;

    findForward = true;

    QLabel *findLabel = new QLabel(tr("Find text:"));
    findEdit = new QLineEdit;
    // don't connect find text, it causes unnecessary changes in editors
    //connect(findEdit,SIGNAL(textChanged(QString)),this,SLOT(findChanged(QString)));
    QLabel *replaceLabel = new QLabel(tr("Replace with:"));
    replaceEdit = new QLineEdit;

    int bwd = 35;
    findNextButton = new QToolButton();
    findNextButton->setIcon(QIcon(":/images/next.png"));
    findNextButton->setMinimumWidth(bwd);
    findNextButton->setToolTip(tr("Find Next"));
    findPrevButton = new QToolButton();
    findPrevButton->setIcon(QIcon(":/images/previous.png"));
    findPrevButton->setMinimumWidth(bwd);
    findPrevButton->setToolTip(tr("Find Previous"));
    findText = "";

    replaceNextButton = new QToolButton();
    replaceNextButton->setIcon(QIcon(":/images/next.png"));
    replaceNextButton->setMinimumWidth(bwd);
    replaceNextButton->setToolTip(tr("Replace and Find Next"));
    replacePrevButton = new QToolButton();
    replacePrevButton->setIcon(QIcon(":/images/previous.png"));
    replacePrevButton->setMinimumWidth(bwd);
    replacePrevButton->setToolTip(tr("Replace and Find Previous"));
    replaceText = "";
    replaceAllButton = new QPushButton(tr("Replace All"));

    wholeWordButton = new QToolButton(this);
    wholeWordButton->setToolTip(tr("Whole Word"));
    wholeWordButton->setIcon(QIcon(":/images/word.png"));
    wholeWordButton->setMinimumWidth(bwd);
    wholeWordButton->setCheckable(true);

    caseSensitiveButton = new QToolButton(this);
    caseSensitiveButton->setToolTip(tr("Case Sensitive"));
    caseSensitiveButton->setIcon(QIcon(":/images/case.png"));
    caseSensitiveButton->setMinimumWidth(bwd);
    caseSensitiveButton->setCheckable(true);

#if USE_REGEX
    regexButton = new QToolButton(this);
    regexButton->setToolTip(tr("RegEx Function"));
    regexButton->setIcon(QIcon(":/images/dotstar.png"));
    regexButton->setCheckable(true);
#endif

    QHBoxLayout *optLayout = new QHBoxLayout();

    okButton = new QPushButton(tr("Find"), this);

    layout = new QGridLayout();
    layout->addWidget(findLabel,row,col,span,span);
    layout->addWidget(findEdit,row,++col,span,span);
    col++;
    layout->addWidget(findPrevButton,row,++col,span,span);
    layout->addWidget(findNextButton,row,++col,span,span);

    QLabel *label      = new QLabel(tr("Use Up/Down arrows to find/replace. Other small buttons filter."));

    optLayout->addWidget(wholeWordButton);
    optLayout->addWidget(caseSensitiveButton);
#if USE_REGEX
    optLayout->addWidget(regexButton);
#endif
    layout->addLayout(optLayout,row,++col,span,span);

    row++;
    col= 0;
    layout->addWidget(replaceLabel,row,col,span,span);
    layout->addWidget(replaceEdit,row,++col,span,span);
    col++;
    layout->addWidget(replacePrevButton,row,++col,span,span);
    layout->addWidget(replaceNextButton,row,++col,span,span);

    layout->addWidget(replaceAllButton,row,++col,span,span);
    row+=2;
    layout->addWidget(label,row,0,span,5);
    layout->addWidget(okButton,row,col,span,span);
    okButton->setDefault(true);

    setLayout(layout);
    setMinimumWidth(500);

    setWindowTitle(tr("Find/Replace Text"));
    connect(findNextButton, SIGNAL(clicked()), this, SLOT(findNextClicked()));
    connect(findPrevButton, SIGNAL(clicked()), this, SLOT(findPrevClicked()));
    connect(replaceNextButton, SIGNAL(clicked()), this, SLOT(replaceNextClicked()));
    connect(replacePrevButton, SIGNAL(clicked()), this, SLOT(replacePrevClicked()));
    connect(replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAllClicked()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(findDirection()));

    editor = NULL;
    findPosition = 0;
}

void ReplaceDialog::findDirection()
{
    if(findForward) {
        findNextClicked();
    }
    else {
        findPrevClicked();
    }
}

QTextDocument::FindFlag ReplaceDialog::getFlags(int prev)
{
    int flags = prev;
    if(this->caseSensitiveButton->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if(this->wholeWordButton->isChecked())
        flags |= QTextDocument::FindWholeWords;
    return (QTextDocument::FindFlag) flags;
}

void ReplaceDialog::setFindHighlight(QPlainTextEdit *ed)
{
    if (ed != NULL) {
        QPalette p = editor->palette();
        p.setColor(QPalette::Highlight, QColor(Qt::yellow));
        //p.setColor(QPalette::HighlightedText, QColor(/*what ever you want*/);
        ed->setPalette(p);
    }
}

/*
 * Find text for user as typed in find line edit box.
 */
void ReplaceDialog::findChanged(QString text)
{
    if(editor == NULL)
        return;

    QTextCursor cur = editor->textCursor();
    cur.beginEditBlock();
    cur.setPosition(findPosition,QTextCursor::MoveAnchor);
    editor->setTextCursor(cur);
    cur.endEditBlock();

#if USE_REGEX
    if(regexButton->isChecked()) {
        QRegExp reg(text);
        reg.setPatternSyntax(QRegExp::RegExp2);
        QTextDocument *ted = const_cast<QTextDocument *>(editor->document());
        cur.beginEditBlock();
        cur = ted->find(reg,findPosition,getFlags());
        editor->setTextCursor(cur);
        cur.endEditBlock();
    }
    else
    {
        /* non regex version */
        editor->find(text,getFlags());
    }
#else
    editor->find(text,getFlags());
#endif

}

void ReplaceDialog::findClicked()
{
    QString text = findEdit->text();

    if (text.isEmpty()) {
        return;
    } else {
        findText = text;
        hide();
    }
}

void ReplaceDialog::findNextClicked()
{
    if(editor == NULL)
        return;

    findForward = true;
    int count = 0;
    QString text = findEdit->text();
    QString edtext = editor->toPlainText();

    editor->setCenterOnScroll(true);

#if USE_REGEX
    if(regexButton->isChecked()) {
        QRegExp reg(text);
        QTextDocument *ted = const_cast<QTextDocument *>(editor->document());
        QTextCursor cur = ted->find(reg,findPosition,getFlags());
        if(cur.hasSelection()) {
            count++;
        }
        else {
            if(showBeginMessage(tr("Find"))) {
                QTextCursor cur = ted->find(reg,findPosition,getFlags());
                if(cur.hasSelection()) {
                    count++;
                }
            }
        }
        editor->setTextCursor(cur);
    }
    else
#endif
    {
        if(editor->find(text,getFlags()) == true) {
            /*
            QTextCursor cur = editor->textCursor();
            QTextCharFormat fmt;
            QBrush findBrush(Qt::yellow);
            fmt.setBackground(findBrush);
            cur.mergeCharFormat(fmt);
            cur.movePosition(QTextCursor::StartOfWord);
            */
            count++;
        }
        else {
            if(showBeginMessage(tr("Find"))) {
                if(editor->find(text,getFlags()) == true) {
                    count++;
                }
            }
        }
    }

    if(count > 0) {
        findPosition = editor->textCursor().position();
    }

}
void ReplaceDialog::findPrevClicked()
{
    if(editor == NULL)
        return;

    findForward = false;
    int count = 0;
    QString text = findEdit->text();
    QString edtext = editor->toPlainText();

    editor->setCenterOnScroll(true);

#if USE_REGEX
    if(regexButton->isChecked()) {
        QRegExp reg(text);
        QTextDocument *ted = const_cast<QTextDocument *>(editor->document());
        QTextCursor cur = ted->find(reg,findPosition,getFlags(QTextDocument::FindBackward));
        if(cur.hasSelection()) {
            count++;
        }
        else {
            if(showBeginMessage(tr("Find"))) {
                QTextCursor cur = ted->find(reg,findPosition,getFlags(QTextDocument::FindBackward));
                if(cur.hasSelection()) {
                    count++;
                }
            }
        }
        editor->setTextCursor(cur);
    }
    else
#endif
    {
        if(edtext.contains(text,Qt::CaseInsensitive)) {
            if(editor->find(text,getFlags(QTextDocument::FindBackward)) == true) {
                count++;
            }
            else {
                if(showEndMessage(tr("Find"))) {
                    if(editor->find(text,getFlags(QTextDocument::FindBackward)) == true) {
                        count++;
                    }
                }
            }
        }
    }

    if(count > 0) {
        QTextCursor cur = editor->textCursor();
        int len = cur.selectedText().length();
        cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
        //cur.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
        cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,len);
        editor->setTextCursor(cur);
        findPosition = editor->textCursor().position();
    }
}

QString ReplaceDialog::getFindText()
{
    return findText;
}
void ReplaceDialog::clearFindText()
{
    findEdit->clear();
}
void ReplaceDialog::setFindText(QString text)
{
    return findEdit->setText(text);
}

void ReplaceDialog::replaceClicked()
{
    QString text = replaceEdit->text();

    if (text.isEmpty()) {
        return;
    } else {
        replaceText = text;
        hide();
    }
}
/*
 * If find text is highlighted, replace and find again.
 */
void ReplaceDialog::replaceNextClicked()
{
    if(editor == NULL)
        return;

    QString s = editor->textCursor().selectedText();
    if(s.length()) {
        QTextCursor cur = editor->textCursor();
        cur.beginEditBlock();
        cur.removeSelectedText();
        cur.insertText(replaceEdit->text());
        cur.endEditBlock();
        findNextClicked();
    }
}

/*
 * If find text is highlighted, replace and find again.
 */
void ReplaceDialog::replacePrevClicked()
{
    if(editor == NULL)
        return;

    QString s = editor->textCursor().selectedText();
    if(s.length()) {
        QTextCursor cur = editor->textCursor();
        cur.beginEditBlock();
        cur.removeSelectedText();
        cur.insertText(replaceEdit->text());
        cur.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
        editor->setTextCursor(cur);
        cur.endEditBlock();

        findPrevClicked();
    }
}

void ReplaceDialog::replaceAllClicked()
{
    int count = 0;
    QString text = findEdit->text();
    if(editor == NULL)
        return;

    editor->textCursor().beginEditBlock();
    QString edtext = editor->toPlainText();
    editor->setCenterOnScroll(true);

    QString s = editor->textCursor().selectedText();
    if(s.length()>0) {
        editor->textCursor().removeSelectedText();
        editor->textCursor().insertText(replaceEdit->text());
        count++;
    }
    while(edtext.contains(text,Qt::CaseInsensitive)) {
        editor->find(text,getFlags());
        s = editor->textCursor().selectedText();
        if(s.length()>0) {
            editor->textCursor().removeSelectedText();
            editor->textCursor().insertText(replaceEdit->text());
            count++;
        }
        else {
            if(showBeginMessage(tr("Replace")) == false)
                break;
        }
        edtext = editor->toPlainText();
    }
    editor->textCursor().endEditBlock();

    QMessageBox::information(this, tr("Replace Done"),
        tr("Replaced %1 instances of \"%2\".").arg(count).arg(text));

    okButton->setFocus(); // focus back to find
}

QString ReplaceDialog::getReplaceText()
{
    return replaceText;
}
void ReplaceDialog::clearReplaceText()
{
    replaceEdit->clear();
}

bool ReplaceDialog::showBeginMessage(QString type)
{
    QMessageBox::StandardButton ret = QMessageBox::information(
        this, type, type+tr(" from beginning?"),
        QMessageBox::Cancel, QMessageBox::Ok);
    if(ret == QMessageBox::Cancel) {
        return false;
    }
    QTextCursor cur = editor->textCursor();
    cur.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    editor->setTextCursor(cur);
    return true;
}

bool ReplaceDialog::showEndMessage(QString type)
{
    QMessageBox::StandardButton ret = QMessageBox::information(
        this, type, type+tr(" from end?"),
        QMessageBox::Cancel, QMessageBox::Ok);
    if(ret == QMessageBox::Cancel) {
        return false;
    }
    QTextCursor cur = editor->textCursor();
    cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    editor->setTextCursor(cur);
    return true;
}

void ReplaceDialog::setEditor(QPlainTextEdit *ed)
{
    editor = ed;
    editor->clearFocus();
    findEdit->setFocus();
    setFindHighlight(editor);
}
