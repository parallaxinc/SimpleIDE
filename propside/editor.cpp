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
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    /* if tab or shift tab, do block shift
     */
    int key = e->key();
    if(key == Qt::Key_Tab || key == Qt::Key_Backtab) {

        QTextCursor cur = this->textCursor();

        int curbeg = cur.selectionStart();
        int curend = cur.selectionEnd();
        int curpos = cur.position();

        /* do we have shift ? */
        bool shift = false;
        if((QApplication::keyboardModifiers() & Qt::SHIFT))
            shift = true;

        /* if a block is selected */
        if(cur.selectedText().length() > 0) {

            QTextBlock blocktext = cur.block();
            QStringList mylist;

            /* move to beginning of block if necessary */
            if(curpos != curbeg) {
                cur.setPosition(curbeg,QTextCursor::MoveAnchor);
                blocktext = cur.block();
                cur.setPosition(curend,QTextCursor::KeepAnchor);
            }

            /* modify block until we're at current end + 1 */
            while(blocktext.contains(curend+1) == false) {
                QString s = blocktext.text();
                if(s.length() == 0)
                    break;
                if(shift == false) {
                    s = "    "+s;
                }
                else {
                    if(s.indexOf("    ") == 0)
                        s = s.mid(4);
                }
                mylist.append(s);
                blocktext = blocktext.next();
            }

            /* remove and replace block */
            if(mylist.length() > 0) {
                QString text = cur.selectedText();
                /* blocks use PragraphSeparator */
                text.replace(QChar::ParagraphSeparator,"\n");
                bool addeol = false;
                QChar ch = text[text.length()-1];
                if(ch == '\n')
                    addeol = true;
                /* remove block */
                cur.removeSelectedText();
                text = "";
                for(int n = 0; n < mylist.count(); n++) {
                    QString s = mylist.at(n);
                    text.append(s);
                    if(n < mylist.count()-1)
                       text.append("\n");
                    else if(addeol)
                        text.append("\n");
                }
                /* replace block */
                cur.insertText(text);

                /* highlight block for user just in case they want to change again */
                cur.setPosition(curbeg,QTextCursor::MoveAnchor);
                cur.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,text.length());
                this->setTextCursor(cur);
            }
        }
        /* no block selected */
        else {
            int cpos = cur.position();
            if(cpos > -1) {
                int n = cur.columnNumber() % 4;
                if (shift == false) {
                    for(; n < 4; n++)
                        insertPlainText(" ");
                }
                else if(cur.columnNumber() != 0) {
                    QString st;
                    for(; n < 4; n++) {
                        cur.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
                        st = cur.selectedText();
                        if(st.length() == 1 && st.at(0) == ' ') {
                            cur.removeSelectedText();
                            if(cur.columnNumber() % 4 == 0)
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
