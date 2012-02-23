#include "terminal.h"

Terminal::Terminal(QWidget *parent, PortListener *serialPort) : QDialog(parent)
{
    QVBoxLayout *termLayout = new QVBoxLayout(this);
    port = serialPort;

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
    this->setWindowFlags(Qt::Tool);
    resize(640,400);
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
    buttonEnable->setText("Disable");
    port->enable(false);
    done(QDialog::Accepted);
}

void Terminal::reject()
{
    buttonEnable->setText("Disable");
    port->enable(false);
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
        port->enable(true);
    }
    else {
        buttonEnable->setText("Enable");
        port->enable(false);
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
