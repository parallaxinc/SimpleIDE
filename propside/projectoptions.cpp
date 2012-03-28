#include "projectoptions.h"
#include "ui_project.h"
#include "properties.h"
#include "asideconfig.h"

const QString ProjectOptions::compiler = "compiler";
const QString ProjectOptions::memtype = "memtype";
const QString ProjectOptions::memTypeLMM = "LMM";
const QString ProjectOptions::memTypeCOG = "COG";
const QString ProjectOptions::memTypeXMM = "XMM";
const QString ProjectOptions::memTypeXMMC = "XMMC";
const QString ProjectOptions::memTypeXMMSINGLE = "XMM-SINGLE";
const QString ProjectOptions::memTypeXMMSPLIT = "XMM-SPLIT";
const QString ProjectOptions::optimization = "optimize";
const QString ProjectOptions::cflags = "defs";
const QString ProjectOptions::lflags = "linker";
const QString ProjectOptions::board = "BOARD";

ProjectOptions::ProjectOptions(QWidget *parent) : QWidget(parent), ui(new Ui::Project)
{
    ui->setupUi(this);

    ui->comboBoxCompiler->addItem("C");
    ui->comboBoxCompiler->addItem("C++");

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
        ui->comboBoxMemoryMode->addItem(memTypeCOG);
        ui->comboBoxMemoryMode->addItem(memTypeXMM);
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
    ui->checkBoxMathlib->setChecked(false);
    ui->checkBoxPthreadLib->setChecked(false);
    ui->checkBoxSimplePrintf->setChecked(true);
    ui->checkBoxStripELF->setChecked(true);
}

ProjectOptions::~ProjectOptions()
{
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
    ui->checkBoxMathlib->setChecked(false);
    ui->checkBoxPthreadLib->setChecked(false);
    ui->checkBoxSimplePrintf->setChecked(false);
    ui->checkBoxStripELF->setChecked(false);
    ui->lineEditCompOptions->setText("");
    ui->lineEditLinkOptions->setText("");
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

    if(getCompiler().indexOf("++") > -1)
        args.append("-fno-rtti");

    /* other compiler options */
    if(getCompOptions().length())
        args.append(cflags+"::"+getCompOptions());

    /* libraries */
    if(getMathLib().length())
        args.append(getMathLib());
    if(getPthreadLib().length())
        args.append(getPthreadLib());

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
    return ui->checkBoxStripELF->isChecked() ? QString ("-s") : QString ("");
}

QString  ProjectOptions::getCompOptions()
{
    return ui->lineEditCompOptions->text().toAscii();
}
QString  ProjectOptions::getLinkOptions()
{
    return ui->lineEditLinkOptions->text().toAscii();
}
QString  ProjectOptions::getBoardType()
{
    return ui->lineEditLinkOptions->text().toAscii();
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
        if(s.contains("-s")) {
            setStripElf(true);
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
