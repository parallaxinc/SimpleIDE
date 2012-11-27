#include "newproject.h"
#include "properties.h"

/*
 * Path is not saved in linux without installer - added workspace to properties dialog.
 */
NewProject::NewProject(QWidget *parent) : QDialog(parent)
{
    mypath = getCurrentPath();
    path = new QLineEdit(mypath,this);
    path->setToolTip(tr("Directory for new project."));
    path->setDisabled(true);

    QPushButton *btnBrowsePath = new QPushButton(this);
    btnBrowsePath->setText(tr("Browse"));

    name = new QLineEdit(this);
    name->setToolTip(tr("New project name and sub-directory."));
    connect(name,SIGNAL(textChanged(QString)),this,SLOT(nameChanged()));

    path->setText(mypath+name->text());
    QLabel *pathLabel = new QLabel(tr("Workspace"));
    QLabel *nameLabel = new QLabel(tr("Project Name"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBrowsePath, SIGNAL(clicked()), this, SLOT(browsePath()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    okButton = (QPushButton*) buttonBox->buttons().at(0);
    cancelButton = (QPushButton*) buttonBox->buttons().at(1);
    if(okButton->text().contains("cancel",Qt::CaseInsensitive)) {
        okButton = (QPushButton*) buttonBox->buttons().at(1);
        cancelButton = (QPushButton*) buttonBox->buttons().at(1);
    }

    QLabel *create = new QLabel(this);
    create->setText(tr("Creating a new project:"));
    create->setFont(QFont(this->font().family(),this->font().pointSize()*1.5,QFont::Bold));
    QLabel *inst = new QLabel(this);
    inst->setText(tr(" 1) Choose project workspace.\n 2) Set project name.\n\n" \
            "A project folder with project name will be created in the workspace.\n" \
            "The new folder will have a main .c file and a .side project file.\n"));

    comptype = new QComboBox();
    QStringList tlist;
    tlist.append("C");
    tlist.append("C++");
    tlist.append("SPIN");
    comptype->addItems(tlist);

    QVBoxLayout *blay = new QVBoxLayout(this);
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(pathLabel,    0,0,1,1);
    layout->addWidget(path,         0,1,1,1); // tab order for widgets is according to add order
    layout->addWidget(btnBrowsePath,0,2,1,1);
    layout->addWidget(nameLabel,    1,0,1,1);
    layout->addWidget(name,         1,1,1,1);
    layout->addWidget(comptype,     1,2,1,1);

    blay->addWidget(create);
    blay->addWidget(inst);
    blay->addLayout(layout);
    blay->addWidget(new QLabel(this));
    blay->addWidget(buttonBox);
    setLayout(blay);
    buttonBox->setFocus();

    int fontSize = path->fontInfo().pixelSize();
    setMinimumWidth(mypath.length()*fontSize+100);
    setWindowFlags(Qt::Tool);
}

NewProject::~NewProject()
{
    delete name;
    delete path;
}

void NewProject::nameChanged()
{
    QString myname = name->text();
    path->setText(mypath+myname);
    QDir path(mypath);
    okButton->setDisabled(true);
    /* don't allow project if mypath is empty */
    if(mypath.length() != 0 && path.exists()) {
        /* make sure our new file has no spaces */
        if(myname.indexOf(" ") > -1 || myname.indexOf("\t") > -1) {
            return;
        }
        if(myname.length() > 0) {
            /* make sure our new path is not a file */
            QFile isFile(mypath+myname);
            if(isFile.exists() == false)
                okButton->setEnabled(true);
        }
    }
}

QString NewProject::getCurrentPath()
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant  lastfile = settings.value(workspaceKey);
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
    QString pathName;
    QString fullname = mypath;

    if(mypath.length() == 0) {
        qDebug() << "mypath is empty.";
    }
    else {
        if(mypath.at(mypath.length()-1) == '/')
            mypath = mypath.left(mypath.length()-1);
    }
    mypath += "/";

    QString filename = QFileDialog::getExistingDirectory(this,tr("New Project Folder"),fullname,QFileDialog::ShowDirsOnly);

    if(filename.length() == 0)
        return;

    pathName = filename;

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    mypath = s;
    if(mypath.indexOf("/") > -1) {
        if(mypath.mid(mypath.length()-1) != "/")
            mypath += "/";
    }
    else if(mypath.indexOf("\\") > -1) {
        if(mypath.mid(mypath.length()-1) != "\\")
            mypath += "\\";
    }
    path->setText(mypath+name->text());
    QSettings settings(publisherKey, ASideGuiKey, this);
    settings.setValue(workspaceKey,mypath);
    int fontSize = path->fontInfo().pixelSize();
    setMinimumWidth(mypath.length()*fontSize+100);
    qDebug() << "New Project Folder " << mypath << name->text();

    nameChanged();
}

void NewProject::accept()
{
    if(name->text().length()<1) {
        QMessageBox::information(this,
                 tr("Need Project Name"),
                 tr("Please enter a project name."),QMessageBox::Ok);
        return;
    }
    int rc = QMessageBox::question(this,
                 tr("Confirm New Project"),
                 tr("Create new project file \"")+name->text()+".side\""+tr(" in\n")+path->text()+tr("?"),
                 QMessageBox::Yes, QMessageBox::No);
    if(rc == QMessageBox::Yes) {
        path->setText(mypath+name->text());
        done(QDialog::Accepted);
    }
}

void NewProject::reject()
{
    done(QDialog::Rejected);
}

void NewProject::showDialog()
{
    int fontSize = path->fontInfo().pixelSize();
    setMinimumWidth(mypath.length()*fontSize+100);

    mypath = getCurrentPath();
    path->setText(mypath+name->text());

    if(okButton == NULL)
        return;
    if(cancelButton == NULL)
        return;

    /* disable OK button until user has satisfied criteria. */
    okButton->setDisabled(true);

    this->setWindowTitle(QString(ASideGuiKey)+tr(" New Project"));
    this->nameChanged();
    this->exec();
}

QString NewProject::getName()
{
    return name->text();
}

QString NewProject::getPath()
{
    return path->text();
}

QString NewProject::getCompilerType()
{
    return comptype->currentText();
}
