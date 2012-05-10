#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>
#include "qextserialport.h"

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);
    void setPortEnable(bool value);

private:
    bool isEnabled;

protected:
    void keyPressEvent(QKeyEvent* event);

public slots:
    void updateReady(QextSerialPort*);

};

#endif // CONSOLE_H
