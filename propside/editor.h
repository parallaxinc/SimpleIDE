#ifndef EDITOR_H
#define EDITOR_H

#include <QtGui>

class Editor : public QPlainTextEdit
{
    Q_OBJECT
public:
    Editor(QWidget *parent);

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
};

#endif // EDITOR_H
