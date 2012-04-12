#include "aboutdialog.h"
#include "properties.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *splash = new QLabel(this);
    splash->setPixmap(QPixmap(":/images/SimpleIDE-Splash.png"));
    splash->setGeometry(0,0,400,300);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(splash);
    layout->addWidget(new QLabel(""));
    QString version = QString("%1 Version %2.%3.%4").arg(ASideGuiKey)
            .arg(IDEVERSION).arg(MINVERSION).arg(FIXVERSION);
    layout->addWidget(new QLabel(version+"\n"+tr("Copyright (C) 2012, Parallax, Inc.")));

    layout->addWidget(buttonBox);
    setLayout(layout);
}

void AboutDialog::accept()
{
    this->done(QDialog::Accepted);
}

void AboutDialog::reject()
{
    this->done(QDialog::Rejected);
}
