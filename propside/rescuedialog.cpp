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

#include "rescuedialog.h"
#include <QMessageBox>

RescueDialog::RescueDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Rescue Me!"));
    setWindowFlags(Qt::Tool);
    layout = new QVBoxLayout(this);
    hlayout = new QHBoxLayout(this);
    copyBtn = new QPushButton(tr("Copy"),this);
    connect(copyBtn, SIGNAL(clicked()), this, SLOT(copyText()));
    label = new QLabel(tr("Copy and paste this information to a place where someone can help."));
    edit  = new QPlainTextEdit();
    hlayout->addWidget(copyBtn);
    hlayout->addWidget(label,100);
    layout->addLayout(hlayout);
    layout->addWidget(edit);
    setLayout(layout);
    resize(600,400);
}

void RescueDialog::setEditText(QString text)
{
    edit->setPlainText(text);
    edit->selectAll();
    edit->setReadOnly(true);
}

void RescueDialog::copyText()
{
    edit->selectAll();
    edit->copy();
    QMessageBox::information(this,tr("Copy Done"),tr("Text copied. Now paste."));
}
