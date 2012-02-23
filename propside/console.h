#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>
#include "qextserialport.h"

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent* event);

public slots:
    void updateReady(QextSerialPort*);
    void updateReady(char *buff, int length);
};

#endif // CONSOLE_H
