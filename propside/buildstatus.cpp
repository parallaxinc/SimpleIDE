#include "buildstatus.h"

#ifdef SPINSIDE
#include "mainspinwindow.h"
#define MAINWINDOW MainSpinWindow
#else
#include "mainwindow.h"
#define MAINWINDOW MainWindow
#endif

BuildStatus::BuildStatus(QWidget *parent) : QPlainTextEdit(parent)
{
    Properties *p = static_cast<MAINWINDOW*>(parent)->propDialog;
    highlighter = new HighlightBuild(this->document(), p);
}

BuildStatus::~BuildStatus()
{
    delete highlighter;
}
