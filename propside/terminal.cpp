#include "terminal.h"

//#if defined(Q_OS_WIN32)
#define TERM_ENABLE_BUTTON
//#endif

Terminal::Terminal(QWidget *parent) : QDialog(parent), portListener(NULL), lastConnectedPortName("")
{
    termEditor = new Console(parent);
    init();
}

void Terminal::init()
{
    QVBoxLayout *termLayout = new QVBoxLayout();
    termLayout->setContentsMargins(4,4,4,4);
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

    cbEchoOn = new QCheckBox(tr("Echo On"),this);
    connect(cbEchoOn,SIGNAL(clicked(bool)),this,SLOT(echoOnChange(bool)));

    options = new TermPrefs(this);
    setEchoOn(options->getEchoOn());

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
    butLayout->addWidget(&portLabel);
    portLabel.setFont(QFont("System", 14));
    butLayout->addWidget(cbEchoOn);
    butLayout->addWidget(buttonBox);
    setLayout(termLayout);
#if !defined(Q_OS_MAC)
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
#endif
    setWindowIcon(QIcon(":/images/console.png"));
    //resize(400,300); // just use default size
}

Console *Terminal::getEditor()
{
    return termEditor;
}

void Terminal::setPortListener(PortListener *listener)
{
    portListener = listener;
    if(listener->getPortName().isEmpty() == false)
        portLabel.setText(listener->getPortName());
    else
        portLabel.setText("");
}

QString Terminal::getPortName()
{
    return portLabel.text();
}

void Terminal::setPortName(QString name)
{
    portLabel.setText(name);
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
    // saving the baud rate is not currently working (dbetz)
    //options->saveBaudRate(baud);
}

BaudRateType Terminal::getBaud()
{
    int index = comboBoxBaud->currentIndex();
    QVariant var = comboBoxBaud->itemData(index);
    bool ok;
    int baud = var.toInt(&ok);
    if(ok) {
        return (BaudRateType) baud;
    }
    else {
        return BAUD115200;
    }
}

int Terminal::getBaudRate()
{
    return options->getBaudRate();
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

void Terminal::echoOnChange(bool value)
{
    termEditor->setEnableEchoOn(value);
    options->saveEchoOn(value);
}

void Terminal::setEchoOn(bool echoOn)
{
    termEditor->setEnableEchoOn(echoOn);
    cbEchoOn->setChecked(echoOn);
}

void Terminal::accept()
{
#ifdef TERM_ENABLE_BUTTON
    buttonEnable->setText("Disable");
#endif
    // save terminal geometry
    QSettings *settings = new QSettings(publisherKey, ASideGuiKey, this);
    if(settings->value(useKeys).toInt() == 1) {
        QByteArray geo = this->saveGeometry();
        settings->setValue(termGeometryKey,geo);
    }
    termEditor->setPortEnable(false);
    portLabel.setEnabled(false);
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
    portLabel.setEnabled(false);
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
        portLabel.setText(portListener->getPortName());
        portLabel.setEnabled(true);
        termEditor->setFocus(Qt::OtherFocusReason);
        emit disablePortCombo();
    }
    else {
        buttonEnable->setText("Enable");
        termEditor->setPortEnable(false);
        //portLabel.setText("");
        portLabel.setEnabled(false);
        portListener->close();
        emit enablePortCombo();
    }
    QApplication::processEvents();
#endif
}

/* don't open/close port here because different OS have different requirements.
 */
void Terminal::setPortEnabled(bool value)
{
    if(value) {
        QString plName = portListener->getPortName();
#ifdef TERM_ENABLE_BUTTON
        buttonEnable->setText("Disable");
#endif
        termEditor->setPortEnable(true);
        portLabel.setEnabled(true);
        lastConnectedPortName = plName;
        this->portLabel.setText(plName);
    }
    else {
#ifdef TERM_ENABLE_BUTTON
        buttonEnable->setText("Enable");
#endif
        termEditor->setPortEnable(false);
        portLabel.setEnabled(false);
        lastConnectedPortName = getPortName();
        this->portLabel.setText("NONE");
    }
    QApplication::processEvents();
}

QString Terminal::getLastConnectedPortName()
{
    return lastConnectedPortName;
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
