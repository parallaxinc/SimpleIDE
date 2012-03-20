#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QDialog(parent)
{
#if !defined(LOADER_TERMINAL)
    termEditor = new Console(parent);
#endif
    init();
}

Terminal::Terminal(QLabel *status, QPlainTextEdit *compileStatus, QProgressBar *progressBar, QWidget *parent) : QDialog(parent)
{
#if defined(LOADER_TERMINAL)
    termEditor = new Loader(status,compileStatus,progressBar,this);
#endif
    init();
}

void Terminal::init()
{
    QVBoxLayout *termLayout = new QVBoxLayout();
    termEditor->setReadOnly(false);
    termEditor->setContextMenuPolicy(Qt::NoContextMenu);

    QAction *copyAction = new QAction(tr("Copy"),this);
    copyAction->setShortcuts(QKeySequence::Copy);
    termEditor->addAction(copyAction);
    QAction *pasteAction = new QAction(tr("Paste"),this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    termEditor->addAction(pasteAction);

    termLayout->addWidget(termEditor);

    QPushButton *buttonClear = new QPushButton(tr("Clear"),this);
    connect(buttonClear,SIGNAL(clicked()), this, SLOT(clearScreen()));

    buttonEnable = new QPushButton(tr("Disable"),this);
    connect(buttonEnable,SIGNAL(clicked()), this, SLOT(toggleEnable()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *butLayout = new QHBoxLayout();
    termLayout->addLayout(butLayout);
    butLayout->addWidget(buttonClear);
    butLayout->addWidget(buttonEnable);
    butLayout->addWidget(buttonBox);
    setLayout(termLayout);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/images/console.png"));
    resize(640,400);
}

#if defined(LOADER_TERMINAL)
Loader *Terminal::getEditor()
{
    return termEditor;
}
#else
Console *Terminal::getEditor()
{
    return termEditor;
}
#endif

void Terminal::setPosition(int x, int y)
{
    int width = this->width();
    int height = this->height();
    this->setGeometry(x,y,width,height);
}

void Terminal::accept()
{
    buttonEnable->setText("Disable");
#if defined(LOADER_TERMINAL)
    termEditor->kill();
#else
    termEditor->setPortEnable(false);
#endif
    done(QDialog::Accepted);
}

void Terminal::reject()
{
    buttonEnable->setText("Disable");
#if defined(LOADER_TERMINAL)
    termEditor->kill();
#else
    termEditor->setPortEnable(false);
#endif
    done(QDialog::Rejected);
}

void Terminal::clearScreen()
{
    termEditor->setPlainText("");
}

void Terminal::toggleEnable()
{
    if(buttonEnable->text().contains("Enable",Qt::CaseInsensitive)) {
        buttonEnable->setText("Disable");
        termEditor->setPortEnable(true);
    }
    else {
        buttonEnable->setText("Enable");
        termEditor->setPortEnable(false);
    }
    QApplication::processEvents();
}

void Terminal::setPortEnabled(bool value)
{
    if(value) {
        buttonEnable->setText("Disable");
        termEditor->setPortEnable(true);
    }
    else {
        buttonEnable->setText("Enable");
        termEditor->setPortEnable(false);
    }
    QApplication::processEvents();
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
