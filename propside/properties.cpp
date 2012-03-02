#include "properties.h"

/*
 * get propeller-elf-gcc path, propeller-load directory path, and user workspace path.
 */
Properties::Properties(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QGroupBox *gbCompiler = new QGroupBox(tr("Compiler"), this);
    QGroupBox *gbIncludes = new QGroupBox(tr("Loader Folder"), this);
    QGroupBox *gbWorkspace = new QGroupBox(tr("Workspace Folder"), this);

    QPushButton *btnCompilerBrowse = new QPushButton(tr("Browse"), this);
    leditCompiler = new QLineEdit(this);
    QHBoxLayout *clayout = new QHBoxLayout();
    clayout->addWidget(leditCompiler);
    clayout->addWidget(btnCompilerBrowse);

    QPushButton *btnIncludesBrowse = new QPushButton(tr("Browse"), this);
    leditIncludes = new QLineEdit(this);
    QHBoxLayout *ilayout = new QHBoxLayout();
    ilayout->addWidget(leditIncludes);
    ilayout->addWidget(btnIncludesBrowse);

    QPushButton *btnWorkspaceBrowse = new QPushButton(tr("Browse"), this);
    leditWorkspace = new QLineEdit(this);
    QHBoxLayout *wlayout = new QHBoxLayout();
    wlayout->addWidget(leditWorkspace);
    wlayout->addWidget(btnWorkspaceBrowse);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btnCompilerBrowse, SIGNAL(clicked()), this, SLOT(browseCompiler()));
    connect(btnIncludesBrowse, SIGNAL(clicked()), this, SLOT(browseIncludes()));
    connect(btnWorkspaceBrowse, SIGNAL(clicked()), this, SLOT(browseWorkspace()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    gbCompiler->setLayout(clayout);
    gbIncludes->setLayout(ilayout);
    gbWorkspace->setLayout(wlayout);

    layout->addWidget(gbCompiler);
    layout->addWidget(gbIncludes);
    layout->addWidget(gbWorkspace);
    layout->addWidget(buttonBox);

    setLayout(layout);

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant compv = settings.value(compilerKey);
    QVariant incv = settings.value(includesKey);
    QVariant wrkv = settings.value(workspaceKey);

#if defined(Q_WS_WIN32)
    mypath = "c:/propgcc/";
#else
    mypath = "/opt/parallax/";
#endif

    if(compv.canConvert(QVariant::String)) {
        QString s = compv.toString();
        s = QDir::fromNativeSeparators(s);
        leditCompiler->setText(s);
    }

    if(incv.canConvert(QVariant::String)) {
        QString s = incv.toString();
        s = QDir::fromNativeSeparators(s);
        leditIncludes->setText(s);
    }

    if(wrkv.canConvert(QVariant::String)) {
        QString s = wrkv.toString();
        s = QDir::fromNativeSeparators(s);
        leditWorkspace->setText(s);
    }

    setWindowFlags(Qt::Tool);
    resize(400,260);
}


void Properties::browseCompiler()
{
    //QString fileName = QFileDialog::getOpenFileName(this,tr("Select Compiler"), mypath, "Compiler (propeller-elf-gcc.*)");
#if defined(Q_WS_WIN32)
    QFileDialog fileDialog(this,  tr("Select Compiler"), mypath+"bin/propeller-elf-gcc.exe", "Compiler (propeller-elf-gcc.exe)");
#else
    QFileDialog fileDialog(this,  tr("Select Compiler"), mypath+"bin/propeller-elf-gcc", "Compiler (propeller-elf-gcc)");
#endif
    fileDialog.exec();
    QStringList files = fileDialog.selectedFiles();
    QString fileName = files.at(0);

    QString s = QDir::fromNativeSeparators(fileName);
    compilerstr = leditCompiler->text();
    if(s.length() > 0) {
        leditCompiler->setText(s);
    }
    qDebug() << "browseCompiler" << s;
}

void Properties::browseIncludes()
{
    QFileDialog fileDialog(this,tr("New Project Folder"),mypath,tr("Project Folder (propeller-load)"));

    QStringList filenames;
    QString pathName;

    fileDialog.setOptions(QFileDialog::ShowDirsOnly);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.selectFile(mypath+"propeller-load");

    if(fileDialog.exec())
        filenames = fileDialog.selectedFiles();
    if(filenames.length() > 0)
        pathName = filenames.at(0);

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    leditIncludes->setText(s);
    qDebug() << "browseIncludes" << s;
}

void Properties::browseWorkspace()
{
    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant vpath = settings.value(workspaceKey,QVariant("~/."));
    QString path = vpath.toString();
    if(path.at(path.length()-1) == '/')
        path = path.mid(0,path.lastIndexOf("/"));
    QFileDialog fileDialog(this,tr("Project Workspace"),path,tr("Project Workspace (*)"));

    QStringList filenames;
    QString pathName;

    fileDialog.setOptions(QFileDialog::ShowDirsOnly);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.selectFile(path);
    if(fileDialog.exec())
        filenames = fileDialog.selectedFiles();
    if(filenames.length() > 0)
        pathName = filenames.at(0);
}

void Properties::accept()
{
    QSettings settings(publisherKey, ASideGuiKey,this);
    settings.setValue(compilerKey,leditCompiler->text());
    settings.setValue(includesKey,leditIncludes->text());
    settings.setValue(configFileKey,leditIncludes->text());
    settings.setValue(workspaceKey,leditWorkspace->text());
    done(QDialog::Accepted);
}

void Properties::reject()
{
    leditCompiler->setText(compilerstr);
    leditIncludes->setText(includesstr);
    done(QDialog::Rejected);
}

void Properties::showProperties()
{
    compilerstr = leditCompiler->text();
    includesstr = leditIncludes->text();
    this->setWindowTitle(QString(ASideGuiKey)+tr(" Compiler Properties"));
    this->exec();
}
