#include "properties.h"

/*
 * get propeller-elf-gcc path, propeller-load directory path, and user workspace path.
 */
Properties::Properties(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(QString(ASideGuiKey)+tr(" Properties"));

    setupFolders();
    setupGeneral();
    // setupHighlight();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&tabWidget);
    layout->addWidget(buttonBox);

    setWindowFlags(Qt::Tool);
    resize(400,260);
}

void Properties::setupFolders()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QGroupBox *box = new QGroupBox();
    box->setLayout(layout);
    tabWidget.addTab(box," Folders ");

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

    connect(btnCompilerBrowse, SIGNAL(clicked()), this, SLOT(browseCompiler()));
    connect(btnIncludesBrowse, SIGNAL(clicked()), this, SLOT(browseIncludes()));
    connect(btnWorkspaceBrowse, SIGNAL(clicked()), this, SLOT(browseWorkspace()));

    gbCompiler->setLayout(clayout);
    gbIncludes->setLayout(ilayout);
    gbWorkspace->setLayout(wlayout);

    layout->addWidget(gbCompiler);
    layout->addWidget(gbIncludes);
    layout->addWidget(gbWorkspace);

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant compv = settings.value(compilerKey);
    QVariant incv = settings.value(includesKey);
    QVariant wrkv = settings.value(workspaceKey);

#if defined(Q_WS_WIN32)
    mypath = "C:/propgcc/";
    QString mygcc = mypath+"bin/propeller-elf-gcc.exe";
#else
    mypath = "/opt/parallax/";
    QString mygcc = mypath+"bin/propeller-elf-gcc";
#endif
    QString myinc = mypath+"propeller-load/";

    if(compv.canConvert(QVariant::String)) {
        QString s = compv.toString();
        s = QDir::fromNativeSeparators(s);
        leditCompiler->setText(s);
    }
    else {
        leditCompiler->setText(mygcc);
    }

    if(incv.canConvert(QVariant::String)) {
        QString s = incv.toString();
        s = QDir::fromNativeSeparators(s);
        leditIncludes->setText(s);
    }
    else {
        leditIncludes->setText(myinc);
    }

    if(wrkv.canConvert(QVariant::String)) {
        QString s = wrkv.toString();
        s = QDir::fromNativeSeparators(s);
        leditWorkspace->setText(s);
    }

}

void Properties::setupGeneral()
{
    QHBoxLayout *tlayout = new QHBoxLayout();
    QFrame *tbox = new QFrame();
    tbox->setLayout(tlayout);
    tabWidget.addTab(tbox," General ");

    QLabel *ltabs = new QLabel("Editor Tab Space Count",tbox);
    tlayout->addWidget(ltabs);
    tabSpaces.setMaximumWidth(40);
    tabSpaces.setText("4");
    tabSpaces.setAlignment(Qt::AlignHCenter);
    tlayout->addWidget(&tabSpaces);

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant tabv = settings.value(tabSpacesKey);
    if(tabv.canConvert(QVariant::Int)) {
        QString s = tabv.toString();
        tabSpaces.setText(s);
    }

}

void Properties::setupHighlight()
{
    QGridLayout *hlayout = new QGridLayout();
    QFrame *hlbox = new QFrame();
    hlbox->setLayout(hlayout);
    tabWidget.addTab(hlbox,"Highlight");

    propertyColors.insert(Properties::Black, new PColor("Black"));
    propertyColors.insert(Properties::DarkGray, new PColor("Dark Gray"));
    propertyColors.insert(Properties::Gray, new PColor("Gray"));
    propertyColors.insert(Properties::LightGray, new PColor("Light Gray"));
    propertyColors.insert(Properties::Blue, new PColor("Blue"));
    propertyColors.insert(Properties::DarkBlue, new PColor("Dark Blue"));
    propertyColors.insert(Properties::Cyan, new PColor("Cyan"));
    propertyColors.insert(Properties::DarkCyan, new PColor("Dark Cyan"));
    propertyColors.insert(Properties::Green, new PColor("Green"));
    propertyColors.insert(Properties::DarkGreen, new PColor("Dark Green"));
    propertyColors.insert(Properties::Magenta, new PColor("Magenta"));
    propertyColors.insert(Properties::DarkMagenta, new PColor("Dark Magenta"));
    propertyColors.insert(Properties::Red, new PColor("Red"));
    propertyColors.insert(Properties::DarkRed, new PColor("Dark Red"));
    propertyColors.insert(Properties::Yellow, new PColor("Yellow"));
    propertyColors.insert(Properties::DarkYellow, new PColor("Dark Yellow"));

    QStringList colorlist;
    for(int n = 0; n < propertyColors.count(); n++)
        colorlist.append(static_cast<PColor*>(propertyColors[n])->getName());

    int hlrow = 0;

    QLabel *lNumStyle = new QLabel("Numbers");
    hlayout->addWidget(lNumStyle,hlrow,0);
    hlNumWeight.setText("Bold");
    hlNumWeight.setChecked(false);
    hlayout->addWidget(&hlNumWeight,hlrow,1);
    hlNumStyle.addItem("Normal");
    hlNumStyle.addItem("Italic");
    hlayout->addWidget(&hlNumStyle,hlrow,2);
    hlNumColor.addItems(colorlist);
    hlayout->addWidget(&hlNumColor,hlrow,3);
    hlNumColor.setCurrentIndex(Properties::Magenta);
    hlrow++;

    QLabel *lFuncStyle = new QLabel("Functions");
    hlayout->addWidget(lFuncStyle,hlrow,0);
    hlFuncWeight.setText("Bold");
    hlFuncWeight.setChecked(false);
    hlayout->addWidget(&hlFuncWeight,hlrow,1);
    hlFuncStyle.addItem("Normal");
    hlFuncStyle.addItem("Italic");
    hlayout->addWidget(&hlFuncStyle,hlrow,2);
    hlFuncColor.addItems(colorlist);
    hlayout->addWidget(&hlFuncColor,hlrow,3);
    hlFuncColor.setCurrentIndex(Properties::Blue);
    hlrow++;

    QLabel *lKeyWordStyle = new QLabel("Key Words");
    hlayout->addWidget(lKeyWordStyle,hlrow,0);
    hlKeyWordWeight.setText("Bold");
    hlKeyWordWeight.setChecked(false);
    hlayout->addWidget(&hlKeyWordWeight,hlrow,1);
    hlKeyWordStyle.addItem("Normal");
    hlKeyWordStyle.addItem("Italic");
    hlayout->addWidget(&hlKeyWordStyle,hlrow,2);
    hlKeyWordColor.addItems(colorlist);
    hlayout->addWidget(&hlKeyWordColor,hlrow,3);
    hlKeyWordColor.setCurrentIndex(Properties::DarkBlue);
    hlrow++;

    QLabel *lPreProcStyle = new QLabel("Pre-Processor");
    hlayout->addWidget(lPreProcStyle,hlrow,0);
    hlPreProcWeight.setText("Bold");
    hlPreProcWeight.setChecked(false);
    hlayout->addWidget(&hlPreProcWeight,hlrow,1);
    hlPreProcStyle.addItem("Normal");
    hlPreProcStyle.addItem("Italic");
    hlayout->addWidget(&hlPreProcStyle,hlrow,2);
    hlPreProcColor.addItems(colorlist);
    hlayout->addWidget(&hlPreProcColor,hlrow,3);
    hlPreProcColor.setCurrentIndex(Properties::DarkYellow);
    hlrow++;

    QLabel *lQuoteStyle = new QLabel("Quotes");
    hlayout->addWidget(lQuoteStyle,hlrow,0);
    hlQuoteWeight.setText("Bold");
    hlQuoteWeight.setChecked(false);
    hlayout->addWidget(&hlQuoteWeight,hlrow,1);
    hlQuoteStyle.addItem("Normal");
    hlQuoteStyle.addItem("Italic");
    hlayout->addWidget(&hlQuoteStyle,hlrow,2);
    hlQuoteColor.addItems(colorlist);
    hlayout->addWidget(&hlQuoteColor,hlrow,3);
    hlQuoteColor.setCurrentIndex(Properties::Red);
    hlrow++;

    QLabel *lLineComStyle = new QLabel("Line Comments");
    hlayout->addWidget(lLineComStyle,hlrow,0);
    hlLineComWeight.setText("Bold");
    hlLineComWeight.setChecked(false);
    hlayout->addWidget(&hlLineComWeight,hlrow,1);
    hlLineComStyle.addItem("Normal");
    hlLineComStyle.addItem("Italic");
    hlayout->addWidget(&hlLineComStyle,hlrow,2);
    hlLineComColor.addItems(colorlist);
    hlayout->addWidget(&hlLineComColor,hlrow,3);
    hlLineComColor.setCurrentIndex(Properties::Green);
    hlrow++;

    QLabel *lBlockComStyle = new QLabel("Block Comments");
    hlayout->addWidget(lBlockComStyle,hlrow,0);
    hlBlockComWeight.setText("Bold");
    hlBlockComWeight.setChecked(false);
    hlayout->addWidget(&hlBlockComWeight,hlrow,1);
    hlBlockComStyle.addItem("Normal");
    hlBlockComStyle.addItem("Italic");
    hlayout->addWidget(&hlBlockComStyle,hlrow,2);
    hlBlockComColor.addItems(colorlist);
    hlayout->addWidget(&hlBlockComColor,hlrow,3);
    hlBlockComColor.setCurrentIndex(Properties::Green);
    hlrow++;
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
    QString path = "";
    if(vpath.canConvert(QVariant::String)) {
        path = vpath.toString();
        int len = path.length()-1;
        if(len < 0)
            path = QDir::rootPath();
        else
        if(path.at(len) == '/')
            path = path.mid(0,path.lastIndexOf("/"));
    }
    else {
        path = QDir::rootPath();
    }
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

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }

    leditWorkspace->setText(s);
}

void Properties::accept()
{
    QSettings settings(publisherKey, ASideGuiKey,this);
    settings.setValue(compilerKey,leditCompiler->text());
    settings.setValue(includesKey,leditIncludes->text());
    settings.setValue(configFileKey,leditIncludes->text());
    settings.setValue(workspaceKey,leditWorkspace->text());
    settings.setValue(tabSpacesKey,tabSpaces.text());
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
    this->show();
    this->activateWindow();
}

int Properties::getTabSpaces()
{
    return tabSpaces.text().toInt();
}
