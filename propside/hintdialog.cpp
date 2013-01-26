#include "hintdialog.h"
#include "ui_hintdialog.h"
#include "properties.h"

HintDialog::HintDialog(QString tag, QString hint, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HintDialog),
    settings(publisherKey, ASideGuiKey),
    key(HintKeyPrefix + tag)
{
    ui->setupUi(this);
    setWindowTitle(tr("Hint"));
    ui->hintLabel->setText(hint);
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(on_okButton_clicked()));
}

HintDialog::~HintDialog()
{
    delete ui;
}

int HintDialog::exec()
{
    int sts = 0;
    if (settings.value(key, true).toBool())
        sts = QDialog::exec();
    return sts;
}

void HintDialog::on_okButton_clicked()
{
    if (!ui->showNextTimeCheckBox->isChecked())
        settings.setValue(key, false);
    close();
}

void HintDialog::hint(QString tag, QString hint, QWidget *parent)
{
    HintDialog *myHint = new HintDialog(tag, hint, parent);
    myHint->exec();
}
