/*
 * Replace is similar to Find.
 */

#include <QtGui>
#include "replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent) : QDialog(parent)
{
    int row = 0;
    int col = 0;
    int span = 1;

    QLabel *findLabel = new QLabel(tr("Find text:"));
    findEdit = new QLineEdit;
    QLabel *replaceLabel = new QLabel(tr("Replace with:"));
    replaceEdit = new QLineEdit;

    findNextButton = new QToolButton();
    findNextButton->setIcon(QIcon(":/images/next.png"));
    findNextButton->setToolTip(tr("Find Next"));
    findPrevButton = new QToolButton();
    findPrevButton->setIcon(QIcon(":/images/previous.png"));
    findNextButton->setToolTip(tr("Find Previous"));
    findText = "";

    replaceNextButton = new QToolButton();
    replaceNextButton->setIcon(QIcon(":/images/next.png"));
    replaceNextButton->setToolTip(tr("Replace Next"));
    replacePrevButton = new QToolButton();
    replacePrevButton->setIcon(QIcon(":/images/previous.png"));
    replacePrevButton->setToolTip(tr("Replace Previous"));
    replaceText = "";
    replaceAllButton = new QPushButton(tr("Replace All"));

    wholeWordButton = new QToolButton(this);
    wholeWordButton->setToolTip(tr("Whole Word"));
    wholeWordButton->setIcon(QIcon(":/images/word.png"));
    wholeWordButton->setCheckable(true);

    caseSensitiveButton = new QToolButton(this);
    caseSensitiveButton->setToolTip(tr("Case Sensitive"));
    caseSensitiveButton->setIcon(QIcon(":/images/case.png"));
    caseSensitiveButton->setCheckable(true);

    QHBoxLayout *optLayout = new QHBoxLayout();

    okButton = new QPushButton(tr("&OK"));

    layout = new QGridLayout();
    layout->addWidget(findLabel,row,col,span,span);
    layout->addWidget(findEdit,row,++col,span,span);
    layout->addWidget(findPrevButton,row,++col,span,span);
    layout->addWidget(findNextButton,row,++col,span,span);

    optLayout->addWidget(wholeWordButton);
    optLayout->addWidget(caseSensitiveButton);
    layout->addLayout(optLayout,row,++col,span,span);

    row++;
    col= 0;
    layout->addWidget(replaceLabel,row,col,span,span);
    layout->addWidget(replaceEdit,row,++col,span,span);
    layout->addWidget(replacePrevButton,row,++col,span,span);
    layout->addWidget(replaceNextButton,row,++col,span,span);
    layout->addWidget(replaceAllButton,row,++col,span,span);
    row+=2;
    layout->addWidget(okButton,row,col,span,span);
    okButton->setDefault(true);

    setLayout(layout);
    setWindowFlags(Qt::Tool);
    setMinimumWidth(500);

    setWindowTitle(tr("Find and Replace Text"));
    connect(findNextButton, SIGNAL(clicked()), this, SLOT(findNextClicked()));
    connect(findPrevButton, SIGNAL(clicked()), this, SLOT(findPrevClicked()));
    connect(replaceNextButton, SIGNAL(clicked()), this, SLOT(replaceNextClicked()));
    connect(replacePrevButton, SIGNAL(clicked()), this, SLOT(replacePrevClicked()));
    connect(replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAllClicked()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    editor = NULL;
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

void ReplaceDialog::findClicked()
{
    QString text = findEdit->text();

    if (text.isEmpty()) {
        //QMessageBox::information(this, tr("Empty Field"),tr("Please enter text."));
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

    int count = 0;
    QString text = findEdit->text();
    QString edtext = editor->toPlainText();

    editor->setCenterOnScroll(true);

    if(edtext.contains(text,Qt::CaseInsensitive)) {
        if(editor->find(text,getFlags()) == true) {
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
    if(count == 0) {
        QMessageBox::information(this, tr("Text Not Found"),
            tr("Can't find text: \"%1\"").arg(text));
    }
    else {
        editor->activateWindow();
    }

}
void ReplaceDialog::findPrevClicked()
{
    if(editor == NULL)
        return;

    int count = 0;
    QString text = findEdit->text();
    QString edtext = editor->toPlainText();

    editor->setCenterOnScroll(true);

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
    if(count == 0) {
        QMessageBox::information(this, tr("Text Not Found"),
            tr("Can't find text: \"%1\"").arg(text));
    }
    else {
        editor->activateWindow();
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
        //QMessageBox::information(this, tr("Empty Field"),tr("Please enter text."));
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

    int count = 0;
    QString text = findEdit->text();
    QString edtext = editor->toPlainText();

    editor->setCenterOnScroll(true);

    QTextCursor cur = editor->textCursor();
    cur.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
    editor->setTextCursor(cur);

    if(edtext.contains(text,Qt::CaseInsensitive)) {
        if(editor->find(text,getFlags()) == true) {
            QString s = editor->textCursor().selectedText();
            if(s.isEmpty() == false) {
                count++;
            }
        }
        else {
            if(showBeginMessage(tr("Replace"))) {
                if(editor->find(text,getFlags()) == true) {
                    QString s = editor->textCursor().selectedText();
                    if(s.isEmpty() == false) {
                        count++;
                    }
                }
            }
        }
    }
    if(count == 0) {
        QMessageBox::information(this, tr("Text Not Found"),
            tr("Can't find text: \"%1\"").arg(text));
    }
    else {
        editor->activateWindow();
        editor->textCursor().removeSelectedText();
        editor->textCursor().insertText(replaceEdit->text());
    }
}

/*
 * If find text is highlighted, replace and find again.
 */
void ReplaceDialog::replacePrevClicked()
{
    if(editor == NULL)
        return;

    int count = 0;
    QString text = findEdit->text();
    QString edtext = editor->toPlainText();

    editor->setCenterOnScroll(true);

    QTextCursor cur = editor->textCursor();
    cur.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
    editor->setTextCursor(cur);

    if(edtext.contains(text,Qt::CaseInsensitive)) {
        if(editor->find(text,getFlags(QTextDocument::FindBackward)) == true) {
            QString s = editor->textCursor().selectedText();
            if(s.isEmpty() == false) {
                count++;
            }
        }
        else {
            if(showEndMessage(tr("Replace"))) {
                if(editor->find(text,getFlags(QTextDocument::FindBackward)) == true) {
                    QString s = editor->textCursor().selectedText();
                    if(s.isEmpty() == false) {
                        count++;
                    }
                }
            }
        }
    }
    if(count == 0) {
        QMessageBox::information(this, tr("Text Not Found"),
            tr("Can't find text: \"%1\"").arg(text));
    }
    else {
        editor->activateWindow();
        editor->textCursor().removeSelectedText();
        editor->textCursor().insertText(replaceEdit->text());
    }
}

void ReplaceDialog::replaceAllClicked()
{
    int count = 0;
    QString text = findEdit->text();
    if(editor == NULL)
        return;

    QString edtext = editor->toPlainText();
    do
    {
        if (edtext.contains(text,Qt::CaseInsensitive)) {
            editor->setCenterOnScroll(true);
            editor->find(text,getFlags());
        }
        QString s = editor->textCursor().selectedText();
        if(s.isEmpty() == false) {
            editor->textCursor().removeSelectedText();
            editor->textCursor().insertText(replaceEdit->text());
            count++;
        }
        else {
            if(showBeginMessage(tr("Replace")) == false)
                break;
        }
        edtext = editor->toPlainText();
    } while(edtext.contains(text,Qt::CaseInsensitive));

    QMessageBox::information(this, tr("Replace Done"),
        tr("Replaced %1 instances of \"%2\".").arg(count).arg(text));

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

void ReplaceDialog::accept()
{
    hide();
}

void ReplaceDialog::setEditor(QPlainTextEdit *ed)
{
    editor = ed;
}

