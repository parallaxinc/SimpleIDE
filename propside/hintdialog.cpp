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
