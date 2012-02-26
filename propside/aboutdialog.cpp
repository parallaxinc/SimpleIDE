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
    QString version = QString("%1 Version %2.%3.%4").arg(ASideGuiKey)
            .arg(IDEVERSION).arg(MINVERSION).arg(FIXVERSION);
    layout->addWidget(new QLabel(version+"         "+tr("Copyright (C) 2012, Parallax, Inc.")));
    QString license(tr("An MIT Licensed open-source IDE using Qt libraries under LGPLv2.1"));
    layout->addWidget(new QLabel(license));
    QString icons(tr("Most ToolBar Icons \"24x24 Free Application Icons\" by www.aha-soft.com,\nused under Creative Commons Attribution-Share Alike 3.0 License."));
    layout->addWidget(new QLabel(icons));

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
