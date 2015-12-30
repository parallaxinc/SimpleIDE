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

#include "aboutdialog.h"
#include "properties.h"

AboutDialog::AboutDialog(QString landing, QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *splash = new QLabel(this);
    splash->setPixmap(QPixmap(":/images/SimpleIDE-Splash3.png"));
    splash->setGeometry(0,0,400,300);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(splash);
    layout->addWidget(new QLabel(""));
    QString version = QString("%1 Version %2.%3.%4").arg(ASideGuiKey)
            .arg(IDEVERSION).arg(MINVERSION).arg(FIXVERSION);
    layout->addWidget(new QLabel(version+"\n"+tr("Copyright (C) 2012-2015, Parallax, Inc.")+" "+
            tr("See credits for license information.")));

    QLabel *landingLabel = new QLabel(landing);
    landingLabel->setOpenExternalLinks(true);
    layout->addWidget(landingLabel);

    showSplashStartCheckBox = new QCheckBox (tr("Show this window at startup."));
    showSplashStartCheckBox->setChecked(true);
    layout->addWidget(showSplashStartCheckBox);

    layout->addWidget(buttonBox);
    setLayout(layout);
}

void AboutDialog::accept()
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    if(showSplashStartCheckBox->isChecked() == false)
        settings.setValue(helpStartupKey, false);
    else
        settings.setValue(helpStartupKey, true);
    this->done(QDialog::Accepted);
}

void AboutDialog::reject()
{
    this->done(QDialog::Rejected);
}

void AboutDialog::show()
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant helpStartup = settings.value(helpStartupKey,true);
    if(helpStartup.canConvert(QVariant::Bool)) {
        showSplashStartCheckBox->setChecked(helpStartup.toBool());
    }
    QDialog::show();
}

int AboutDialog::exec()
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant helpStartup = settings.value(helpStartupKey,true);
    if(helpStartup.canConvert(QVariant::Bool)) {
        showSplashStartCheckBox->setChecked(helpStartup.toBool());
    }
    return QDialog::exec();
}
