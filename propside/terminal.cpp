#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *termLayout = new QVBoxLayout(this);

    termEditor = new Console(parent);
    termEditor->setReadOnly(false);
    termEditor->setContextMenuPolicy(Qt::NoContextMenu);

    QAction *copyAction = new QAction(tr("Copy"),this);
    copyAction->setShortcuts(QKeySequence::Copy);
    termEditor->addAction(copyAction);
    QAction *pasteAction = new QAction(tr("Paste"),this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    termEditor->addAction(pasteAction);

    termEditor->setMaximumBlockCount(300);
    termLayout->addWidget(termEditor);
    QPushButton *cls = new QPushButton(tr("Clear"),this);
    connect(cls,SIGNAL(clicked()), this, SLOT(clearScreen()));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QHBoxLayout *butLayout = new QHBoxLayout(this);
    termLayout->addLayout(butLayout);
    butLayout->addWidget(cls);
    butLayout->addWidget(buttonBox);
    setLayout(termLayout);
    this->setWindowFlags(Qt::Tool);
    resize(600,400);
}

Console *Terminal::getEditor()
{
    return termEditor;
}

void Terminal::setPosition(int x, int y)
{
    int width = this->width();
    int height = this->height();
    this->setGeometry(x,y,width,height);
}

void Terminal::accept()
{
    done(QDialog::Accepted);
}

void Terminal::reject()
{
    done(QDialog::Rejected);
}

void Terminal::clearScreen()
{
    termEditor->setPlainText("");
}

void Terminal::copyFromFile()
{
    termEditor->copy();
}
void Terminal::cutFromFile()
{
    termEditor->cut();
}
void Terminal::pasteToFile()
{
    termEditor->paste();
}
