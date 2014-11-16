/*
 * This file is part of the Parallax Propeller SimpleIDE development environment.
 *
 * Copyright (C) 2014 Parallax Incorporated
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hardware.h"
#include "properties.h"
#include "asideconfig.h"

Hardware::Hardware(QWidget *parent) : QDialog(parent), ui(new Ui::Hardware)
{
    aSideConfig = new ASideConfig();

    ui->setupUi(this);

    connect(ui->comboBoxBoard,SIGNAL(activated(int)),this,SLOT(boardChanged(int)));
    connect(ui->pushButtonDelete,SIGNAL(clicked()),this,SLOT(deleteBoard()));

    ui->comboBoxBoard->setEditable(true);

    ui->tabWidget->setCurrentIndex(0);

    ui->comboBoxClkMode->addItem(tr("RCFAST"));
    ui->comboBoxClkMode->addItem(tr("RCSLOW"));
    ui->comboBoxClkMode->addItem(tr("XINPUT"));
    ui->comboBoxClkMode->addItem(tr("XTAL1+PLL1X"));
    ui->comboBoxClkMode->addItem(tr("XTAL1+PLL2X"));
    ui->comboBoxClkMode->addItem(tr("XTAL1+PLL4X"));
    ui->comboBoxClkMode->addItem(tr("XTAL1+PLL8X"));
    ui->comboBoxClkMode->addItem(tr("XTAL1+PLL16X"));
    ui->comboBoxClkMode->addItem(tr("XTAL2+PLL1X"));
    ui->comboBoxClkMode->addItem(tr("XTAL2+PLL2X"));
    ui->comboBoxClkMode->addItem(tr("XTAL2+PLL4X"));
    ui->comboBoxClkMode->addItem(tr("XTAL2+PLL8X"));
    ui->comboBoxClkMode->addItem(tr("XTAL2+PLL16X"));
    ui->comboBoxClkMode->addItem(tr("XTAL3+PLL1X"));
    ui->comboBoxClkMode->addItem(tr("XTAL3+PLL2X"));
    ui->comboBoxClkMode->addItem(tr("XTAL3+PLL4X"));
    ui->comboBoxClkMode->addItem(tr("XTAL3+PLL8X"));
    ui->comboBoxClkMode->addItem(tr("XTAL3+PLL16X"));

    ui->comboBoxBaudrate->addItem(tr("9600"));
    ui->comboBoxBaudrate->addItem(tr("19200"));
    ui->comboBoxBaudrate->addItem(tr("38400"));
    ui->comboBoxBaudrate->addItem(tr("56000"));
    ui->comboBoxBaudrate->addItem(tr("115200"));

    ui->comboBoxDataMemory->addItem(tr("HUB"));
    ui->comboBoxDataMemory->addItem(tr("RAM"));

    ui->comboBoxCodeMemory->addItem(tr("HUB"));
    ui->comboBoxCodeMemory->addItem(tr("RAM"));
    ui->comboBoxCodeMemory->addItem(tr("FLASH"));

    ui->comboBoxCacheSize->addItem(tr(""));
    ui->comboBoxCacheSize->addItem(tr("2K"));
    ui->comboBoxCacheSize->addItem(tr("4K"));
    ui->comboBoxCacheSize->addItem(tr("8K"));

    ui->comboBoxExternalFlashSize->addItem(tr(""));
    ui->comboBoxExternalFlashSize->addItem(tr("16M"));
    ui->comboBoxExternalFlashSize->addItem(tr("8M"));
    ui->comboBoxExternalFlashSize->addItem(tr("4M"));
    ui->comboBoxExternalFlashSize->addItem(tr("2M"));
    ui->comboBoxExternalFlashSize->addItem(tr("1M"));
    ui->comboBoxExternalFlashSize->addItem(tr("512K"));
    ui->comboBoxExternalFlashSize->addItem(tr("256K"));
    ui->comboBoxExternalFlashSize->setEditable(true);

    ui->comboBoxExternalRamSize->addItem(tr(""));
    ui->comboBoxExternalRamSize->addItem(tr("32M"));
    ui->comboBoxExternalRamSize->addItem(tr("16M"));
    ui->comboBoxExternalRamSize->addItem(tr("8M"));
    ui->comboBoxExternalRamSize->addItem(tr("4M"));
    ui->comboBoxExternalRamSize->addItem(tr("2M"));
    ui->comboBoxExternalRamSize->addItem(tr("1M"));
    ui->comboBoxExternalRamSize->addItem(tr("512K"));
    ui->comboBoxExternalRamSize->addItem(tr("256K"));
    ui->comboBoxExternalRamSize->addItem(tr("64K"));
    ui->comboBoxExternalRamSize->addItem(tr("32K"));
    ui->comboBoxExternalRamSize->setEditable(true);

    ui->lineEditClkFreq->setText(tr("80000000"));
    ui->lineEditRxPin->setText(tr("31"));
    ui->lineEditTxPin->setText(tr("30"));
    ui->lineEditTvPin->setText(tr("12"));
}

Hardware::~Hardware()
{
    delete ui;
}

/*
 * load board from aSide config file using aSideConfig
 */
void Hardware::loadBoards()
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant sv = settings.value(configFileKey);
    if(sv.canConvert(QVariant::String))
        aSideCfgFile = sv.toString();
    sv = settings.value(separatorKey);
    if(sv.canConvert(QVariant::String))
        aSideSeparator = sv.toString();
    if(aSideConfig->loadBoards(aSideCfgFile) == 0)
        return;

    /* get board types */
    QStringList boards = aSideConfig->getBoardNames();
    ui->comboBoxBoard->clear();
    for(int n = 0; n < boards.count(); n++) {
        ui->comboBoxBoard->addItem(boards.at(n));
    }

    /* load first board info */
    ASideBoard *brd = aSideConfig->getBoardData(ui->comboBoxBoard->itemText(0));
    setDialogBoardInfo(brd);
}

/*
 * 1. copy old config file to backup
 * 2. save boards to new config file
 * 3. then copy new to old config file
 */
void Hardware::saveBoards()
{
    QString back = +".backup";
    QMessageBox mbox(QMessageBox::Critical,tr("Config File Error"),"", QMessageBox::Ok);
    QFile file;
    if(!file.exists(aSideCfgFile)) {
        mbox.setInformativeText(tr("Can't find file: ")+aSideCfgFile);
        mbox.exec();
        return;
    }
    QFile cfg(aSideCfgFile);
/*
    QFile bup(this->aSideCfgFile+back);
    if(bup.exists()) bup.remove();
    if(!cfg.copy(this->aSideCfgFile+back)) {
        mbox.setInformativeText(tr("Can't backup file: ")+aSideCfgFile);
        mbox.exec();
        return;
    }
*/
    QByteArray barry = "";
    QString currentName = ui->comboBoxBoard->currentText().toUpper();
    ASideBoard *board = aSideConfig->getBoardByName(currentName);
    if(board == NULL)
        ui->comboBoxBoard->addItem(currentName);
    for(int n = 0; n < ui->comboBoxBoard->count(); n++)
    {
        QString name = ui->comboBoxBoard->itemText(n);
        ASideBoard *board = aSideConfig->getBoardByName(name);
        if(board == NULL && name == currentName)
            board = aSideConfig->newBoard(name);
        if(name == currentName)
            setBoardInfo(board);
        QString ba = board->getFormattedConfig();
        qDebug() << ba;
        barry.append(ba);
    }


    QMessageBox sbox(QMessageBox::Question,tr("Saving Config File"),"",
                     QMessageBox::Save | QMessageBox::Cancel);

    sbox.setInformativeText(tr("Save new ")+aSideCfgFile+tr("?"));

    if(sbox.exec() == QMessageBox::Save) {
        if(cfg.open(QIODevice::WriteOnly | QFile::Text)) {
            cfg.write(barry);
            cfg.flush();
            cfg.close();
        }
    }
}

void Hardware::accept()
{
    saveBoards();
    done(QDialog::Accepted);
}

void Hardware::reject()
{
    done(QDialog::Rejected);
}

void Hardware::deleteBoard()
{
    aSideConfig->deleteBoardByName(ui->comboBoxBoard->currentText());
    ui->comboBoxBoard->removeItem(ui->comboBoxBoard->currentIndex());
}

void Hardware::boardChanged(int index)
{
    ASideBoard *brd = aSideConfig->getBoardData(ui->comboBoxBoard->itemText(index));
    setDialogBoardInfo(brd);
}

void Hardware::setBoardInfo(ASideBoard *board)
{
    board->set(ASideBoard::clkfreq,ui->lineEditClkFreq->text());

    if(ui->lineEditCacheDriver->text().length() > 0)
        board->set(ASideBoard::cachedriver,ui->lineEditCacheDriver->text());
    if(ui->lineEditCacheParm1->text().length() > 0)
        board->set(ASideBoard::cacheparam1,ui->lineEditCacheParm1->text());
    if(ui->lineEditCacheParm2->text().length() > 0)
        board->set(ASideBoard::cacheparam2,ui->lineEditCacheParm2->text());

    board->set(ASideBoard::rxpin,ui->lineEditRxPin->text());
    board->set(ASideBoard::txpin,ui->lineEditTxPin->text());

    if(ui->lineEditTvPin->text().length() > 0)
        board->set(ASideBoard::tvpin,ui->lineEditTvPin->text());

    board->set(ASideBoard::clkmode, ui->comboBoxClkMode->currentText());
    board->set(ASideBoard::baudrate, ui->comboBoxBaudrate->currentText());
    board->set(ASideBoard::textseg, ui->comboBoxCodeMemory->currentText());
    board->set(ASideBoard::dataseg, ui->comboBoxDataMemory->currentText());
    if(ui->comboBoxCacheSize->currentText().length() > 0)
        board->set(ASideBoard::cachesize, ui->comboBoxCacheSize->currentText());
    if(ui->comboBoxExternalFlashSize->currentText().length() > 0)
        board->set(ASideBoard::flashsize, ui->comboBoxExternalFlashSize->currentText());
    if(ui->comboBoxExternalRamSize->currentText().length() > 0)
        board->set(ASideBoard::ramsize, ui->comboBoxExternalRamSize->currentText());
}

void Hardware::setDialogBoardInfo(ASideBoard *board)
{
    ui->lineEditClkFreq->setText(board->get(ASideBoard::clkfreq));
    ui->lineEditRxPin->setText(board->get(ASideBoard::rxpin));
    ui->lineEditTxPin->setText(board->get(ASideBoard::txpin));
    ui->lineEditTvPin->setText(board->get(ASideBoard::tvpin));

    QString cds = board->get(ASideBoard::cachedriver);
    QString cp1 = board->get(ASideBoard::cacheparam1);
    QString cp2 = board->get(ASideBoard::cacheparam2);

    if(cds.length() > 0)
        ui->lineEditCacheDriver->setText(board->get(ASideBoard::cachedriver));
    if(cp1.length() > 0)
        ui->lineEditCacheParm1->setText(board->get(ASideBoard::cacheparam1));
    if(cp2.length() > 0)
        ui->lineEditCacheParm2->setText(board->get(ASideBoard::cacheparam2));

    QString clkmode = board->get(ASideBoard::clkmode);
    QString baudrate = board->get(ASideBoard::baudrate);
    QString textseg = board->get(ASideBoard::textseg);
    QString dataseg = board->get(ASideBoard::dataseg);
    QString cachesize = board->get(ASideBoard::cachesize);
    QString flashsize = board->get(ASideBoard::flashsize);
    QString ramsize = board->get(ASideBoard::ramsize);

    setComboCurrent(ui->comboBoxClkMode,clkmode);
    setComboCurrent(ui->comboBoxBaudrate,baudrate);
    setComboCurrent(ui->comboBoxCodeMemory,textseg);
    setComboCurrent(ui->comboBoxDataMemory,dataseg);
    setComboCurrent(ui->comboBoxCacheSize,cachesize);
    setComboCurrent(ui->comboBoxExternalFlashSize,flashsize);
    setComboCurrent(ui->comboBoxExternalRamSize,ramsize);
}

void Hardware::setComboCurrent(QComboBox *cb, QString value)
{
    for(int n = cb->count()-1; n > -1; n--) {
        QString s = cb->itemText(n);
        if(s == value) {
            cb->setCurrentIndex(n);
            break;
        }
    }
}
