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

#include "ui_project.h"
#include "projectoptions.h"
#include "properties.h"
#include "asideconfig.h"

const QString ProjectOptions::compiler = "compiler";
const QString ProjectOptions::memtype = "memtype";
const QString ProjectOptions::memTypeLMM = "LMM Main RAM";
const QString ProjectOptions::memTypeCMM = "CMM Main RAM Compact";
const QString ProjectOptions::memTypeCOG = "COG Cog RAM";
const QString ProjectOptions::memTypeXMM = "XMM";
const QString ProjectOptions::memTypeXMMC = "XMMC External Flash Code Main RAM Data";
const QString ProjectOptions::memTypeXMMSINGLE = "XMM-SINGLE External RAM";
const QString ProjectOptions::memTypeXMMSPLIT = "XMM-SPLIT External Flash Code + RAM Data";
const QString ProjectOptions::optimization = "optimize";
const QString ProjectOptions::cflags = "defs";
const QString ProjectOptions::lflags = "linker";
const QString ProjectOptions::board = "BOARD";

const QString ProjectOptions::C_COMPILER = "C";
const QString ProjectOptions::CPP_COMPILER = "C++";
const QString ProjectOptions::SPIN_COMPILER = "SPIN";

ProjectOptions::ProjectOptions(QWidget *parent, QComboBox *boardType) : QWidget(parent), ui(new Ui::Project)
{
    ui->setupUi(this);

    if(boardType == NULL) {
        cbBoard = ui->hardwareComboBox;
    }
    else {
        cbBoard = boardType;
        ui->labelHardware->setVisible(false);
        ui->hardwareToolButton->setVisible(false);
        ui->hardwareComboBox->setVisible(false);

        QRect rect = ui->comboBoxCompiler->geometry();
        int height = rect.height();
        rect.setTop(rect.top()-height);
        rect.setBottom(rect.bottom()-height);
        ui->comboBoxCompiler->setGeometry(rect);

        rect = ui->labelcompiler->geometry();
        height = rect.height();
        rect.setTop(rect.top()-height);
        rect.setBottom(rect.bottom()-height);
        ui->labelcompiler->setGeometry(rect);

        rect = ui->comboBoxMemoryMode->geometry();
        height = rect.height();
        rect.setTop(rect.top()-height);
        rect.setBottom(rect.bottom()-height);
        ui->comboBoxMemoryMode->setGeometry(rect);

        rect = ui->labelmem->geometry();
        height = rect.height();
        rect.setTop(rect.top()-height);
        rect.setBottom(rect.bottom()-height);
        ui->labelmem->setGeometry(rect);

        rect = ui->comboBoxOptimization->geometry();
        height = rect.height();
        rect.setTop(rect.top()-height);
        rect.setBottom(rect.bottom()-height);
        ui->comboBoxOptimization->setGeometry(rect);

        rect = ui->labelOptimization->geometry();
        height = rect.height();
        rect.setTop(rect.top()-height);
        rect.setBottom(rect.bottom()-height);
        ui->labelOptimization->setGeometry(rect);
    }

    ui->comboBoxCompiler->addItem(ProjectOptions::C_COMPILER);
    ui->comboBoxCompiler->addItem(ProjectOptions::CPP_COMPILER);
#ifdef SPIN
    ui->comboBoxCompiler->addItem(ProjectOptions::SPIN_COMPILER);
#endif
    tabs.append(ui->tabWidget->widget(this->TAB_OPT));
    tabs.append(ui->tabWidget->widget(this->TAB_C_COMP));
    tabs.append(ui->tabWidget->widget(this->TAB_C_LIB));
    tabs.append(ui->tabWidget->widget(this->TAB_SPIN_COMP));

    connect(ui->comboBoxCompiler,SIGNAL(currentIndexChanged(QString)), this, SLOT(compilerChanged(QString)));

    compilerChanged("");
    compilerChanged(ui->comboBoxCompiler->currentText());

    QFile file("memorymodels.txt");
    if(file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QString s = file.readAll();
        QStringList list = s.split("\n",QString::SkipEmptyParts);
        for(int n = 0; n < list.length(); n++)
            ui->comboBoxMemoryMode->addItem(list.at(n));
        file.close();
    }
    else {
        ui->comboBoxMemoryMode->addItem(memTypeLMM);
        ui->comboBoxMemoryMode->addItem(memTypeCMM);
        ui->comboBoxMemoryMode->addItem(memTypeCOG);
        ui->comboBoxMemoryMode->addItem(memTypeXMMC);
        ui->comboBoxMemoryMode->addItem(memTypeXMMSINGLE);
        ui->comboBoxMemoryMode->addItem(memTypeXMMSPLIT);
    }

    ui->comboBoxOptimization->addItem("-Os Size");
    ui->comboBoxOptimization->addItem("-O2 Speed");
    ui->comboBoxOptimization->addItem("-O1 Mixed");
    ui->comboBoxOptimization->addItem("-O0 None");

    ui->checkBox32bitDouble->setChecked(true);
    ui->checkBoxWarnAll->setChecked(false);
    ui->checkBoxNoFcache->setChecked(false);
    ui->checkBoxExceptions->setChecked(false);
    ui->checkBoxTinylib->setChecked(false);
    ui->checkBoxMathlib->setChecked(false);
    ui->checkBoxPthreadLib->setChecked(false);
    ui->checkBoxSimplePrintf->setChecked(false);
    ui->checkBoxStripELF->setChecked(false);
    ui->checkBoxMakeLibrary->setChecked(false);
    ui->checkBoxEnableGc->setChecked(false);

    // keep strip and no exceptions but hide them
    ui->checkBoxStripELF->setVisible(false);
    //ui->checkBoxExceptions->setVisible(false);

    // turn this on when Ted commits tiny lib
    // Tiny Lib has proven more trouble than it's worth for simple users.
    // but turning it off is trouble too.
    // ui->checkBoxTinylib->setVisible(false);

    // start with project options
    ui->tabWidget->setCurrentIndex(0);
}

ProjectOptions::~ProjectOptions()
{
}

void ProjectOptions::compilerChanged(QString comp)
{
    for(int n = ui->tabWidget->count()-1; n > 0; n--)
        ui->tabWidget->removeTab(n);
    if(comp.contains("C",Qt::CaseInsensitive)) {
        ui->tabWidget->addTab(tabs.at(this->TAB_C_COMP),tr("Compiler"));
        ui->tabWidget->addTab(tabs.at(this->TAB_C_LIB),tr("Linker"));
        ui->comboBoxMemoryMode->setEnabled(true);
        ui->comboBoxOptimization->setEnabled(true);
    }
    else if(comp.contains("SPIN",Qt::CaseInsensitive)) {
        ui->tabWidget->addTab(tabs.at(this->TAB_SPIN_COMP),tr("Compiler"));
        ui->comboBoxMemoryMode->setEnabled(false);
        ui->comboBoxOptimization->setEnabled(false);
    }
    emit compilerChanged();
}

void ProjectOptions::clearOptions()
{
    ui->comboBoxCompiler->setCurrentIndex(0);
    ui->comboBoxMemoryMode->setCurrentIndex(0);
    ui->comboBoxOptimization->setCurrentIndex(0);
    ui->checkBox32bitDouble->setChecked(false);
    ui->checkBoxWarnAll->setChecked(false);
    ui->checkBoxNoFcache->setChecked(false);
    ui->checkBoxExceptions->setChecked(false);
    ui->checkBoxTinylib->setChecked(false);
    ui->checkBoxMathlib->setChecked(false);
    ui->checkBoxPthreadLib->setChecked(false);
    ui->checkBoxSimplePrintf->setChecked(false);
    ui->checkBoxStripELF->setChecked(false);
    ui->checkBoxMakeLibrary->setChecked(false);
    ui->checkBoxEnableGc->setChecked(false);
    ui->lineEditCompOptions->setText("");
    ui->lineEditLinkOptions->setText("");
    ui->lineEditSpinCompOptions->setText("");
}

QToolButton *ProjectOptions::getHardwareButton()
{
    return ui->hardwareToolButton;
}

QComboBox   *ProjectOptions::getHardwareComboBox()
{
    return cbBoard;
}

QStringList ProjectOptions::getOptions()
{
    QStringList args;

    //portName = cbPort->itemText(cbPort->currentIndex());
    //boardName = cbBoard->itemText(cbBoard->currentIndex());

    args.append(compiler+"="+getCompiler());
    args.append(memtype+"="+getMemModel());
    args.append(optimization+"="+getOptimization());

    if(get32bitDoubles().length())
        args.append(get32bitDoubles());
    if(getWarnAll().length())
        args.append(getWarnAll());
    if(getNoFcache().length())
        args.append(getNoFcache());
    if(getExceptions().length())
        args.append(getExceptions());
    if(getSimplePrintf().length())
        args.append(getSimplePrintf());
    if(getEnableGcSections().length())
        args.append(getEnableGcSections());

    if(getCompiler().indexOf("++") > -1)
        args.append("-fno-rtti");

    /* other compiler options */
    if(getCompOptions().length())
        args.append(cflags+"::"+getCompOptions());

    /* libraries */
    if(getTinyLib().length())
        args.append(getTinyLib());
    if(getMathLib().length())
        args.append(getMathLib());
    if(getPthreadLib().length())
        args.append(getPthreadLib());

    if(getMakeLibrary().length())
        args.append(getMakeLibrary());

    /* other linker options */
    if(getLinkOptions().length())
        args.append(lflags+"::"+getLinkOptions());

    /* strip */
    if(getStripElf().length())
        args.append(getStripElf());

    /* board type */
    if(getBoardType().length())
        args.append(board+"::"+getBoardType());
    else
        args.append(board+"::");

    //qDebug() << args;
    return args;
}

QStringList ProjectOptions::getMemModelList()
{
    QStringList list;
    QString item;
    QVariant sv;
    int len = ui->comboBoxMemoryMode->count();

    for(int n = 0; n < len; n++) {
        sv = ui->comboBoxMemoryMode->itemData(n,Qt::DisplayRole);
        if(sv.canConvert(QVariant::String)) {
            item = sv.toString();
            item = item.mid(0, item.indexOf(" "));
            list.append(item);
        }
    }
    return list;
}

QString  ProjectOptions::getCompiler()
{
    return ui->comboBoxCompiler->currentText();
}
QString  ProjectOptions::getMemModel()
{
    return ui->comboBoxMemoryMode->currentText().toLower();
}
QString  ProjectOptions::getOptimization()
{
    QStringList opts = ui->comboBoxOptimization->currentText().split(" ");
    return opts[0];
}

QString  ProjectOptions::get32bitDoubles()
{
    return ui->checkBox32bitDouble->isChecked() ? QString ("-m32bit-doubles") : QString ("");
}
QString  ProjectOptions::getWarnAll()
{
    return ui->checkBoxWarnAll->isChecked() ? QString ("-Wall") : QString ("");
}
QString  ProjectOptions::getNoFcache()
{
    return ui->checkBoxNoFcache->isChecked() ? QString ("-mno-fcache") : QString ("");
}
QString  ProjectOptions::getExceptions()
{
    return ui->checkBoxExceptions->isChecked() ? QString ("-fexceptions") : QString ("-fno-exceptions");
}
QString  ProjectOptions::getTinyLib()
{
    return ui->checkBoxTinylib->isChecked() ? QString ("-ltiny") : QString ("");
}
QString  ProjectOptions::getMathLib()
{
    return ui->checkBoxMathlib->isChecked() ? QString ("-lm") : QString ("");
}
QString  ProjectOptions::getPthreadLib()
{
    return ui->checkBoxPthreadLib->isChecked() ? QString ("-lpthread") : QString ("");
}
QString  ProjectOptions::getSimplePrintf()
{
    return ui->checkBoxSimplePrintf->isChecked() ? QString ("-Dprintf=__simple_printf") : QString ("");
}

QString  ProjectOptions::getStripElf()
{
    return QString (""); // never strip now that we have config variable patching
}

QString  ProjectOptions::getCompOptions()
{
    return ui->lineEditCompOptions->text().toLatin1();
}
QString  ProjectOptions::getLinkOptions()
{
    return ui->lineEditLinkOptions->text().toLatin1();
}
QString  ProjectOptions::getBoardType()
{
    return boardType;
}
QString ProjectOptions::getMakeLibrary()
{
    return ui->checkBoxMakeLibrary->isChecked() ? QString ("-create_library") : QString("");
}
QString ProjectOptions::getEnableGcSections()
{
    return ui->checkBoxEnableGc->isChecked() ? QString ("-enable_pruning") : QString("");
}

void ProjectOptions::setOptions(QString s)
{
    if(s.at(0) != '>')
        return;

    s = s.mid(s.lastIndexOf('>')+1);
    if(s.at(0) != '-') {

        QStringList flags = s.split("::");
        QStringList arr = s.split('=');
        QString name;
        QString value;

        // handle flags in the form of "flag::any string"
        if(flags.length() > 1) {
            name = flags[0];
            value = flags[1];
            if(name.compare(cflags,Qt::CaseInsensitive) == 0) {
                this->setCompOptions(value);
            }
            else
            if(name.compare(lflags,Qt::CaseInsensitive) == 0) {
                this->setLinkOptions(value);
            }
            else
            if(name.compare(board,Qt::CaseInsensitive) == 0) {
                this->setBoardType(value);
            }
        }
        else // handle parameters as "name=value"
        if(arr.length() > 1) {
            name = arr[0];
            value = arr[1];
            if(name.compare(compiler) == 0) {
                this->setCompiler(value);
            }
            else
            if(name.compare(memtype) == 0) {
                this->setMemModel(value);
            }
            else
            if(name.compare(optimization) == 0) {
                this->setOptimization(value);
            }
        }
    }
    else {
        if(s.contains("32bit")) {
            set32bitDoubles(true);
        }
        else
        if(s.contains("-Wall")) {
            setWarnAll(true);
        }
        else
        if(s.contains("no-fcache")) {
            setNoFcache(true);
        }
        else
        if(s.contains("fexception")) {
            setExceptions(true);
        }
        else
        if(s.contains("ltiny")) {
            setTinyLib(true);
        }
        else
        if(s.contains("lm")) {
            setMathLib(true);
        }
        else
        if(s.contains("lpthread")) {
            setPthreadLib(true);
        }
        else
        if(s.contains("simple_printf")) {
            setSimplePrintf(true);
        }
        else
        if(s.contains("create_library")) {
            setMakeLibrary(true);
        }
        else
        if(s.contains("enable_pruning")) {
            setEnableGcSections(true);
        }
    }
}

void ProjectOptions::setCompiler(QString s)
{
    int n = ui->comboBoxCompiler->count();
    while(--n > -1) {
        QString op = ui->comboBoxCompiler->itemText(n);
        if(op.compare(s, Qt::CaseInsensitive) == 0) {
            ui->comboBoxCompiler->setCurrentIndex(n);
            break;
        }
    }
}
void ProjectOptions::setMemModel(QString s)
{
    int n = ui->comboBoxMemoryMode->count();
    while(--n > -1) {
        QString op = ui->comboBoxMemoryMode->itemText(n);
        if(s.length() < op.length() && op.mid(s.length(),1).at(0) == ' ') {
            op = op.mid(0,s.length());
        }
        if(op.compare(s, Qt::CaseInsensitive) == 0) {
            ui->comboBoxMemoryMode->setCurrentIndex(n);
            break;
        }
    }
}
void ProjectOptions::setOptimization(QString s)
{
    int n = ui->comboBoxOptimization->count();
    while(--n > -1) {
        QString op = ui->comboBoxOptimization->itemText(n);
        if(op.contains(s, Qt::CaseInsensitive)) {
            ui->comboBoxOptimization->setCurrentIndex(n);
            break;
        }
    }
}
void ProjectOptions::set32bitDoubles(bool s)
{
    ui->checkBox32bitDouble->setChecked(s);
}
void ProjectOptions::setWarnAll(bool s)
{
    ui->checkBoxWarnAll->setChecked(s);
}
void ProjectOptions::setNoFcache(bool s)
{
    ui->checkBoxNoFcache->setChecked(s);
}
void ProjectOptions::setExceptions(bool s)
{
    ui->checkBoxExceptions->setChecked(s);
}
void ProjectOptions::setTinyLib(bool s)
{
    ui->checkBoxTinylib->setChecked(s);
}
void ProjectOptions::setMathLib(bool s)
{
    ui->checkBoxMathlib->setChecked(s);
}
void ProjectOptions::setPthreadLib(bool s)
{
    ui->checkBoxPthreadLib->setChecked(s);
}
void ProjectOptions::setSimplePrintf(bool s)
{
    ui->checkBoxSimplePrintf->setChecked(s);
}
void ProjectOptions::setStripElf(bool s)
{
    if(s == true) s = false;  // never strip because loader needs symbols
    ui->checkBoxStripELF->setChecked(s);
}
void ProjectOptions::setCompOptions(QString s)
{
    ui->lineEditCompOptions->setText(s);
}
void ProjectOptions::setLinkOptions(QString s)
{
    ui->lineEditLinkOptions->setText(s);
}
void ProjectOptions::setBoardType(QString s)
{
    boardType = s.trimmed();
}
void ProjectOptions::setMakeLibrary(bool s)
{
    ui->checkBoxMakeLibrary->setChecked(s);
}
void ProjectOptions::setEnableGcSections(bool s)
{
    ui->checkBoxEnableGc->setChecked(s);
}

QStringList ProjectOptions::getSpinOptions()
{
    QStringList args;

    args.append(compiler+"="+getCompiler());

    /* other compiler options */
    if(getSpinCompOptions().length())
        args.append(cflags+"::"+getSpinCompOptions());

    return args;
}

QString  ProjectOptions::getSpinCompOptions()
{
    return ui->lineEditSpinCompOptions->text().toLatin1();
}

void ProjectOptions::setSpinOptions(QString s)
{
    if(s.at(0) != '>')
        return;

    s = s.mid(s.lastIndexOf('>')+1);
    if(s.at(0) != '-') {

        QStringList flags = s.split("::");
        QStringList arr = s.split('=');
        QString name;
        QString value;

        // handle flags in the form of "flag::any string"
        if(flags.length() > 1) {
            name = flags[0];
            value = flags[1];
            if(name.compare(cflags,Qt::CaseInsensitive) == 0) {
                this->setSpinCompOptions(value);
            }
        }
        else // handle parameters as "name=value"
        if(arr.length() > 1) {
            name = arr[0];
            value = arr[1];
            if(name.compare(compiler) == 0) {
                this->setCompiler(value);
            }
        }
    }
}

void ProjectOptions::setSpinCompOptions(QString s)
{
    ui->lineEditSpinCompOptions->setText(s);
}

void ProjectOptions::setMaximumWidth(int width)
{
    QWidget::setMaximumWidth(width);
    ui->tabWidget->setMaximumWidth(width);
}

void ProjectOptions::setMinimumWidth(int width)
{
    QWidget::setMinimumWidth(width);
    ui->tabWidget->setMinimumWidth(width);
}

