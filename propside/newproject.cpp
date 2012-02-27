#include "newproject.h"
#include "properties.h"

NewProject::NewProject(QWidget *parent) : QDialog(parent)
{
    name = new QLineEdit(tr("project"), this);

    QString userpath = getCurrentPath();
    path = new QLineEdit(userpath,this);
    QLabel *nameLabel = new QLabel(tr("Project Name"));
    QLabel *pathLabel = new QLabel(tr("Folder"));

    QPushButton *btnBrowsePath = new QPushButton(this);
    btnBrowsePath->setText(tr("Browse"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBrowsePath, SIGNAL(clicked()), this, SLOT(browsePath()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *blay = new QVBoxLayout(this);
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(nameLabel,0,0,1,1);
    layout->addWidget(name,0,1,1,1);
    layout->addWidget(pathLabel,1,0,1,1);
    layout->addWidget(path,1,1,1,1);
    layout->addWidget(btnBrowsePath,1,2,1,1);

    blay->addLayout(layout);
    blay->addWidget(buttonBox);
    setLayout(blay);
    buttonBox->setFocus();

    setMinimumWidth(userpath.length()*path->fontInfo().pointSize()+100);
    setWindowFlags(Qt::Tool);
}

NewProject::~NewProject()
{
    delete name;
    delete path;
}

QString NewProject::getCurrentPath()
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant  lastfile = settings.value(lastFileNameKey);
    QString userpath("");
    if(lastfile.canConvert(QVariant::String)) {
        userpath = lastfile.toString();
        userpath = QDir::fromNativeSeparators(userpath);
        /* "/" gets converted to user OS version */
        userpath = userpath.mid(0,userpath.lastIndexOf("/")+1);
    }
    return userpath;
}

void NewProject::browsePath()
{
    QFileDialog fileDialog(this);
    QString mypath = getCurrentPath();
    if(mypath.length())
        fileDialog.setDirectory(mypath);
    QString pathName = fileDialog.getExistingDirectory(this,
        tr("New Project Folder"), tr("Project Folder (*)"));

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    if(s.indexOf("/") > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    else if(s.indexOf("\\") > -1) {
        if(s.mid(s.length()-1) != "\\")
            s += "\\";
    }
    path->setText(s);
    qDebug() << "New Project Folder " << s;
}

void NewProject::accept()
{
    done(QDialog::Accepted);
}

void NewProject::reject()
{
    done(QDialog::Rejected);
}

void NewProject::showDialog()
{
    this->setWindowTitle(QString(ASideGuiKey)+tr(" New Project"));
    this->show();
}

QString NewProject::getName()
{
    return name->text();
}

QString NewProject::getPath()
{
    return path->text();
}
