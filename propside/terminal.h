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
    explicit Terminal(QLabel *status, QPlainTextEdit *compileStatus, QProgressBar *progressBar, QWidget *parent);
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

#if defined(LOADER_TERMINAL)
public:
    Loader  *getEditor();
private:
    Loader  *termEditor;
#else
public:
    Console *getEditor();
private:
    Console *termEditor;
#endif

private:
    QPushButton  *buttonEnable;
};

#endif // TERMINAL_H
