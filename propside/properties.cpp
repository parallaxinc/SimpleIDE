#include "properties.h"
#include "directory.h"

#define ENABLE_CLEAR_AND_EXIT
/*
 * get propeller-elf-gcc path, propeller-load directory path, and user workspace path.
 */
Properties::Properties(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(QString(ASideGuiKey)+tr(" Properties"));

    /* clean for testing only */
    // cleanSettings();

    setupFolders(); // always call this before setupSpinFolders();
    setupSpinFolders();
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
    resize(500,260);
}

void Properties::cleanSettings()
{
    QSettings settings(publisherKey, ASideGuiKey);
    QStringList list = settings.allKeys();

    foreach(QString key, list) {
        if(key.indexOf(ASideGuiKey) == 0) {
            settings.remove(key);
        }
    }

    settings.remove(publisherComKey);
    settings.remove(publisherKey);
}

void Properties::setupFolders()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QFrame *box = new QFrame();
    box->setLayout(layout);
    tabWidget.addTab(box," GCC Folders ");

    QGroupBox *gbCompiler = new QGroupBox(tr("GCC Compiler"), this);
    QGroupBox *gbLibrary = new QGroupBox(tr("Library Folder"), this);
    QGroupBox *gbWorkspace = new QGroupBox(tr("Workspace Folder"), this);

    QPushButton *btnCompilerBrowse = new QPushButton(tr("Browse"), this);
    leditGccCompiler = new QLineEdit(this);
    QHBoxLayout *clayout = new QHBoxLayout();
    clayout->addWidget(leditGccCompiler);
    clayout->addWidget(btnCompilerBrowse);

    QPushButton *btnLibraryBrowse = new QPushButton(tr("Browse"), this);
    leditGccLibrary = new QLineEdit(this);
    QHBoxLayout *ilayout = new QHBoxLayout();
    ilayout->addWidget(leditGccLibrary);
    ilayout->addWidget(btnLibraryBrowse);

    QPushButton *btnWorkspaceBrowse = new QPushButton(tr("Browse"), this);
    leditGccWorkspace = new QLineEdit(this);
    QHBoxLayout *wlayout = new QHBoxLayout();
    wlayout->addWidget(leditGccWorkspace);
    wlayout->addWidget(btnWorkspaceBrowse);

    connect(btnCompilerBrowse,  SIGNAL(clicked()), this, SLOT(browseGccCompiler()));
    connect(btnLibraryBrowse,  SIGNAL(clicked()), this, SLOT(browseGccLibrary()));
    connect(btnWorkspaceBrowse, SIGNAL(clicked()), this, SLOT(browseGccWorkspace()));

    gbCompiler->setLayout(clayout);
    gbLibrary->setLayout(ilayout);
    gbWorkspace->setLayout(wlayout);

    layout->addWidget(gbCompiler);
    layout->addWidget(gbLibrary);
    layout->addWidget(gbWorkspace);

    setupPropGccWorkspace();
    setupPropGccCompiler();
}

/*
 * get the pre-packaged read-only application workspace.
 */
QString Properties::getApplicationWorkspace()
{
    QSettings settings(publisherKey, ASideGuiKey);

    /*
     * By convention in Windows we keep a SimpleIDE workspace in
     *   "Program Files\SimpleIDE\Workspace"
     * In Mac it's in the same level as SimpleIDE.app.
     * For development it can be set to another value.
     */
    QString pkwrk = QApplication::applicationDirPath()+"/";
#if defined(Q_WS_MAC)
    pkwrk += "../../";
#endif
    pkwrk += "../Workspace/";

    QVariant pkgv  = settings.value(packageKey, pkwrk);
    if(pkgv.canConvert(QVariant::String)) {
        QString s = pkgv.toString();
        if(s.length() > 0)
            settings.setValue(packageKey, s);
    }
    else {
        settings.setValue(packageKey,pkwrk);
    }
    return pkwrk;
}

void Properties::setupPropGccWorkspace()
{
    QSettings settings(publisherKey, ASideGuiKey);

    QString pkwrk = this->getApplicationWorkspace();

    QString mywrk = QDir::homePath()+"/";

    if(QFile::exists(mywrk+"Documents")) {
        mywrk = mywrk +"Documents/";
    }
    else if(QFile::exists(mywrk+"My Documents")) {
        mywrk = mywrk +"My Documents/";
    }

    mywrk += "SimpleIDE/";

    QDir wrkd(mywrk);
    if(wrkd.exists(mywrk) == false) {
        //wrkd.mkdir(mywrk); recursive copy does it
        Directory::recursiveCopyDir(pkwrk, mywrk);
    }

    QString mylib;
    if(QFile::exists(mywrk+"Learn/Simple Libraries")) {
        mylib = mywrk+"Learn/Simple Libraries";
    }

    QVariant libv  = settings.value(gccLibraryKey, mylib);
    QVariant wrkv  = settings.value(gccWorkspaceKey, mywrk);

    fileStringProperty(&wrkv,  leditGccWorkspace, gccWorkspaceKey, &mywrk);
    fileStringProperty(&libv,  leditGccLibrary,   gccLibraryKey,   &mylib);

    settings.setValue(gccLibraryKey,leditGccLibrary->text());
    settings.setValue(gccWorkspaceKey,leditGccWorkspace->text());
}

void Properties::setupPropGccCompiler()
{
    QSettings settings(publisherKey, ASideGuiKey);

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

    QVariant compv = settings.value(gccCompilerKey);

    if(QFile::exists(mygcc)) {
        qDebug() << "Found Default Compiler.";
    }
    else {
        if(compv.canConvert(QVariant::String)) {
            QString s = compv.toString();
            if(s.length() > 0) {
                mygcc = QDir::fromNativeSeparators(s);
                mypath = mygcc.mid(0,mygcc.lastIndexOf("/bin")+1);
            }
        }
    }
    fileStringProperty(&compv, leditGccCompiler,  gccCompilerKey,  &mygcc);
    settings.setValue(gccCompilerKey,mygcc);
}

void Properties::setupSpinFolders()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QFrame *box = new QFrame();
    box->setLayout(layout);
    tabWidget.addTab(box," "+tr("Spin Folders")+" ");

    QGroupBox *gbCompiler = new QGroupBox(tr("Spin Compiler"), this);
    QGroupBox *gbLibrary  = new QGroupBox(tr("Library Folder"), this);
    QGroupBox *gbWorkspace = new QGroupBox(tr("Workspace Folder"), this);

    QPushButton *btnCompilerBrowse = new QPushButton(tr("Browse"), this);
    leditSpinCompiler = new QLineEdit(this);
    QHBoxLayout *clayout = new QHBoxLayout();
    clayout->addWidget(leditSpinCompiler);
    clayout->addWidget(btnCompilerBrowse);

    QPushButton *btnLibraryBrowse = new QPushButton(tr("Browse"), this);
    leditSpinLibrary = new QLineEdit(this);
    QHBoxLayout *ilayout = new QHBoxLayout();
    ilayout->addWidget(leditSpinLibrary);
    ilayout->addWidget(btnLibraryBrowse);

    QPushButton *btnWorkspaceBrowse = new QPushButton(tr("Browse"), this);
    leditSpinWorkspace = new QLineEdit(this);
    QHBoxLayout *wlayout = new QHBoxLayout();
    wlayout->addWidget(leditSpinWorkspace);
    wlayout->addWidget(btnWorkspaceBrowse);

    connect(btnCompilerBrowse, SIGNAL(clicked()), this, SLOT(browseSpinCompiler()));
    connect(btnLibraryBrowse, SIGNAL(clicked()), this, SLOT(browseSpinLibrary()));
    connect(btnWorkspaceBrowse, SIGNAL(clicked()), this, SLOT(browseSpinWorkspace()));

    gbCompiler->setLayout(clayout);
    gbLibrary->setLayout(ilayout);
    gbWorkspace->setLayout(wlayout);

    layout->addWidget(gbCompiler);
    layout->addWidget(gbLibrary);
    layout->addWidget(gbWorkspace);

    QSettings settings(publisherKey, ASideGuiKey,this);

    QVariant gv = settings.value(gccCompilerKey,"");
    QString mygcc;

    if(gv.canConvert(QVariant::String)) {
        QString s = gv.toString();
        s = QDir::fromNativeSeparators(s);
        mygcc = s;
    }

    // setupFolders() sets mygcc
    QString myspin = mygcc.mid(0,mygcc.lastIndexOf("/"))+"/";

    if(QFile::exists(myspin+"spin")) {
        myspin += "spin";
    }
    else if(QFile::exists(myspin+"spin.exe")) {
        myspin += "spin.exe";
    }
    else if(QFile::exists(myspin+"bstc")) {
        myspin += "bstc";
    }
    else if(QFile::exists(myspin+"bstc.exe")) {
        myspin += "bstc.exe";
    }
    else if(QFile::exists(myspin+"bstc.linux")) {
        myspin += "bstc.linux";
    }
    else if(QFile::exists(myspin+"bstc.osx")) {
        myspin += "bstc.osx";
    }
    else {
        qDebug() << "Default Spin Compiler not found.";
    }

    QString mylib(myspin);
    if(mylib.lastIndexOf("/") == mylib.length()-1)
        mylib = mylib.left(mylib.length()-1);
    if(myspin.contains("/bin",Qt::CaseInsensitive))
        mylib = mylib.left(mylib.lastIndexOf("/bin")+1)+"spin/";

    QVariant compv = settings.value(spinCompilerKey,myspin);
    QVariant incv = settings.value(spinLibraryKey,mylib);
    QVariant wrkv = settings.value(spinWorkspaceKey);

    fileStringProperty(&compv, leditSpinCompiler, spinCompilerKey, &myspin);
    fileStringProperty(&incv,  leditSpinLibrary,  spinLibraryKey,  &mylib);

    QString mywrk;
    if(wrkv.canConvert(QVariant::String)) {
        mywrk = wrkv.toString();
        mywrk = QDir::fromNativeSeparators(mywrk);
    }

    fileStringProperty(&wrkv,  leditSpinWorkspace, spinWorkspaceKey, &mywrk);

    if(mywrk.length() == 0) {
        leditSpinWorkspace->setText(this->leditGccWorkspace->text());
    }

    this->spinCompilerStr = leditSpinCompiler->text();
    this->spinLibraryStr = leditSpinLibrary->text();
    this->spinWorkspaceStr = leditSpinWorkspace->text();
}

void Properties::fileStringProperty(QVariant *var, QLineEdit *ledit, const char *key, QString *value)
{
    QSettings settings(publisherKey, ASideGuiKey,this);
    if(var->canConvert(QVariant::String)) {
        QString s = var->toString();
        if(s.length() > 0) {
            s = QDir::fromNativeSeparators(s);
            ledit->setText(s);
        }
        else {
            ledit->setText(*value);
            settings.setValue(key,*value);
        }
    }
    else {
        ledit->setText(*value);
        settings.setValue(key,*value);
    }
}

void Properties::setupGeneral()
{
    int row = 0;

    QFrame *tbox = new QFrame();

    tabWidget.addTab(tbox," "+tr("General")+" ");

    QVBoxLayout *glayout = new QVBoxLayout();
    tbox->setLayout(glayout);

    QSettings settings(publisherKey, ASideGuiKey,this);
    QVariant var;

    QGroupBox *gbLoader = new QGroupBox(tr("Loader Folder"), tbox);

    QPushButton *btnLoaderBrowse = new QPushButton(tr("Browse"), this);
    leditLoader = new QLineEdit(this);
    QHBoxLayout *clayout = new QHBoxLayout();
    clayout->addWidget(leditLoader);
    clayout->addWidget(btnLoaderBrowse);

    QHBoxLayout *ilayout = new QHBoxLayout();
    ilayout->addWidget(leditLoader);
    ilayout->addWidget(btnLoaderBrowse);

    connect(btnLoaderBrowse,  SIGNAL(clicked()), this, SLOT(browseLoader()));

    QString myloader = mypath+"propeller-load/";
    if(QFile::exists(myloader))
        qDebug() << "Found Default Loader Path.";

    QVariant loadv = settings.value(propLoaderKey, myloader);

    fileStringProperty(&loadv, leditLoader, propLoaderKey, &myloader);

    settings.setValue(propLoaderKey, myloader);

    gbLoader->setLayout(ilayout);


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
    resetType.setCurrentIndex((int)DTR);
    tlayout->addWidget(&resetType,row++,1);

    var = settings.value(resetTypeKey,(int)DTR);
    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        resetType.setCurrentIndex(var.toInt());
    }

#ifdef ENABLE_CLEAR_AND_EXIT
    QLabel *lclear = new QLabel(tr("Clear program settings for exit."),tbox);
    tlayout->addWidget(lclear,row,0);
    QPushButton *clearSettings = new QPushButton(tr("Clear Settings"),this);
    clearSettings->setToolTip(tr("Clear settings for exit."));
    connect(clearSettings,SIGNAL(clicked()),this,SLOT(cleanSettings()));
    tlayout->addWidget(clearSettings,row,1);
#endif

#ifdef ENABLE_KEEP_ZIP_FOLDER
    keepZipFolder.setText(tr("Keep Zip Folder"));
    keepZipFolder.setChecked(false);
    tlayout->addWidget(&keepZipFolder, row++, 0);
#endif

    gbGeneral->setLayout(tlayout);
    glayout->addWidget(gbLoader);
    glayout->addWidget(gbGeneral);
}

void Properties::setupOptional()
{
    QFrame *tbox = new QFrame();

    tabWidget.addTab(tbox,tr("Optional"));

    QVBoxLayout *glayout = new QVBoxLayout();
    tbox->setLayout(glayout);

    QSettings settings(publisherKey, ASideGuiKey);
    QVariant var;

    QGroupBox *gbCompiler = new QGroupBox(tr("Spin Compiler"),tbox);
    leditSpinCompiler = new QLineEdit(this);

    QLabel *compLabel = new QLabel();
    // spin compiler either BSTC or Roy's SPIN compiler
    var = settings.value(spinCompilerKey);
    if(var.canConvert(QVariant::String)) {
        QString s = var.toString();
        if(s.length() > 0)
            leditSpinCompiler->setText(s);
    }
    else {
        leditSpinCompiler->setText(mypath+"bstc");
    }
    QPushButton *btnCompilerBrowse = new QPushButton(tr("Browse"), this);
    QHBoxLayout *clayout = new QHBoxLayout();
    clayout->addWidget(compLabel);
    clayout->addWidget(leditSpinCompiler);
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

    propertyColors.insert(PColor::Black, new PColor(tr("Black"), "Black", Qt::black));
    propertyColors.insert(PColor::DarkGray, new PColor(tr("Dark Gray"), "Dark Gray", Qt::darkGray));
    propertyColors.insert(PColor::Gray, new PColor(tr("Gray"), "Gray", Qt::gray));
    propertyColors.insert(PColor::LightGray, new PColor(tr("Light Gray"), "Light Gray", Qt::lightGray));
    propertyColors.insert(PColor::Blue, new PColor(tr("Blue"), "Blue", Qt::blue));
    propertyColors.insert(PColor::DarkBlue, new PColor(tr("Dark Blue"), "Dark Blue", Qt::darkBlue));
    propertyColors.insert(PColor::Cyan, new PColor(tr("Cyan"), "Cyan", Qt::cyan));
    propertyColors.insert(PColor::DarkCyan, new PColor(tr("Dark Cyan"), "Dark Cyan", Qt::darkCyan));
    propertyColors.insert(PColor::Green, new PColor(tr("Green"), "Green", Qt::green));
    propertyColors.insert(PColor::DarkGreen, new PColor(tr("Dark Green"), "Dark Green", Qt::darkGreen));
    propertyColors.insert(PColor::Magenta, new PColor(tr("Magenta"), "Magenta", Qt::magenta));
    propertyColors.insert(PColor::DarkMagenta, new PColor(tr("Dark Magenta"), "Dark Magenta", Qt::darkMagenta));
    propertyColors.insert(PColor::Red, new PColor(tr("Red"), "Red", Qt::red));
    propertyColors.insert(PColor::DarkRed, new PColor(tr("Dark Red"), "Dark Red", Qt::darkRed));
    propertyColors.insert(PColor::Yellow, new PColor(tr("Yellow"), "Yellow", Qt::yellow));
    propertyColors.insert(PColor::DarkYellow, new PColor(tr("Dark Yellow"), "Dark Yellow", Qt::darkYellow));
    propertyColors.insert(PColor::White, new PColor(tr("White"), "White", Qt::white));

    QStringList colorlist;
    for(int n = 0; n < propertyColors.count(); n++) {
        colorlist.append(static_cast<PColor*>(propertyColors[n])->getName());
    }

    QSettings settings(publisherKey, ASideGuiKey);
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
    hlNumColor.setCurrentIndex(PColor::Magenta);
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

    var = settings.value(hlNumColorKey,PColor::Magenta);
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
    hlFuncColor.setCurrentIndex(PColor::Blue);
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

    var = settings.value(hlFuncColorKey,PColor::Blue);
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
    hlKeyWordColor.setCurrentIndex(PColor::DarkBlue);
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

    var = settings.value(hlKeyWordColorKey,PColor::DarkBlue);
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
    hlPreProcColor.setCurrentIndex(PColor::DarkYellow);
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

    var = settings.value(hlPreProcColorKey,PColor::DarkYellow);
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
    hlQuoteColor.setCurrentIndex(PColor::Red);
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

    var = settings.value(hlQuoteColorKey,PColor::Red);
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
    hlLineComColor.setCurrentIndex(PColor::Green);
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

    var = settings.value(hlLineComColorKey,PColor::DarkGreen);
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
    hlBlockComColor.setCurrentIndex(PColor::Green);
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

    var = settings.value(hlBlockComColorKey,PColor::DarkGreen);
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

void Properties::browseGccCompiler()
{
    QString compiler = leditGccCompiler->text();
    if(compiler.length() < 1)
        compiler = mypath;

#if defined(Q_WS_WIN32)
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Propeller Compiler"), compiler, "Compiler (propeller-elf-gcc.exe)");
#else
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Propeller Compiler"), compiler, "Compiler (propeller-elf-gcc)");
#endif

    QString s = QDir::fromNativeSeparators(fileName);
    gccCompilerStr = leditGccCompiler->text();
    if(s.length() > 0) {
        mypath = s;
        leditGccCompiler->setText(s);
        if(s.lastIndexOf("/bin/") > 0) {
            s = s.mid(0,s.lastIndexOf("/bin/"))+"/propeller-load/";
            mypath = s;
            leditLoader->setText(mypath);
        }
    }
    qDebug() << "browseGccCompiler" << s;
}

void Properties::browseGccLibrary()
{
    QString pathName;
    gccLibraryStr = leditGccLibrary->text();
    if(gccLibraryStr.length() < 1)
        gccLibraryStr = mypath;

#if defined(Q_WS_WIN32)
    pathName = QFileDialog::getExistingDirectory(this,tr("Select GCC Library Folder"), gccLibraryStr, QFileDialog::ShowDirsOnly);
#else
    pathName = QFileDialog::getExistingDirectory(this,tr("Select GCC Library Folder"), gccLibraryStr, QFileDialog::ShowDirsOnly);
#endif

    QString s = QDir::fromNativeSeparators(pathName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    leditGccLibrary->setText(s);
    mypath = pathName;

    qDebug() << "browseGccLibrary" << s;
}

void Properties::browseGccWorkspace()
{
    QSettings settings(publisherKey, ASideGuiKey);
    QVariant vpath = settings.value(gccWorkspaceKey,QVariant("~/."));
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

    pathName = QFileDialog::getExistingDirectory(this,tr("Select GCC Project Workspace Folder"), path, QFileDialog::ShowDirsOnly);

    QString s = QDir::fromNativeSeparators(pathName);
    gccWorkspaceStr = leditGccWorkspace->text();
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }

    leditGccWorkspace->setText(s);
    settings.setValue(gccWorkspaceKey, s);
}

void Properties::browseSpinCompiler()
{
    QString compiler = leditSpinCompiler->text();
    if(compiler.length() < 1)
        compiler = mypath;

#if defined(Q_WS_WIN32)
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Spin Compiler"), compiler, "Compiler (spin.exe bstc.exe)");
#else
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Spin Compiler"), compiler, "Compiler (spin spin.* bstc bstc.*)");
#endif

    QString s = QDir::fromNativeSeparators(fileName);
    spinCompilerStr = leditSpinCompiler->text();
    if(s.length() > 0) {
        mypath = s;
        leditSpinCompiler->setText(s);
    }
    qDebug() << "browseSpinCompiler" << s;
}

void Properties::browseSpinLibrary()
{
    QString pathName;
    QString path = mypath;
    spinLibraryStr = leditSpinLibrary->text();
    if(spinLibraryStr.length() < 1)
        spinLibraryStr = mypath;

    pathName = QFileDialog::getExistingDirectory(this,tr("Select Spin Library Folder"), spinLibraryStr, QFileDialog::ShowDirsOnly);

    QString s = QDir::fromNativeSeparators(pathName);

    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    leditSpinLibrary->setText(s);
    mypath = pathName;

    qDebug() << "browseSpinLibrary" << s;
}

void Properties::browseSpinWorkspace()
{
    QSettings settings(publisherKey, ASideGuiKey);
    QVariant vpath = settings.value(spinWorkspaceKey,QVariant("~/."));
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

    pathName = QFileDialog::getExistingDirectory(this,tr("Select Spin Project Workspace Folder"), path, QFileDialog::ShowDirsOnly);

    QString s = QDir::fromNativeSeparators(pathName);
    spinWorkspaceStr = leditSpinWorkspace->text();
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    leditSpinWorkspace->setText(s);
    settings.setValue(spinWorkspaceKey, s);
}

void Properties::browseLoader()
{
    QString pathName;
    QString path = mypath + "propeller-load";
    loaderStr = leditLoader->text();
    if(loaderStr.length() < 1)
        loaderStr = path;

    pathName = QFileDialog::getExistingDirectory(this,tr("Select Propeller Loader Folder"), loaderStr, QFileDialog::ShowDirsOnly);

    QString s = QDir::fromNativeSeparators(pathName);
    loaderStr = leditLoader->text();
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    leditLoader->setText(s);

    qDebug() << "browseLoader" << s;
}

void Properties::accept()
{
    QSettings settings(publisherKey, ASideGuiKey);
    if(settings.allKeys().count() == 0) {
        done(QDialog::Accepted);
        return;
    }

    settings.setValue(gccCompilerKey,leditGccCompiler->text());
    settings.setValue(gccLibraryKey,leditGccLibrary->text());
    settings.setValue(gccWorkspaceKey,leditGccWorkspace->text());
    settings.setValue(spinCompilerKey,leditSpinCompiler->text());
    settings.setValue(spinLibraryKey,leditSpinLibrary->text());
    settings.setValue(spinWorkspaceKey,leditSpinWorkspace->text());
    settings.setValue(configFileKey,leditLoader->text());

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
    QSettings settings(publisherKey, ASideGuiKey);
    if(settings.allKeys().count() == 0) {
        done(QDialog::Rejected);
        return;
    }

    leditGccCompiler->setText(gccCompilerStr);
    leditGccLibrary->setText(gccLibraryStr);
    leditGccWorkspace->setText(gccWorkspaceStr);
    leditSpinCompiler->setText(spinCompilerStr);
    leditSpinLibrary->setText(spinLibraryStr);
    leditSpinWorkspace->setText(spinWorkspaceStr);
    leditLoader->setText(loaderStr);

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
    gccCompilerStr = leditGccCompiler->text();
    gccLibraryStr = leditGccLibrary->text();
    gccWorkspaceStr = leditGccWorkspace->text();
    spinCompilerStr = leditSpinCompiler->text();
    spinLibraryStr = leditSpinLibrary->text();
    spinWorkspaceStr = leditSpinWorkspace->text();
    loaderStr = leditLoader->text();

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

bool Properties::getKeepZipFolder()
{
    return this->keepZipFolder.isChecked();
}
