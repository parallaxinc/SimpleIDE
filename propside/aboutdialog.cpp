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
#if 0
    QString license(tr("An MIT Licensed open-source IDE using Qt libraries under LGPLv2.1"));
    layout->addWidget(new QLabel(license));
    QString icons(tr("Most Tool Icons \"24x24 Free Application Icons\" by www.aha-soft.com,\nused under Creative Commons Attribution-Share Alike 3.0 License."));
    layout->addWidget(new QLabel(icons));
    QString ctags(tr("Uses ctags binary program for project browser functions under GPLv2.\nThe ctags source/binary are kept at http://ctags.sourceforge.net"));
    layout->addWidget(new QLabel(ctags));
    QString propgcc(tr("Uses Propeller GCC under GPLv3 and exception for key compiler tools.\nPropeller GCC source/binary are at http://propgcc.googlecode.com"));
    layout->addWidget(new QLabel(propgcc));
#endif
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
