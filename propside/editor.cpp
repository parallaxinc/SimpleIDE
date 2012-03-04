#include "editor.h"
#include "mainwindow.h"

Editor::Editor(QWidget *parent) : QPlainTextEdit(parent)
{
    mainwindow = parent;
    ctrlPressed = false;
    setMouseTracking(true);
}

void Editor::keyPressEvent (QKeyEvent *e)
{
    if((QApplication::keyboardModifiers() & Qt::CTRL) && ctrlPressed == false) {
        ctrlPressed = true;
        QTextCursor cur = this->cursorForPosition(mousepos);
        cur.select(QTextCursor::WordUnderCursor);
        QString text = cur.selectedText();
        if(text.length() > 0) {
            //qDebug() << "keyPressEvent ctrlPressed " << text;
            if(static_cast<MainWindow*>(mainwindow)->isTagged(text)) {
                this->setTextCursor(cur);
            }
        }
    }
    QPlainTextEdit::keyPressEvent(e);
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
