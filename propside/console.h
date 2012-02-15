#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent* event);

};

#endif // CONSOLE_H
