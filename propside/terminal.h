#ifndef TERMINAL_H
#define TERMINAL_H

#include <QtGui>
#include "console.h"

class Terminal : public QDialog
{
    Q_OBJECT
public:
    explicit Terminal(QWidget *parent);
    Console *getEditor();
    void setPosition(int x, int y);
    void accept();
    void reject();

public slots:
    void clearScreen();
    void copyFromFile();
    void cutFromFile();
    void pasteToFile();

private:
    Console *termEditor;
};

#endif // TERMINAL_H
