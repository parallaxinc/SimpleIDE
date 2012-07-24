#include "terminal.h"

//#if defined(Q_WS_WIN32)
#define TERM_ENABLE_BUTTON
//#endif

Terminal::Terminal(QWidget *parent) : QDialog(parent)
{
    termEditor = new Console(parent);
    init();
}

void Terminal::init()
{
    QVBoxLayout *termLayout = new QVBoxLayout();
    termEditor->setReadOnly(false);

    QAction *copyAction = new QAction(tr("Copy"),this);
    copyAction->setShortcuts(QKeySequence::Copy);
    termEditor->addAction(copyAction);
    QAction *pasteAction = new QAction(tr("Paste"),this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    termEditor->addAction(pasteAction);

    termEditor->setMaximumBlockCount(512);
    termLayout->addWidget(termEditor);

    QPushButton *buttonClear = new QPushButton(tr("Clear"),this);
    connect(buttonClear,SIGNAL(clicked()), this, SLOT(clearScreen()));
    buttonClear->setAutoDefault(false);
    buttonClear->setDefault(false);

    comboBoxBaud = new QComboBox(this);
    comboBoxBaud->addItem("115200", QVariant(BAUD115200));
    comboBoxBaud->addItem("57600", QVariant(BAUD57600));
    comboBoxBaud->addItem("38400", QVariant(BAUD38400));
    comboBoxBaud->addItem("19200", QVariant(BAUD19200));
    comboBoxBaud->addItem("9600", QVariant(BAUD9600));
    comboBoxBaud->addItem("4800", QVariant(BAUD4800));
    comboBoxBaud->addItem("2400", QVariant(BAUD2400));
    comboBoxBaud->addItem("1200", QVariant(BAUD1200));
    connect(comboBoxBaud,SIGNAL(currentIndexChanged(int)),this,SLOT(baudRateChange(int)));

    options = new TermPrefs(termEditor, comboBoxBaud);

    QPushButton *buttonOpt = new QPushButton(tr("Options"),this);
    connect(buttonOpt,SIGNAL(clicked()), this, SLOT(showOptions()));
    buttonOpt->setAutoDefault(false);
    buttonOpt->setDefault(false);

#ifdef TERM_ENABLE_BUTTON
    buttonEnable = new QPushButton(tr("Disable"),this);
    connect(buttonEnable,SIGNAL(clicked()), this, SLOT(toggleEnable()));
    buttonEnable->setAutoDefault(false);
    buttonEnable->setDefault(false);
#endif

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QPushButton* button = buttonBox->button(QDialogButtonBox::Ok);
    button->setAutoDefault(true);
    button->setDefault(true);

    QHBoxLayout *butLayout = new QHBoxLayout();
    termLayout->addLayout(butLayout);
    butLayout->addWidget(buttonClear);
    butLayout->addWidget(buttonOpt);
#ifdef TERM_ENABLE_BUTTON
    butLayout->addWidget(buttonEnable);
#endif
    butLayout->addWidget(comboBoxBaud);
    butLayout->addWidget(buttonBox);
    setLayout(termLayout);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/images/console.png"));
    resize(800,400);
}

Console *Terminal::getEditor()
{
    return termEditor;
}

void Terminal::setPortListener(PortListener *listener)
{
    portListener = listener;
}

void Terminal::setPosition(int x, int y)
{
    int width = this->width();
    int height = this->height();
    this->setGeometry(x,y,width,height);
}

void Terminal::baudRateChange(int index)
{
    QVariant var = comboBoxBaud->itemData(index);
    bool ok;
    int baud = var.toInt(&ok);
    portListener->init(portListener->getPortName(), (BaudRateType) baud);
}

int Terminal::baudRate()
{
    QVariant var = comboBoxBaud->itemData(comboBoxBaud->currentIndex());
    bool ok;
    int baud = var.toInt(&ok);
    return baud;
}

bool Terminal::setBaudRate(int baud)
{
    for(int n = comboBoxBaud->count(); n > -1; n--) {
        if(comboBoxBaud->itemData(n) == baud) {
            comboBoxBaud->setCurrentIndex(n);
            return true;
        }
    }
    return false;
}

void Terminal::accept()
{
#ifdef TERM_ENABLE_BUTTON
    buttonEnable->setText("Disable");
#endif
    // save terminal geometry
    QSettings *settings = new QSettings(publisherKey, ASideGuiKey, this);
    QByteArray geo = this->saveGeometry();
    settings->setValue(termGeometryKey,geo);

    termEditor->setPortEnable(false);
    done(QDialog::Accepted);
}

void Terminal::reject()
{
#ifdef TERM_ENABLE_BUTTON
    buttonEnable->setText("Disable");
#endif
    // save terminal geometry
    QSettings *settings = new QSettings(publisherKey, ASideGuiKey, this);
    QByteArray geo = this->saveGeometry();
    settings->setValue(termGeometryKey,geo);

    termEditor->setPortEnable(false);
    done(QDialog::Rejected);
}

void Terminal::clearScreen()
{
    termEditor->clear();
}

void Terminal::toggleEnable()
{
#ifdef TERM_ENABLE_BUTTON
    if(buttonEnable->text().contains("Enable",Qt::CaseInsensitive)) {
        buttonEnable->setText("Disable");
        termEditor->setPortEnable(true);
        portListener->open();
        termEditor->setFocus(Qt::OtherFocusReason);
    }
    else {
        buttonEnable->setText("Enable");
        termEditor->setPortEnable(false);
        portListener->close();
    }
    QApplication::processEvents();
#endif
}

void Terminal::setPortEnabled(bool value)
{
    if(value) {
#ifdef TERM_ENABLE_BUTTON
        buttonEnable->setText("Disable");
#endif
        termEditor->setPortEnable(true);
    }
    else {
#ifdef TERM_ENABLE_BUTTON
        buttonEnable->setText("Enable");
#endif
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

void Terminal::showOptions()
{
    options->showDialog();
}
