#ifndef TERMINAL_H
#define TERMINAL_H

#include <QtGui>
#include "console.h"
#include "PortListener.h"
#include "loader.h"

class Terminal : public QDialog
{
    Q_OBJECT
public:
    explicit Terminal(QWidget *parent);
    void setPosition(int x, int y);
    void accept();
    void reject();

private:
    void init();

public slots:
    void toggleEnable();
    void setPortEnabled(bool value);
    void clearScreen();
    void copyFromFile();
    void cutFromFile();
    void pasteToFile();

public:
    Console *getEditor();
private:
    Console *termEditor;

private:
    QPushButton  *buttonEnable;
};

#endif // TERMINAL_H
