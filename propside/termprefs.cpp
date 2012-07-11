#include "termprefs.h"
#include "ui_TermPrefs.h"

TermPrefs::TermPrefs(Console *con) : ui(new Ui::TermPrefs)
{
    ui->setupUi(this);
    serialConsole = con;

    /* setup application registry info */
    QCoreApplication::setOrganizationName(publisherKey);
    QCoreApplication::setOrganizationDomain(publisherComKey);
    QCoreApplication::setApplicationName(ASideGuiKey);

    /* global settings */
    settings = new QSettings(QString(publisherKey), QString(ASideGuiKey), this);

    settingNames.clear();
    settingNames.append(enableKeyClearScreen);
    settingNames.append(enableKeyHomeCursor);
    settingNames.append(enableKeyPosXYCursor);
    settingNames.append(enableKeyMoveCursorLeft);
    settingNames.append(enableKeyMoveCursorRight);
    settingNames.append(enableKeyMoveCursorUp);
    settingNames.append(enableKeyMoveCursorDown);
    settingNames.append(enableKeyBeepSpeaker);
    settingNames.append(enableKeyBackspace);
    settingNames.append(enableKeyTab);
    settingNames.append(enableKeyCReturn);
    settingNames.append(enableKeyClearToEOL);
    settingNames.append(enableKeyClearLinesBelow);
    settingNames.append(enableKeyNewLine);
    settingNames.append(enableKeyPosCursorX);
    settingNames.append(enableKeyPosCursorY);

    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
    connect(ui->buttonBox, SIGNAL(accepted()),this,SLOT(accept()));

    readSettings();
}

TermPrefs::~TermPrefs()
{
    delete settings;
}

void TermPrefs::clearAll()
{

}

/*
 * get all options and save to settings
 */
void TermPrefs::saveSettings()
{
    bool enable[16];
    bool enableSwap;
    int len = settingNames.count();

    int j = 0;
    enable[j] = ui->cbClearScreen->isChecked();
    serialConsole->setEnableClearScreen(enable[j++]);

    enable[j] = ui->cbHomeCursor->isChecked();
    serialConsole->setEnableHomeCursor(enable[j++]);

    enable[j] = ui->cbPositionCursorXY->isChecked();
    serialConsole->setEnablePosXYCursor(enable[j++]);

    enable[j] = ui->cbMoveLeft->isChecked();
    serialConsole->setEnableMoveCursorLeft(enable[j++]);

    enable[j] = ui->cbMoveRight->isChecked();
    serialConsole->setEnableMoveCursorRight(enable[j++]);

    enable[j] = ui->cbMoveUp->isChecked();
    serialConsole->setEnableMoveCursorUp(enable[j++]);

    enable[j] = ui->cbMoveDown->isChecked();
    serialConsole->setEnableMoveCursorDown(enable[j++]);

    enable[j] = ui->cbBeepSpeaker->isChecked();
    serialConsole->setEnableBeepSpeaker(enable[j++]);

    enable[j] = ui->cbBackSpace->isChecked();
    serialConsole->setEnableBackspace(enable[j++]);

    enable[j] = ui->cbTab->isChecked();
    serialConsole->setEnableTab(enable[j++]);

    enable[j] = ui->cbCReturn->isChecked();
    serialConsole->setEnableCReturn(enable[j++]);

    enable[j] = ui->cbClearToEOL->isChecked();
    serialConsole->setEnableClearToEOL(enable[j++]);

    enable[j] = ui->cbClearLinesBelow->isChecked();
    serialConsole->setEnableClearLinesBelow(enable[j++]);

    enable[j] = ui->cbNewLine->isChecked();
    serialConsole->setEnableNewLine(enable[j++]);

    enable[j] = ui->cbPositionCursorX->isChecked();
    serialConsole->setEnablePosCursorX(enable[j++]);

    enable[j] = ui->cbPositionCursorY->isChecked();
    serialConsole->setEnablePosCursorY(enable[j++]);

    /* now save settings
     */
    for(int n = 0; n < len; n++) {
        QString key(settingNames[n]);
        settings->setValue(key,enable[n]);
    }

    enableSwap = ui->cbSwapNLCR->checkState();
    serialConsole->setEnableSwapNLCR(enableSwap);
    settings->setValue(enableKeySwapNLCR,enableSwap);
}

/*
 * read all settings and save to enable options
 */
void TermPrefs::readSettings()
{
    bool enable[16];
    bool enableSwap;

    int len = settingNames.count();

    QVariant var;
    for(int n = 0; n < len; n++) {
        QString key(settingNames[n]);
        var = settings->value(key,QVariant(false));
        if(var.canConvert(QVariant::Bool)) {
            bool val = var.toBool();
            enable[n] = val;
        }
        else {
            enable[n] = false;
        }
    }

    int j = 0;
    ui->cbClearScreen->setChecked(enable[j]);
    serialConsole->setEnableClearScreen(enable[j++]);

    ui->cbHomeCursor->setChecked(enable[j]);
    serialConsole->setEnableHomeCursor(enable[j++]);

    ui->cbPositionCursorXY->setChecked(enable[j]);
    serialConsole->setEnablePosXYCursor(enable[j++]);

    ui->cbMoveLeft->setChecked(enable[j]);
    serialConsole->setEnableMoveCursorLeft(enable[j++]);

    ui->cbMoveRight->setChecked(enable[j]);
    serialConsole->setEnableMoveCursorRight(enable[j++]);

    ui->cbMoveUp->setChecked(enable[j]);
    serialConsole->setEnableMoveCursorUp(enable[j++]);

    ui->cbMoveDown->setChecked(enable[j]);
    serialConsole->setEnableMoveCursorDown(enable[j++]);

    ui->cbBeepSpeaker->setChecked(enable[j]);
    serialConsole->setEnableBeepSpeaker(enable[j++]);

    ui->cbBackSpace->setChecked(enable[j]);
    serialConsole->setEnableBackspace(enable[j++]);

    ui->cbTab->setChecked(enable[j]);
    serialConsole->setEnableTab(enable[j++]);

    ui->cbCReturn->setChecked(enable[j]);
    serialConsole->setEnableCReturn(enable[j++]);

    ui->cbClearToEOL->setChecked(enable[j]);
    serialConsole->setEnableClearToEOL(enable[j++]);

    ui->cbClearLinesBelow->setChecked(enable[j]);
    serialConsole->setEnableClearLinesBelow(enable[j++]);

    ui->cbNewLine->setChecked(enable[j]);
    serialConsole->setEnableNewLine(enable[j++]);

    ui->cbPositionCursorX->setChecked(enable[j]);
    serialConsole->setEnablePosCursorX(enable[j++]);

    ui->cbPositionCursorY->setChecked(enable[j]);
    serialConsole->setEnablePosCursorY(enable[j++]);

    var = settings->value(enableKeySwapNLCR,QVariant(false));
    if(var.canConvert(QVariant::Bool)) {
        bool val = var.toBool();
        enableSwap = val;
    }
    else {
        enableSwap = false;
    }
    ui->cbSwapNLCR->setChecked(enableSwap);
    serialConsole->setEnableSwapNLCR(enableSwap);

}

void TermPrefs::showDialog()
{
    readSettings();
    this->show();
}

void TermPrefs::accept()
{
    saveSettings();
    this->hide();
}

void TermPrefs::reject()
{
    readSettings();
    this->hide();
}
