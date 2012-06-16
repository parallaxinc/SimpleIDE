#include "properties.h"

/*
 * get propeller-elf-gcc path, propeller-load directory path, and user workspace path.
 */
Properties::Properties(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(QString(ASideGuiKey)+tr(" Properties"));

    /* clean for testing only */
    // cleanSettings();

    setupFolders();
    setupGeneral();
    //setupOptional(); // add later
    setupHighlight();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&tabWidget);
    layout->addWidget(buttonBox);

    setWindowFlags(Qt::Tool);
    resize(600,260);
}

void Properties::cleanSettings()
{
    QSettings settings(publisherKey, ASideGuiKey,this);
    QStringList list = settings.allKeys();

    foreach(QString key, list) {
        if(key.indexOf(ASideGuiKey) == 0) {
            settings.remove(key);
        }
    }

    settings.remove(publisherComKey);
    settings.remove(publisherKey);

    qApp->exit(0);
}

void Properties::setupFolders()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QFrame *box = new QFrame();
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

#if defined(Q_WS_WIN32)
    mypath = "C:/propgcc/";
    QString mygcc = mypath+"bin/propeller-elf-gcc.exe";
#elif defined(Q_WS_MAC)
    QString apath = QApplication::applicationFilePath();
    apath = apath.mid(0,apath.lastIndexOf(".app"));
    apath = apath.mid(0,apath.lastIndexOf("/"));
    mypath = apath+"/parallax/";
    QString mygcc = mypath+"bin/propeller-elf-gcc";
#else
    mypath = "/opt/parallax/";
    QString mygcc = mypath+"bin/propeller-elf-gcc";
    if(QFile::exists(mygcc) == false) {
        qDebug() << "Alternative Default Compiler?";
        mypath = "./parallax/";
        mygcc = mypath+"bin/propeller-elf-gcc";
    }
#endif
    QString myinc = mypath+"propeller-load/";

    if(QFile::exists(mygcc))
        qDebug() << "Found Default Compiler.";
    if(QFile::exists(myinc))
        qDebug() << "Found Default Loader Path.";

    QVariant compv = settings.value(compilerKey,mygcc);
    QVariant incv = settings.value(includesKey,myinc);
    QVariant wrkv = settings.value(workspaceKey);

    if(compv.canConvert(QVariant::String)) {
        QString s = compv.toString();
        if(s.length() > 0) {
            s = QDir::fromNativeSeparators(s);
            leditCompiler->setText(s);
        }
        else {
            leditCompiler->setText(mygcc);
            settings.setValue(compilerKey,mygcc);
        }
    }
    else {
        leditCompiler->setText(mygcc);
        settings.setValue(compilerKey,mygcc);
    }

    if(incv.canConvert(QVariant::String)) {
        QString s = incv.toString();
        if(s.length() > 0) {
            s = QDir::fromNativeSeparators(s);
            leditIncludes->setText(s);
        }
        else {
            leditIncludes->setText(myinc);
            settings.setValue(includesKey,myinc);
        }
    }
    else {
        leditIncludes->setText(myinc);
        settings.setValue(includesKey,myinc);
    }

    if(wrkv.canConvert(QVariant::String)) {
        QString s = wrkv.toString();
        s = QDir::fromNativeSeparators(s);
        leditWorkspace->setText(s);
    }

}

void Properties::setupGeneral()
{
    int row = 0;

    QFrame *tbox = new QFrame();

    tabWidget.addTab(tbox," General ");

    QVBoxLayout *glayout = new QVBoxLayout();
    tbox->setLayout(glayout);

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant var;

    QGroupBox *gbGeneral = new QGroupBox(tr("General Settings"),tbox);
    QGridLayout *tlayout = new QGridLayout();

    QLabel *ltabs = new QLabel(tr("Editor Tab Space Count"),tbox);
    tlayout->addWidget(ltabs,row,0);
    tabSpaces.setMaximumWidth(40);
    tabSpaces.setText("4");
    tabSpaces.setAlignment(Qt::AlignHCenter);
    tlayout->addWidget(&tabSpaces,row++,1);

    var = settings.value(tabSpacesKey);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        tabSpaces.setText(s);
    }

    QLabel *lLoadDelay = new QLabel(tr("Loader Delay"),tbox);
    tlayout->addWidget(lLoadDelay,row,0);
    loadDelay.setMaximumWidth(40);
    loadDelay.setText("0");
    loadDelay.setAlignment(Qt::AlignHCenter);
    tlayout->addWidget(&loadDelay,row++,1);

    var = settings.value(loadDelayKey);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        loadDelay.setText(s);
    }

    QLabel *lreset = new QLabel(tr("Reset Signal"),tbox);
    tlayout->addWidget(lreset,row,0);
    resetType.addItem("DTR");
    resetType.addItem("RTS");
    resetType.addItem("CFG");
    resetType.setCurrentIndex((int)CFG);
    tlayout->addWidget(&resetType,row++,1);

    var = settings.value(resetTypeKey,(int)CFG);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        resetType.setCurrentIndex(var.toInt());
    }

    QLabel *lclear = new QLabel(tr("Clear options for next startup."),tbox);
    tlayout->addWidget(lclear,row,0);
    QPushButton *clearSettings = new QPushButton(tr("Clear and Exit"),this);
    clearSettings->setToolTip(tr("Exit Program"));
    connect(clearSettings,SIGNAL(clicked()),this,SLOT(cleanSettings()));
    tlayout->addWidget(clearSettings,row,1);

    gbGeneral->setLayout(tlayout);
    glayout->addWidget(gbGeneral);
}

void Properties::setupOptional()
{
    QFrame *tbox = new QFrame();

    tabWidget.addTab(tbox,tr("Optional"));

    QVBoxLayout *glayout = new QVBoxLayout();
    tbox->setLayout(glayout);

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant var;

    QGroupBox *gbCompiler = new QGroupBox(tr("Spin Compiler"),tbox);

    QLabel *compLabel = new QLabel();
    // spin compiler either BSTC or Roy's SPIN compiler
    var = settings.value(spinCompilerKey);
    if(var.canConvert(QVariant::String)) {
        QString s = var.toString();
        if(s.length() > 0)
            leditSpinCompiler.setText(s);
    }
    else {
        leditSpinCompiler.setText(mypath+"bstc");
    }
    QPushButton *btnCompilerBrowse = new QPushButton(tr("Browse"), this);
    QHBoxLayout *clayout = new QHBoxLayout();
    clayout->addWidget(compLabel);
    clayout->addWidget(&leditSpinCompiler);
    clayout->addWidget(btnCompilerBrowse);


    QGroupBox *gbAltTerm = new QGroupBox(tr("Alternative Terminal Program"),tbox);

    QLabel *altTermLabel = new QLabel();
    // spin compiler either BSTC or Roy's SPIN compiler
    var = settings.value(altTerminalKey);
    if(var.canConvert(QVariant::String)) {
        QString s = var.toString();
        if(s.length() > 0)
            leditAltTerminal.setText(s);
    }

    QPushButton *btnAltTermBrowse = new QPushButton(tr("Browse"), this);
    QHBoxLayout *atlayout = new QHBoxLayout();
    atlayout->addWidget(altTermLabel);
    atlayout->addWidget(&leditAltTerminal);
    atlayout->addWidget(btnAltTermBrowse);

    gbCompiler->setLayout(clayout);
    gbAltTerm->setLayout(atlayout);

    glayout->addWidget(gbCompiler);
    glayout->addWidget(gbAltTerm);

}

void Properties::addHighlights(QComboBox *box, QVector<PColor*> pcolor)
{
    for(int n = 0; n < pcolor.count(); n++) {
        QPixmap pixmap(20,20);
        pixmap.fill(pcolor.at(n)->getValue());
        QIcon icon(pixmap);
        box->addItem(icon, static_cast<PColor*>(propertyColors[n])->getName());
    }
}

void Properties::setupHighlight()
{
    QGridLayout *hlayout = new QGridLayout();
    QFrame *hlbox = new QFrame();
    hlbox->setLayout(hlayout);
    tabWidget.addTab(hlbox,tr("Highlight"));

    propertyColors.insert(Properties::Black, new PColor(tr("Black"), Qt::black));
    propertyColors.insert(Properties::DarkGray, new PColor(tr("Dark Gray"), Qt::darkGray));
    propertyColors.insert(Properties::Gray, new PColor(tr("Gray"),Qt::gray));
    propertyColors.insert(Properties::LightGray, new PColor(tr("Light Gray"),Qt::lightGray));
    propertyColors.insert(Properties::Blue, new PColor(tr("Blue"),Qt::blue));
    propertyColors.insert(Properties::DarkBlue, new PColor(tr("Dark Blue"),Qt::darkBlue));
    propertyColors.insert(Properties::Cyan, new PColor(tr("Cyan"),Qt::cyan));
    propertyColors.insert(Properties::DarkCyan, new PColor(tr("Dark Cyan"),Qt::darkCyan));
    propertyColors.insert(Properties::Green, new PColor(tr("Green"),Qt::green));
    propertyColors.insert(Properties::DarkGreen, new PColor(tr("Dark Green"),Qt::darkGreen));
    propertyColors.insert(Properties::Magenta, new PColor(tr("Magenta"),Qt::magenta));
    propertyColors.insert(Properties::DarkMagenta, new PColor(tr("Dark Magenta"),Qt::darkMagenta));
    propertyColors.insert(Properties::Red, new PColor(tr("Red"),Qt::red));
    propertyColors.insert(Properties::DarkRed, new PColor(tr("Dark Red"),Qt::darkRed));
    propertyColors.insert(Properties::Yellow, new PColor(tr("Yellow"),Qt::yellow));
    propertyColors.insert(Properties::DarkYellow, new PColor(tr("Dark Yellow"),Qt::darkYellow));

    QStringList colorlist;
    for(int n = 0; n < propertyColors.count(); n++) {
        colorlist.append(static_cast<PColor*>(propertyColors[n])->getName());
    }

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant var;

    int hlrow = 0;

    /*
        hlEnableKey                 // not implemented

        hlNumStyleKey               // Numeric style normal = 0 italic = 1
        hlNumWeightKey              // bold weight checked
        hlNumColorKey               // color integer
        hlFuncStyleKey              // function style - see Numeric
        hlFuncWeightKey             // bold weight checked
        hlFuncColorKey              // color integer
        hlKeyWordStyleKey
        hlKeyWordWeightKey
        hlKeyWordColorKey
        hlPreProcStyleKey
        hlPreProcWeightKey
        hlPreProcColorKey
        hlQuoteStyleKey
        hlQuoteWeightKey
        hlQuoteColorKey
        hlLineComStyleKey
        hlLineComWeightKey
        hlLineComColorKey
        hlBlockComStyleKey
        hlBlockComWeightKey
        hlBlockComColorKey
      */

    QLabel *lNumStyle = new QLabel(tr("Numbers"));
    hlayout->addWidget(lNumStyle,hlrow,0);
    hlNumWeight.setText(tr("Bold"));
    hlNumWeight.setChecked(false);
    hlayout->addWidget(&hlNumWeight,hlrow,1);
    hlNumStyle.setText(tr("Italic"));
    hlNumStyle.setChecked(false);
    hlayout->addWidget(&hlNumStyle,hlrow,2);
    addHighlights(&hlNumColor, propertyColors);
    hlayout->addWidget(&hlNumColor,hlrow,3);
    hlNumColor.setCurrentIndex(Properties::Magenta);
    hlrow++;

    var = settings.value(hlNumWeightKey,true);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlNumWeight.setChecked(var.toBool());
        settings.setValue(hlNumWeightKey,var.toBool());
    }

    var = settings.value(hlNumStyleKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlNumStyle.setChecked(var.toBool());
        settings.setValue(hlNumStyleKey,var.toBool());
    }

    var = settings.value(hlNumColorKey,Properties::Magenta);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlNumColor.setCurrentIndex(n);
        settings.setValue(hlNumColorKey,n);
    }

    QLabel *lFuncStyle = new QLabel(tr("Functions"));
    hlayout->addWidget(lFuncStyle,hlrow,0);
    hlFuncWeight.setText(tr("Bold"));
    hlFuncWeight.setChecked(false);
    hlayout->addWidget(&hlFuncWeight,hlrow,1);
    hlFuncStyle.setText(tr("Italic"));
    hlFuncStyle.setChecked(false);
    hlayout->addWidget(&hlFuncStyle,hlrow,2);
    addHighlights(&hlFuncColor, propertyColors);
    hlayout->addWidget(&hlFuncColor,hlrow,3);
    hlFuncColor.setCurrentIndex(Properties::Blue);
    hlrow++;

    var = settings.value(hlFuncWeightKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlFuncWeight.setChecked(var.toBool());
        settings.setValue(hlFuncWeightKey,var.toBool());
    }

    var = settings.value(hlFuncStyleKey,true);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlFuncStyle.setChecked(var.toBool());
        settings.setValue(hlFuncStyleKey,var.toBool());
    }

    var = settings.value(hlFuncColorKey,Properties::Blue);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlFuncColor.setCurrentIndex(n);
        settings.setValue(hlFuncColorKey,n);
    }


    QLabel *lKeyWordStyle = new QLabel(tr("Key Words"));
    hlayout->addWidget(lKeyWordStyle,hlrow,0);
    hlKeyWordWeight.setText(tr("Bold"));
    hlKeyWordWeight.setChecked(true);
    hlayout->addWidget(&hlKeyWordWeight,hlrow,1);
    hlKeyWordStyle.setText(tr("Italic"));
    hlKeyWordStyle.setChecked(false);
    hlayout->addWidget(&hlKeyWordStyle,hlrow,2);
    addHighlights(&hlKeyWordColor, propertyColors);
    hlayout->addWidget(&hlKeyWordColor,hlrow,3);
    hlKeyWordColor.setCurrentIndex(Properties::DarkBlue);
    hlrow++;

    var = settings.value(hlKeyWordWeightKey,true);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlKeyWordWeight.setChecked(var.toBool());
        settings.setValue(hlKeyWordWeightKey,var.toBool());
    }

    var = settings.value(hlKeyWordStyleKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlKeyWordStyle.setChecked(var.toBool());
        settings.setValue(hlKeyWordStyleKey,var.toBool());
    }

    var = settings.value(hlKeyWordColorKey,Properties::DarkBlue);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlKeyWordColor.setCurrentIndex(n);
        settings.setValue(hlKeyWordColorKey,n);
    }

    QLabel *lPreProcStyle = new QLabel(tr("Pre-Processor"));
    hlayout->addWidget(lPreProcStyle,hlrow,0);
    hlPreProcWeight.setText(tr("Bold"));
    hlPreProcWeight.setChecked(false);
    hlayout->addWidget(&hlPreProcWeight,hlrow,1);
    hlPreProcStyle.setText(tr("Italic"));
    hlPreProcStyle.setChecked(false);
    hlayout->addWidget(&hlPreProcStyle,hlrow,2);
    addHighlights(&hlPreProcColor, propertyColors);
    hlayout->addWidget(&hlPreProcColor,hlrow,3);
    hlPreProcColor.setCurrentIndex(Properties::DarkYellow);
    hlrow++;

    var = settings.value(hlPreProcWeightKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlPreProcWeight.setChecked(var.toBool());
        settings.setValue(hlPreProcWeightKey,var.toBool());
    }

    var = settings.value(hlPreProcStyleKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlPreProcStyle.setChecked(var.toBool());
        settings.setValue(hlPreProcStyleKey,var.toBool());
    }

    var = settings.value(hlPreProcColorKey,Properties::DarkYellow);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlPreProcColor.setCurrentIndex(n);
        settings.setValue(hlPreProcColorKey,n);
    }

    QLabel *lQuoteStyle = new QLabel(tr("Quotes"));
    hlayout->addWidget(lQuoteStyle,hlrow,0);
    hlQuoteWeight.setText(tr("Bold"));
    hlQuoteWeight.setChecked(false);
    hlayout->addWidget(&hlQuoteWeight,hlrow,1);
    hlQuoteStyle.setText(tr("Italic"));
    hlQuoteStyle.setChecked(false);
    hlayout->addWidget(&hlQuoteStyle,hlrow,2);
    addHighlights(&hlQuoteColor, propertyColors);
    hlayout->addWidget(&hlQuoteColor,hlrow,3);
    hlQuoteColor.setCurrentIndex(Properties::Red);
    hlrow++;

    var = settings.value(hlQuoteWeightKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlQuoteWeight.setChecked(var.toBool());
        settings.setValue(hlQuoteWeightKey,var.toBool());
    }

    var = settings.value(hlQuoteStyleKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlQuoteStyle.setChecked(var.toBool());
        settings.setValue(hlQuoteStyleKey,var.toBool());
    }

    var = settings.value(hlQuoteColorKey,Properties::Red);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlQuoteColor.setCurrentIndex(n);
        settings.setValue(hlQuoteColorKey,n);
    }

    QLabel *lLineComStyle = new QLabel(tr("Line Comments"));
    hlayout->addWidget(lLineComStyle,hlrow,0);
    hlLineComWeight.setText(tr("Bold"));
    hlLineComWeight.setChecked(false);
    hlayout->addWidget(&hlLineComWeight,hlrow,1);
    hlLineComStyle.setText(tr("Italic"));
    hlLineComStyle.setChecked(false);
    hlayout->addWidget(&hlLineComStyle,hlrow,2);
    addHighlights(&hlLineComColor, propertyColors);
    hlayout->addWidget(&hlLineComColor,hlrow,3);
    hlLineComColor.setCurrentIndex(Properties::Green);
    hlrow++;

    var = settings.value(hlLineComWeightKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlLineComWeight.setChecked(var.toBool());
        settings.setValue(hlLineComWeightKey,var.toBool());
    }

    var = settings.value(hlLineComStyleKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlLineComStyle.setChecked(var.toBool());
        settings.setValue(hlLineComStyleKey,var.toBool());
    }

    var = settings.value(hlLineComColorKey,Properties::DarkGreen);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlLineComColor.setCurrentIndex(n);
        settings.setValue(hlLineComColorKey,n);
    }

    QLabel *lBlockComStyle = new QLabel(tr("Block Comments"));
    hlayout->addWidget(lBlockComStyle,hlrow,0);
    hlBlockComWeight.setText(tr("Bold"));
    hlBlockComWeight.setChecked(false);
    hlayout->addWidget(&hlBlockComWeight,hlrow,1);
    hlBlockComStyle.setText(tr("Italic"));
    hlBlockComStyle.setChecked(false);
    hlayout->addWidget(&hlBlockComStyle,hlrow,2);
    addHighlights(&hlBlockComColor, propertyColors);
    hlayout->addWidget(&hlBlockComColor,hlrow,3);
    hlBlockComColor.setCurrentIndex(Properties::Green);
    hlrow++;

    var = settings.value(hlBlockComWeightKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlBlockComWeight.setChecked(var.toBool());
        settings.setValue(hlBlockComWeightKey,var.toBool());
    }

    var = settings.value(hlBlockComStyleKey,false);
    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        hlBlockComStyle.setChecked(var.toBool());
        settings.setValue(hlBlockComStyleKey,var.toBool());
    }

    var = settings.value(hlBlockComColorKey,Properties::DarkGreen);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        hlBlockComColor.setCurrentIndex(n);
        settings.setValue(hlBlockComColorKey,n);
    }

}

Qt::GlobalColor Properties::getQtColor(int index)
{
    if(index > -1 && index < propertyColors.count()) {
        return static_cast<PColor*>(propertyColors.at(index))->getValue();
    }
    return Qt::black; // just return black on failure
}

int  Properties::setComboIndexByValue(QComboBox *combo, QString value)
{
    for(int n = combo->count()-1; n > -1; n--) {
        if(combo->itemText(n).compare(value) == 0) {
            combo->setCurrentIndex(n);
            return n;
        }
    }
    return -1;
}

void Properties::browseCompiler()
{
    QString compiler = leditCompiler->text();
    if(compiler.length() < 1)
        compiler = mypath;

#if defined(Q_WS_WIN32)
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Propeller Compiler"), compiler, "Compiler (propeller-elf-gcc.exe)");
#else
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Propeller Compiler"), compiler, "Compiler (propeller-elf-gcc)");
#endif

    QString s = QDir::fromNativeSeparators(fileName);
    compilerstr = leditCompiler->text();
    if(s.length() > 0) {
        mypath = s;
        leditCompiler->setText(s);
        if(s.lastIndexOf("/bin/") > 0) {
            s = s.mid(0,s.lastIndexOf("/bin/"))+"/propeller-load/";
            mypath = s;
            leditIncludes->setText(mypath);
        }
    }
    qDebug() << "browseCompiler" << s;
}

void Properties::browseIncludes()
{
    QString pathName;
    QString path = mypath;

#if defined(Q_WS_WIN32)
    pathName = QFileDialog::getExistingDirectory(this,tr("Select Propeller Loader Folder"), path, QFileDialog::ShowDirsOnly);
#else
    pathName = QFileDialog::getExistingDirectory(this,tr("Select Propeller Loader Folder"), path, QFileDialog::ShowDirsOnly);
#endif

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    leditIncludes->setText(s);
    mypath = path;

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
    QString pathName;
    if(path.length() < 1)
        path = mypath;
#if defined(Q_WS_WIN32)
    pathName = QFileDialog::getExistingDirectory(this,tr("Select Project Workspace Folder"), path, QFileDialog::ShowDirsOnly);
#else
    pathName = QFileDialog::getExistingDirectory(this,tr("Select Project Workspace Folder"), path, QFileDialog::ShowDirsOnly);
#endif

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }

    leditWorkspace->setText(s);
    settings.setValue(workspaceKey, s);
}

void Properties::accept()
{
    QSettings settings(publisherKey, ASideGuiKey,this);

    settings.setValue(compilerKey,leditCompiler->text());
    settings.setValue(includesKey,leditIncludes->text());
    settings.setValue(configFileKey,leditIncludes->text());
    settings.setValue(workspaceKey,leditWorkspace->text());

    settings.setValue(tabSpacesKey,tabSpaces.text());
    settings.setValue(loadDelayKey,loadDelay.text());
    settings.setValue(resetTypeKey,resetType.currentIndex());

    settings.setValue(hlNumStyleKey,hlNumStyle.isChecked());
    settings.setValue(hlNumWeightKey,hlNumWeight.isChecked());
    settings.setValue(hlNumColorKey,hlNumColor.currentIndex());
    settings.setValue(hlFuncStyleKey,hlFuncStyle.isChecked());
    settings.setValue(hlFuncWeightKey,hlFuncWeight.isChecked());
    settings.setValue(hlFuncColorKey,hlFuncColor.currentIndex());
    settings.setValue(hlKeyWordStyleKey,hlKeyWordStyle.isChecked());
    settings.setValue(hlKeyWordWeightKey,hlKeyWordWeight.isChecked());
    settings.setValue(hlKeyWordColorKey,hlKeyWordColor.currentIndex());
    settings.setValue(hlPreProcStyleKey,hlPreProcStyle.isChecked());
    settings.setValue(hlPreProcWeightKey,hlPreProcWeight.isChecked());
    settings.setValue(hlPreProcColorKey,hlPreProcColor.currentIndex());
    settings.setValue(hlQuoteStyleKey,hlQuoteStyle.isChecked());
    settings.setValue(hlQuoteWeightKey,hlQuoteWeight.isChecked());
    settings.setValue(hlQuoteColorKey,hlQuoteColor.currentIndex());
    settings.setValue(hlLineComStyleKey,hlLineComStyle.isChecked());
    settings.setValue(hlLineComWeightKey,hlLineComWeight.isChecked());
    settings.setValue(hlLineComColorKey,hlLineComColor.currentIndex());
    settings.setValue(hlBlockComStyleKey,hlBlockComStyle.isChecked());
    settings.setValue(hlBlockComWeightKey,hlBlockComWeight.isChecked());
    settings.setValue(hlBlockComColorKey,hlBlockComColor.currentIndex());

    done(QDialog::Accepted);
}

void Properties::reject()
{
    // TODO: restore everything to settings values.
    leditCompiler->setText(compilerstr);
    leditIncludes->setText(includesstr);
    leditWorkspace->setText(workspacestr);

    tabSpaces.setText(tabSpacesStr);
    loadDelay.setText(loadDelayStr);
    resetType.setCurrentIndex(resetTypeEnum);
    hlNumStyle.setChecked(hlNumStyleBool);
    hlNumWeight.setChecked(hlNumWeightBool);
    hlNumColor.setCurrentIndex(hlNumColorIndex);
    hlFuncStyle.setChecked(hlFuncStyleBool);
    hlFuncStyle.setChecked(hlFuncWeightBool);
    hlFuncColor.setCurrentIndex(hlFuncColorIndex);
    hlKeyWordStyle.setChecked(hlKeyWordStyleBool);
    hlKeyWordWeight.setChecked(hlKeyWordWeightBool);
    hlKeyWordColor.setCurrentIndex(hlKeyWordColorIndex);
    hlPreProcStyle.setChecked(hlPreProcStyleBool);
    hlPreProcWeight.setChecked(hlPreProcWeightBool);
    hlPreProcColor.setCurrentIndex(hlPreProcColorIndex);
    hlQuoteStyle.setChecked(hlQuoteStyleBool);
    hlQuoteWeight.setChecked(hlQuoteWeightBool);
    hlQuoteColor.setCurrentIndex(hlQuoteColorIndex);
    hlLineComStyle.setChecked(hlLineComStyleBool);
    hlLineComWeight.setChecked(hlLineComWeightBool);
    hlLineComColor.setCurrentIndex(hlLineComColorIndex);
    hlBlockComStyle.setChecked(hlBlockComStyleBool);
    hlBlockComWeight.setChecked(hlBlockComWeightBool);
    hlBlockComColor.setCurrentIndex(hlBlockComColorIndex);

    done(QDialog::Rejected);
}

void Properties::showProperties()
{
    compilerstr = leditCompiler->text();
    includesstr = leditIncludes->text();
    workspacestr = leditWorkspace->text();
    tabSpacesStr = tabSpaces.text();
    loadDelayStr = loadDelay.text();
    resetTypeEnum = (Reset)resetType.currentIndex();
    hlNumStyleBool = hlNumStyle.isChecked();
    hlNumWeightBool = hlNumWeight.isChecked();
    hlNumColorIndex = hlNumColor.currentIndex();
    hlFuncStyleBool = hlFuncStyle.isChecked();
    hlFuncWeightBool = hlFuncStyle.isChecked();
    hlFuncColorIndex = hlFuncColor.currentIndex();
    hlKeyWordStyleBool = hlKeyWordStyle.isChecked();
    hlKeyWordWeightBool = hlKeyWordWeight.isChecked();
    hlKeyWordColorIndex = hlKeyWordColor.currentIndex();
    hlPreProcStyleBool = hlPreProcStyle.isChecked();
    hlPreProcWeightBool = hlPreProcWeight.isChecked();
    hlPreProcColorIndex = hlPreProcColor.currentIndex();
    hlQuoteStyleBool = hlQuoteStyle.isChecked();
    hlQuoteWeightBool = hlQuoteWeight.isChecked();
    hlQuoteColorIndex = hlQuoteColor.currentIndex();
    hlLineComStyleBool = hlLineComStyle.isChecked();
    hlLineComWeightBool = hlLineComWeight.isChecked();
    hlLineComColorIndex = hlLineComColor.currentIndex();
    hlBlockComStyleBool = hlBlockComStyle.isChecked();
    hlBlockComWeightBool = hlBlockComWeight.isChecked();
    hlBlockComColorIndex = hlBlockComColor.currentIndex();

    this->show();
    this->raise();
    this->activateWindow();
}

int Properties::getTabSpaces()
{
    return tabSpaces.text().toInt();
}

int Properties::getLoadDelay()
{
    return loadDelay.text().toInt();
}

Properties::Reset Properties::getResetType()
{
    return (Reset) resetType.currentIndex();
}
