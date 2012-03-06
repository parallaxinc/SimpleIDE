#ifndef TERMINAL_H
#define TERMINAL_H

#include <QtGui>
#include "console.h"
#include "PortListener.h"

class Terminal : public QDialog
{
    Q_OBJECT
public:
    explicit Terminal(QWidget *parent, PortListener *serialPort);
    Console *getEditor();
    void setPosition(int x, int y);
    void accept();
    void reject();

public slots:
    void toggleEnable();
    void setPortEnabled(bool value);
    void clearScreen();
    void copyFromFile();
    void cutFromFile();
    void pasteToFile();

private:
    Console      *termEditor;
    PortListener *port;
    QPushButton  *buttonEnable;
};

#endif // TERMINAL_H
