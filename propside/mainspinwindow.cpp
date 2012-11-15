/*
 * Copyright (c) 2012 Parallax, Inc.
 * Initial Code by John Steven Denson
 *
 * All Rights for this file are MIT Licensed.
 *

 TERMS OF USE: MIT License
 +--------------------------------------------------------------------
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 +--------------------------------------------------------------------
 */

#include "mainspinwindow.h"
#include "qextserialenumerator.h"
#include "qportcombobox.h"
#include "Sleeper.h"

#define SD_TOOLS
#define APPWINDOW_MIN_HEIGHT 480
#define APPWINDOW_MIN_WIDTH 480
#define EDITOR_MIN_WIDTH 500
#define PROJECT_WIDTH 270

#define SOURCE_FILE_TYPES "Source Files (*.c *.cpp *.h *.cogc *.ecogc *.spin *.espin);; All (*)"
#define PROJECT_FILE_FILTER "SIDE Project (*.side);; All (*)"

#define GDB_TABNAME "GDB Output"
#define SIDE_EXTENSION ".side"
#define SPIN_TEXT      "SPIN"
#define SPIN_EXTENSION ".spin"

#define NewFile "&New"
#define OpenFile "&Open"
#define SaveFile "&Save"
#define SaveAsFile "Save &As"

#define SaveAsProject "Save As Project"
#define CloneProject "Clone Project"

#define ProjectView "Set Project View"
#define SimpleView  "Set Simple View"

MainSpinWindow::MainSpinWindow(QWidget *parent) : QMainWindow(parent)
{
    /* setup application registry info */
    QCoreApplication::setOrganizationName(publisherKey);
    QCoreApplication::setOrganizationDomain(publisherComKey);
    QCoreApplication::setApplicationName(ASideGuiKey);

    /* global settings */
    settings = new QSettings(publisherKey, ASideGuiKey, this);

    /* get last geometry. using x,y,w,h is unreliable.
     */
    QVariant geov = settings->value(ASideGuiGeometry);
    // byte array convert is always possible
    QByteArray geo = geov.toByteArray();
    // restoreGeometry makes sure the array is valid
    this->restoreGeometry(geo);

    /* setup properties dialog */
    propDialog = new Properties(this);
    connect(propDialog,SIGNAL(accepted()),this,SLOT(propertiesAccepted()));

    /* detect user's startup view */
    simpleViewType = true;
    QVariant viewv = settings->value(simpleViewKey);
    if(viewv.canConvert(QVariant::Bool)) {
        simpleViewType = viewv.toBool();
    }

    /* setup user's editor font */
    QVariant fontv = settings->value(editorFontKey);
    if(fontv.canConvert(QVariant::String)) {
        QString family = fontv.toString();
        editorFont = QFont(family);
    }
    else {
        editorFont = QFont("Courier New", 10, QFont::Normal, false);
    }

    fontv = settings->value(fontSizeKey);
    if(fontv.canConvert(QVariant::Int)) {
        int size = fontv.toInt();
        editorFont.setPointSize(size);
    }
    else {
        editorFont.setPointSize(10);
    }


    /* setup new project dialog */
    newProjDialog = new NewProject(this);
    connect(newProjDialog,SIGNAL(accepted()),this,SLOT(newProjectAccepted()));

    /* setup find/replace dialog */
    replaceDialog = new ReplaceDialog(this);

    /* new ASideConfig class */
    aSideConfig = new ASideConfig();

    projectModel = NULL;
    referenceModel = NULL;

    /* main container */
    setWindowTitle(ASideGuiKey);
    QSplitter *vsplit = new QSplitter(this);
    setCentralWidget(vsplit);

    /* minimum window height */
    this->setMinimumHeight(APPWINDOW_MIN_HEIGHT);
    this->setMinimumWidth(APPWINDOW_MIN_WIDTH);

    /* project tools */
    setupProjectTools(vsplit);

    /* start with an empty file if fresh install */
    newFile();

    /* get app settings at startup and before any compiler call */
    getApplicationSettings();

    /* set up ctag tool */
    ctags = new CTags(aSideCompilerPath);

    /* setup gui components */
    setupFileMenu();
    setupHelpMenu();
    setupToolBars();

    /* show gui */
    QApplication::processEvents();

    /* get user's last open path */
    QVariant  lastfile = settings->value(lastFileNameKey);
    if(lastfile.canConvert(QVariant::String)) {
        lastPath = sourcePath(lastfile.toString());
    }

    initBoardTypes();

    /* start a process object for the loader to use */
    process = new QProcess(this);

    projectFile = "none";

    buildC = new BuildC(projectOptions, compileStatus, status, programSize, progress, cbBoard, propDialog);
#ifdef SPIN
    buildSpin = new BuildSpin(projectOptions, compileStatus, status, programSize, progress, cbBoard, propDialog);
#endif
    builder = buildC;

    /* setup loader and port listener */
    /* setup the terminal dialog box */
    term = new Terminal(this);
    termEditor = term->getEditor();

    QVariant gv = settings->value(termGeometryKey);
    if(gv.canConvert(QVariant::ByteArray)) {
        QByteArray geo = gv.toByteArray();
        term->restoreGeometry(geo);
    }

    /* tell port listener to use terminal editor for i/o */
    portListener = new PortListener(this, termEditor);
    portListener->setTerminalWindow(termEditor);

    term->setPortListener(portListener);

    //term->setWindowTitle(QString(ASideGuiKey)+" "+tr("Simple Terminal"));
    // education request that the window title be SimpleIDE Terminal
    term->setWindowTitle(QString(ASideGuiKey)+" "+tr("Terminal"));

    connect(term,SIGNAL(accepted()),this,SLOT(terminalClosed()));
    connect(term,SIGNAL(rejected()),this,SLOT(terminalClosed()));

    /* get available ports at startup */
    enumeratePorts();

    /* these are read once per app startup */
    QVariant lastportv  = settings->value(lastPortNameKey);
    if(lastportv.canConvert(QVariant::String))
        portName = lastportv.toString();

    /* setup the first port displayed in the combo box */
    if(cbPort->count() > 0) {
        int ndx = 0;
        if(portName.length() != 0) {
            for(int n = cbPort->count()-1; n > -1; n--)
                if(cbPort->itemText(n) == portName)
                {
                    ndx = n;
                    break;
                }
        }
        setCurrentPort(ndx);
    }

    this->show();
    QApplication::processEvents();

    /* load the last file into the editor to make user happy */
    QVariant lastfilev = settings->value(lastFileNameKey);
    if(!lastfilev.isNull()) {
        if(lastfilev.canConvert(QVariant::String)) {
            QString fileName = lastfilev.toString();
            if(fileName.length() > 0 && QFile::exists(fileName)) {
#ifndef SPIN
                if(fileName.mid(fileName.lastIndexOf(".")+1).contains("spin",Qt::CaseInsensitive)) {
                    QMessageBox::critical(
                            this,tr("SPIN Not Supported"),
                            tr("Spin projects are not supported with this version."),
                            QMessageBox::Ok);
                    return;
                }
#endif
                openFileName(fileName);
                setProject(); // last file is always first project
            }
        }
    }

    // old hardware dialog configuration feature
    //  hardwareDialog = new Hardware(this);
    //  connect(hardwareDialog,SIGNAL(accepted()),this,SLOT(initBoardTypes()));

    int tab = editorTabs->currentIndex();
    if(tab > -1) {
        Editor *ed = editors->at(tab);
        ed->setFocus();
        ed->raise();
    }

    this->show(); // show gui before about for mac
    QApplication::processEvents();

    /* show help dialog */
    QVariant helpStartup = settings->value(helpStartupKey,true);
    if(helpStartup.canConvert(QVariant::Bool)) {
        if(helpStartup == true)
            aboutShow();
    }

}

void MainSpinWindow::keyHandler(QKeyEvent* event)
{
    //qDebug() << "MainSpinWindow::keyHandler";
#if 1
    int key = termEditor->eventKey(event);
    if(key < 1) return;
#else
    int key = event->key();
    switch(key)
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        key = termEditor->getEnter();
        break;
    case Qt::Key_Backspace:
        key = '\b';
        break;
    case Qt::Key_Alt:
        return;
    case Qt::Key_Control:
        return;
    case Qt::Key_Shift:
        return;
    default:
        if(QApplication::keyboardModifiers() & Qt::CTRL) {
            key &= ~0xe0;
        }
        else {
            if(event->text().length() > 0) {
                QChar c = event->text().at(0);
                key = (int)c.toAscii();
            }
        }
        break;
    }
#endif

    QByteArray barry;
    barry.append((char)key);
    portListener->send(barry);
}

void MainSpinWindow::sendPortMessage(QString s)
{
    QByteArray barry;
    foreach(QChar c, s) {
        barry.append(c);
        portListener->send(barry);
        barry.clear();
        this->thread()->yieldCurrentThread();
    }
}

void MainSpinWindow::terminalEditorTextChanged()
{
    //QString text = termEditor->toPlainText();
}

/*
 * get the application settings from the registry for compile/startup
 */
void MainSpinWindow::getApplicationSettings()
{
    QFile file;
    QVariant compv = settings->value(compilerKey);

    if(compv.canConvert(QVariant::String))
        aSideCompiler = compv.toString();

    if(!file.exists(aSideCompiler))
    {
        propDialog->showProperties();
    }

    /* get the separator used at startup
     * Qt always translates \ to / so this isn't really necessary
     */
    QString appPath = QCoreApplication::applicationDirPath ();
    if(appPath.indexOf('\\') > -1)
        aSideSeparator = "\\";
    else
        aSideSeparator = "/";

    /* get the compiler path */
    if(aSideCompiler.indexOf('\\') > -1) {
        aSideCompilerPath = aSideCompiler.mid(0,aSideCompiler.lastIndexOf('\\')+1);
    }
    else if(aSideCompiler.indexOf('/') > -1) {
        aSideCompilerPath = aSideCompiler.mid(0,aSideCompiler.lastIndexOf('/')+1);
    }

#if defined(Q_WS_WIN32)
    aSideLoader = aSideCompilerPath + "propeller-load.exe";
#else
    aSideLoader = aSideCompilerPath + "propeller-load";
#endif

    /* get the include path and config file set by user */
    QVariant incv = settings->value(includesKey);
    QVariant cfgv = settings->value(configFileKey);

    /* convert registry values to strings */
    if(incv.canConvert(QVariant::String))
        aSideIncludes = incv.toString();

    if(cfgv.canConvert(QVariant::String))
        aSideCfgFile = cfgv.toString();

    /* get doc path from include path */
    QString tmp = aSideIncludes;
    if(tmp.length() > 0) {
        if(tmp.at(tmp.length()-1) == '/')
            tmp = tmp.left(tmp.length()-1);
        tmp = tmp.left(tmp.lastIndexOf("/")+1)+"share/lib/html";
        aSideDocPath = tmp;
    }

    if(!file.exists(aSideCfgFile))
    {
        propDialog->showProperties();
    }
    else
    {
        /* load boards in case there were changes */
        aSideConfig->loadBoards(aSideCfgFile);
    }

    QVariant wrkv = settings->value(workspaceKey);
    if(wrkv.canConvert(QVariant::String) == false) {
        propDialog->showProperties();
    }
}

void MainSpinWindow::exitSave()
{
    bool saveAll = false;
    QMessageBox mbox(QMessageBox::Question, tr("Save File?"), "",
                     QMessageBox::Discard | QMessageBox::Save | QMessageBox::SaveAll, this);

    saveProjectOptions();

    for(int tab = editorTabs->count()-1; tab > -1; tab--)
    {
        QString tabName = editorTabs->tabText(tab);
        if(tabName.at(tabName.length()-1) == '*')
        {
            mbox.setInformativeText(tr("Save File? ") + tabName.mid(0,tabName.indexOf(" *")));
            if(saveAll)
            {
                saveFileByTabIndex(tab);
            }
            else
            {
                int ret = mbox.exec();
                switch (ret) {
                    case QMessageBox::Discard:
                        // Don't Save was clicked
                        return;
                        break;
                    case QMessageBox::Save:
                        // Save was clicked
                        saveFileByTabIndex(tab);
                        break;
                    case QMessageBox::SaveAll:
                        // save all was clicked
                        saveAll = true;
                        break;
                    default:
                        // should never be reached
                        break;
                }
            }
        }
    }

}

void MainSpinWindow::closeEvent(QCloseEvent *event)
{
    if(event->type()) {}; // silence compiler
    quitProgram();
}

void MainSpinWindow::quitProgram()
{
    /* never leave port open */
    portListener->close();
    term->accept(); // just in case serial terminal is open

    programStopBuild();

    exitSave(); // find
    QString fileName = "";

    if(projectFile.isEmpty()) {
        fileName = editorTabs->tabToolTip(editorTabs->currentIndex());
        if(!fileName.isEmpty())
            settings->setValue(lastFileNameKey,fileName);
    }
    else {
        QFile proj(projectFile);
        if(proj.open(QFile::ReadOnly | QFile::Text)) {
            fileName = sourcePath(projectFile)+proj.readLine();
            fileName = fileName.trimmed();
            proj.close();
        }
        settings->setValue(lastFileNameKey,fileName);
        saveProjectOptions();
    }

    QString boardstr = cbBoard->itemText(cbBoard->currentIndex());
    QString portstr = cbPort->itemText(cbPort->currentIndex());

    settings->setValue(lastBoardNameKey,boardstr);
    settings->setValue(lastPortNameKey,portstr);

    QString fontstr = editorFont.toString();
    settings->setValue(editorFontKey,fontstr);

    int fontsize = editorFont.pointSize();
    settings->setValue(fontSizeKey,fontsize);

    // save user's width/height
    QByteArray geo = this->saveGeometry();
    settings->setValue(ASideGuiGeometry,geo);

    delete replaceDialog;
    delete propDialog;
    delete projectOptions;
    delete term;

    qApp->exit(0);
}

void MainSpinWindow::newFile()
{
    fileChangeDisable = true;
    setupEditor();
    int tab = editors->count()-1;
    editorTabs->addTab(editors->at(tab),(const QString&)untitledstr);
    editorTabs->setCurrentIndex(tab);
    Editor *ed = editors->at(tab);
    ed->setFocus();
    fileChangeDisable = false;
}

void MainSpinWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull()) {
//        fileName = fileDialog.getOpenFileName(this, tr("Open File"), lastPath, tr(SOURCE_FILE_TYPES));
//#endif
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), lastPath, SOURCE_FILE_TYPES); //"All (*)");
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
    }
#if 1
    if(fileName.indexOf(SIDE_EXTENSION) > 0) {
        // save old project options before loading new one
        saveProjectOptions();
        // load new project
        projectFile = fileName;
        setCurrentProject(projectFile);
        QFile proj(projectFile);
        if(proj.open(QFile::ReadOnly | QFile::Text)) {
            fileName = sourcePath(projectFile)+proj.readLine();
            fileName = fileName.trimmed();
            proj.close();
        }
#ifndef SPIN
        if(fileName.mid(fileName.lastIndexOf(".")+1).contains("spin",Qt::CaseInsensitive)) {
            QMessageBox::critical(
                    this,tr("SPIN Not Supported"),
                    tr("Spin projects are not supported with this version."),
                    QMessageBox::Ok);
            return;
        }
#endif
        updateProjectTree(fileName);
    }
    else
#endif
    {
        if(fileName.length())
            setCurrentFile(fileName);
    }
    openFileName(fileName);

    /* for old project manager method only
    if(projectFile.length() == 0) {
        setProject();
    }
    else if(editorTabs->count() == 1) {
        setProject();
    }
    */
}

bool MainSpinWindow::isFileUTF16(QFile *file)
{
    char str[2];
    file->read(str,2);
    file->seek(0);
    if(str[0] == -1 && str[1] == -2) {
        return true;
    }
    return false;
}

void MainSpinWindow::openFileName(QString fileName)
{
    QString data;
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly))
        {
            QTextStream in(&file);
            if(this->isFileUTF16(&file))
                in.setCodec("UTF-16");
            else
                in.setCodec("UTF-8");
            data = in.readAll();
            file.close();
            data = data.replace('\t',"    ");
            QString sname = this->shortFileName(fileName);
            if(editorTabs->count()>0) {
                for(int n = editorTabs->count()-1; n > -1; n--) {
                    if(editorTabs->tabText(n) == sname) {
                        setEditorTab(n, sname, fileName, data);
                        return;
                    }
                }
            }
            if(editorTabs->tabText(0).contains(untitledstr)) {
                setEditorTab(0, sname, fileName, data);
                return;
            }
            newFile();
            setEditorTab(editorTabs->count()-1, sname, fileName, data);
        }
    }
}

/*
 * this is similar to when a user clicks a tab's X
 */
void MainSpinWindow::closeFile()
{
    int tab = editorTabs->currentIndex();
    if(tab > -1)
        closeTab(tab);
}

/*
 * close project if open and close all tabs.
 * do exitSave function and close all windows.
 */
void MainSpinWindow::closeAll()
{
    saveProjectOptions();
    setWindowTitle(QString(ASideGuiKey));
    // this->projectOptions->clearOptions();
    if(projectModel != NULL) {
        delete projectModel;
        projectModel = NULL;
    }
    for(int tab = editorTabs->count()-1; tab > -1; tab--)
        closeTab(tab);
}

/*
 * New project should start a project wizard.
 * First generation wizard should just ask for
 * project name and project folder (workspace?).
 * Wizard should always create a project.c
 * file that will contain main.
 */
void MainSpinWindow::newProject()
{
    newProjDialog->showDialog();
}

void MainSpinWindow::newProjectAccepted()
{
    QString name = newProjDialog->getName();
    QString path = newProjDialog->getPath();
    QDir dir(path);

    QString comp = newProjDialog->getCompilerType();

    qDebug() << "Project Name:" << name;
    qDebug() << "Project Compiler:" << comp;
    qDebug() << "Project Path:" << path;

    QString C_maintemplate("/**\n" \
         " * @file "+name+".c\n" \
         " * This is the main "+name+" program start point.\n" \
         " */\n" \
         "\n" \
         "/**\n" \
         " * Main program function.\n" \
         " */\n" \
         "int main(void)\n" \
         "{\n" \
         "    return 0;\n" \
         "}\n" \
         "\n");

    QString Cpp_maintemplate("/**\n" \
         " * @file "+name+".cpp\n" \
         " * This is the main "+name+" program start point.\n" \
         " */\n" \
         "\n" \
         "/**\n" \
         " * Main program function.\n" \
         " */\n" \
         "int main(void)\n" \
         "{\n" \
         "    return 0;\n" \
         "}\n" \
         "\n");
#ifdef SPIN
    QString SPIN_maintemplate("{{\n" \
         " * @file "+name+".spin\n" \
         " * This is the main "+name+" program start point.\n" \
         "}} \n" \
         "\n" \
         "{{\n" \
         " * Main program function.\n" \
         "}}\n" \
         "pub main\n" \
         "\n" \
         "    repeat\n" \
         "\n" \
         "\n");
#endif
    QString mains;
    QString mainName(path+"/"+name);
    
    if(comp.compare("C", Qt::CaseInsensitive) == 0) {
        mains = C_maintemplate;
        mainName += ".c";
        projectOptions->setCompiler("C");
    }
    else if(comp.compare("C++", Qt::CaseInsensitive) == 0) {
        mains = Cpp_maintemplate;
        mainName += ".cpp";
        projectOptions->setCompiler("C++");
    }
#ifdef SPIN
    else if(comp.compare("Spin", Qt::CaseInsensitive) == 0) {
        mains = SPIN_maintemplate;
        mainName += SPIN_EXTENSION;
        projectOptions->setCompiler(SPIN_TEXT);
    }
    else
#endif
    {
        return;
    }

    qDebug() << "Project Start File: " << mainName;

    closeProject();

    if(dir.exists(path) == 0)
        dir.mkdir(path);

    QFile mainfile(mainName);
    if(mainfile.exists() == false) {
        QTextStream os(&mainfile);
        os.setCodec("UTF-8"); // for now save everything as UTF-8
        if(mainfile.open(QFile::ReadWrite)) {
            os << mains;
            mainfile.close();
        }
    }

    projectFile = path+"/"+name+SIDE_EXTENSION;
    qDebug() << "Project File: " << projectFile;
    setCurrentProject(projectFile);
    qDebug() << "Update Project: " << projectFile;
    updateProjectTree(mainName);
    qDebug() << "Open Project File: " << projectFile;
    openFile(projectFile);
    qDebug() << "Set Compiler: " << comp;
    projectOptions->setCompiler(comp);
    qDebug() << "Save Project File: " << projectFile;
    saveProjectOptions();
}

void MainSpinWindow::openProject(const QString &path)
{
    QString fileName = path;

#ifndef SPIN
        if(fileName.mid(fileName.lastIndexOf(".")+1).contains("spin",Qt::CaseInsensitive)) {
            QMessageBox::critical(
                    this,tr("SPIN Not Supported"),
                    tr("Spin projects are not supported with this version."),
                    QMessageBox::Ok);
            return;
        }
#endif

    if (fileName.isNull()) {
        fileName = fileDialog.getOpenFileName(this, tr("Open Project"), lastPath, "Project Files (*.side)");
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
    }
    if(fileName.indexOf(SIDE_EXTENSION) > 0) {
        // save and close old project options before loading new one
        closeProject();
        // load new project
        projectFile = fileName;
        setCurrentProject(projectFile);
        QFile proj(projectFile);
        if(proj.open(QFile::ReadOnly | QFile::Text)) {
            fileName = sourcePath(projectFile)+proj.readLine();
            fileName = fileName.trimmed();
            proj.close();
        }

#ifndef SPIN
        if(fileName.mid(fileName.lastIndexOf(".")+1).contains("spin",Qt::CaseInsensitive)) {
            QMessageBox::critical(
                    this,tr("SPIN Not Supported"),
                    tr("Spin projects are not supported with this version."),
                    QMessageBox::Ok);
            return;
        }
#endif

        updateProjectTree(fileName);
    }
    openFileName(fileName);
    /* for old project manager method only
    if(projectFile.length() == 0) {
        setProject();
    }
    else if(editorTabs->count() == 1) {
        setProject();
    }
    */

}

// find the difference between s1 and s2. i.e. result = s2 - s1
QString MainSpinWindow::pathDiff(QString s2, QString s1)
{
    s1 = s1.replace("\\","/");
    s2 = s2.replace("\\","/");
    QString result(s2);
    while(s2.contains(s1,Qt::CaseInsensitive) == false)
        s1 = s1.mid(0,s1.lastIndexOf("/"));
    result = result.replace(s1,"");
    return result;
}

/*
 * fixup project links.
 *
 * 1. if link is a full path (not link), make it a relative link
 * 2. adjust link to target project folder
 */
QString MainSpinWindow::saveAsProjectLinkFix(QString srcPath, QString dstPath, QString link)
{
    QString fix = "";

    link = link.replace("\\","/");
    srcPath = srcPath.replace("\\","/");
    dstPath = dstPath.replace("\\","/");

    /*
     * Two important cases:
     * 1. link is relative ../
     * 2. link is absolute /
     */
    QString fs;
    QDir path(dstPath);

    if(link.left(3) == "../") {
        if(QFile::exists(srcPath+link) != true) {
            return fix;
        }
        QFile file(srcPath+link);
        fs = file.fileName();
        fs = path.relativeFilePath(fs);
        fix = fs;
    }
    else {
        if(QFile::exists(link) != true) {
            return fix;
        }
        fs = link;
        fs = path.relativeFilePath(fs);
        fix = fs;
    }
    return fix;
}

/*
 * Save As project: saves a copy of the project in another path
 *
 * 1. function assumes an empty projectFolder parameter means to copy existing project
 * 2. asks user for destination project name and folder (folder and name can be different)
 * 3. creates new project folder if necessary (project can be in original folder)
 * 4. copy project file from source to destination as new project name
 * 5. copies the project main file from source to destination as new project main file
 *    fixes up any links in project file and writes file
 *
 */
void MainSpinWindow::saveAsProject(const QString &inputProjFile)
{
    bool ok;
    QString projFolder(sourcePath(inputProjFile));
    QString projFile = inputProjFile;
    /*
     * 1. function assumes an empty projectFolder parameter means to copy existing project.
     * if projectFolder is empty saveAs from existing project.
     */
    if(projFolder.length() == 0) {
        projFolder = sourcePath(projectFile);
    }
    if(projFile.length() == 0) {
        projFile = projectFile;
    }

    if(projFile.length() == 0) {
        QMessageBox::critical(
                this,tr("No Project"),
                tr("Can't \"Save As Project\" from an empty project.")+"\n"+
                tr("Please create a new project or open an existing one."),
                QMessageBox::Ok);
        return;
    }

    QString dstName;
    QString srcPath  = projFolder;
    QDir spath(srcPath);

    if(spath.exists() == false) {
        QMessageBox::critical(
                this,tr("Project Folder not Found."),
                tr("Can't \"Save As Project\" from a non-existing folder."),
                QMessageBox::Ok);
        return;
    }

    /*
     * 2. asks user for destination project name and folder (folder and name can be different)
     * get project name
     */
    dstName = QInputDialog::getText(this, tr("New Project Name"), tr("Enter new project name."), QLineEdit::Normal, "project", &ok) ;
    if(ok == false)
        return;

    /*
     * get project folder
     */
    QString dstPath = QFileDialog::getExistingDirectory(this,tr("Destination Project Folder"), lastPath, QFileDialog::ShowDirsOnly);
    if(dstPath.length() < 1)
        return;
    lastPath = dstPath;

    dstPath = dstPath+"/";    // make sure we have a trailing / for file copy

    QString dstProjFile= dstPath+dstName+SIDE_EXTENSION;
    int rc = QMessageBox::question(this,
                 tr("Confirm Save As Project"),
                 tr("Save As Project ?")+"\n"+dstProjFile,
                 QMessageBox::Yes, QMessageBox::No);
    if(rc == QMessageBox::No) {
        return;
    }

    /*
     * 3. creates new project folder if necessary (project can be in original folder)
     */
    QDir dpath(dstPath);

    if(dpath.exists() == false)
        dpath.mkdir(dstPath);

    if(dpath.exists() == false) {
        QMessageBox::critical(
                this,tr("Save As Project Error."),
                tr("System can not create project in ")+dstPath,
                QMessageBox::Ok);
        return;
    }

    /*
     * 4. copy project file from source to destination as new project name
     */

    // find .side file
    // remove new file before copy or copy will fail
    if(QFile::exists(dstProjFile))
        QFile::remove(dstProjFile);

    // copy project file
    QFile::copy(projFile,dstProjFile);

    QString projSrc;
    QFile sproj(projFile);
    if(sproj.open(QFile::ReadOnly | QFile::Text)) {
        projSrc = sproj.readAll();
        sproj.close();
    }

    /*
     * 5. copies the project main file from source to destination as new project main file
     */
    QStringList projList = projSrc.split("\n", QString::SkipEmptyParts);
    QString srcMainFile = sourcePath(projFile)+projList.at(0);
    QString dstMainFile = projList.at(0);
    dstMainFile = dstMainFile.trimmed();
    QString dstMainExt = dstMainFile.mid(dstMainFile.lastIndexOf("."));
    dstMainFile = dstPath+dstName+dstMainExt;

    // remove new file before copy or copy will fail
    if(QFile::exists(dstMainFile))
        QFile::remove(dstMainFile);
    // copy project main file
    QFile::copy(srcMainFile,dstMainFile);

    /*
     * Make a new project list. preprend project main file after this.
     * fixes up any links in project file and writes file
     */
    QStringList newList;
    for(int n = 1; n < projList.length(); n++) {
        QString item = projList.at(n);

        // special handling
        // TODO - fix -I and -L too
        if(item.indexOf(FILELINK) > 0) {
            QStringList list = item.split(FILELINK,QString::SkipEmptyParts);
            if(list.length() < 2) {
                QMessageBox::information(
                        this,tr("Project File Error."),
                        tr("Save As Project expected a link, but got:\n")+item+"\n\n"+
                        tr("Please manually adjust it by adding a correct link in the Project Manager list.")+" "+
                        tr("After adding the correct link, remove the bad link."));
            }
            else {
                QString als = list[1];
                als = als.trimmed();
                als = saveAsProjectLinkFix(projFolder, dstPath, als);
                if(als.length() > 0)
                    item = list[0]+FILELINK+als;
                else
                    QMessageBox::information(
                            this,tr("Can't Fix Link"),
                            tr("Save As Project was not able to fix the link:\n")+item+"\n\n"+
                            tr("Please manually adjust it by adding the correct link in the Project Manager list.")+" "+
                            tr("After adding the correct link, remove the bad link."));
                newList.append(item);
            }
        }
        else if(item.indexOf("-I") == 0) {
            QStringList list = item.split("-I",QString::SkipEmptyParts);
            if(list.length() < 1) {
                QMessageBox::information(
                        this,tr("Project File Error."),
                        tr("Save As Project expected a -I link, but got:\n")+item+"\n\n"+
                        tr("Please manually adjust it by adding a correct link in the Project Manager list.")+" "+
                        tr("After adding the correct link, remove the bad link."));
            }
            else {
                QString als = list[0];
                als = als.trimmed();
                als = saveAsProjectLinkFix(projFolder, dstPath, als);
                if(als.length() > 0)
                    item = "-I "+als;
                else
                    QMessageBox::information(
                            this,tr("Can't Fix Link"),
                            tr("Save As Project was not able to fix the link:\n")+item+"\n\n"+
                            tr("Please manually adjust it by adding the correct link in the Project Manager list.")+" "+
                            tr("After adding the correct link, remove the bad link."));
                newList.append(item);
            }
        }
        else if(item.indexOf("-L") == 0) {
            QStringList list = item.split("-L",QString::SkipEmptyParts);
            if(list.length() < 1) {
                QMessageBox::information(
                        this,tr("Project File Error."),
                        tr("Save As Project expected a -L link, but got:\n")+item+"\n\n"+
                        tr("Please manually adjust it by adding a correct link in the Project Manager list.")+" "+
                        tr("After adding the correct link, remove the bad link."));
            }
            else {
                QString als = list[0];
                als = als.trimmed();
                als = saveAsProjectLinkFix(projFolder, dstPath, als);
                if(als.length() > 0)
                    item = "-L "+als;
                else
                    QMessageBox::information(
                            this,tr("Can't Fix Link"),
                            tr("Save As Project was not able to fix the link:\n")+item+"\n\n"+
                            tr("Please manually adjust it by adding the correct link in the Project Manager list.")+" "+
                            tr("After adding the correct link, remove the bad link."));
                newList.append(item);
            }
        }
        // project options
        else if(item[0] == '>') {
            newList.append(item);
        }
        // different destination
        else if(sourcePath(projFile) != sourcePath(dstProjFile)) {
            newList.append(item);
            QString dst = sourcePath(dstProjFile)+item;
            if(QFile::exists(dst))
                QFile::remove(dst);
            QFile::copy(sourcePath(projFile)+item, dst);
        }
        // same destination, just copy item
        else {
            newList.append(item);
        }
    }

    newList.sort();

    projSrc = dstName+dstMainExt+"\n";

    for(int n = 0; n < newList.length(); n++) {
        projSrc+=newList.at(n)+"\n";
    }

    QFile dproj(dstProjFile);
    if(dproj.open(QFile::WriteOnly | QFile::Text)) {
        dproj.write(projSrc.toAscii());
        dproj.close();
    }

    this->openProject(dstProjFile);

}

/*
 * clone project copies a project from a source to destination path for education.
 */
void MainSpinWindow::cloneProject()
{
    QString src;
    QString dst;
    QVariant vs;

    vs = settings->value(cloneSrcKey,QVariant(lastPath));
    if(vs.canConvert(QVariant::String)) {
        src = vs.toString();
    }

    QString srcFile = QFileDialog::getOpenFileName(this,tr("Project to Clone"), src, PROJECT_FILE_FILTER);
    if(srcFile.length() == 0)
        return;

    lastPath = srcFile;
    saveAsProject(srcFile);
}


/*
 * close project runs through project file list and closes files.
 * finally it closes the project manager side bar.
 */
void MainSpinWindow::closeProject()
{
    /* ask to save options
     */
    int rc = QMessageBox::No;

    if(projectFile.length() == 0)
        return;

    rc = QMessageBox::question(this,
            tr("Save Project?"),
            tr("Save project manager settings before close?"),
            QMessageBox::Yes, QMessageBox::No);

    /* save options
     */
    if(rc == QMessageBox::Yes)
        saveProjectOptions();

    /* go through project file list and close files
     */
    QFile file(projectFile);
    QString proj = "";
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        proj = file.readAll();
        file.close();
    }

    proj = proj.trimmed(); // kill extra white space
    QStringList list = proj.split("\n");

    /* Run through file list and delete matches.
     */
    for(int n = 0; n < list.length(); n++) {
        for(int tab = editorTabs->count()-1; tab > -1; tab--) {
            QString s = sourcePath(projectFile)+list.at(n);
            if(s.length() == 0)
                continue;
            if(s.at(0) == '>')
                continue;
            // close exact tab
            if(editorTabs->tabToolTip(tab).compare(s) == 0)
                closeTab(tab);
            s = s.mid(0,s.lastIndexOf('.'));
            if(editorTabs->tabToolTip(tab).compare(s+SHOW_ASM_EXTENTION) == 0)
                closeTab(tab);
            if(editorTabs->tabToolTip(tab).compare(s+SHOW_ASM_EXTENTION) == 0)
                closeTab(tab);
            if(editorTabs->tabToolTip(tab).compare(s+SHOW_MAP_EXTENTION) == 0)
                closeTab(tab);
        }
    }

    /* This causes us to lose project information on next load.
     * Not sure why. Leave it out for now.
     */
    projectOptions->clearOptions();

    /* close project manager side bar
     */
    setWindowTitle(QString(ASideGuiKey));
    if(projectModel != NULL) {
        delete projectModel;
        projectModel = NULL;
    }
    projectFile.clear();
}


void MainSpinWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        closeProject();
        openProject(action->data().toString());
    }
}


void MainSpinWindow::setCurrentFile(const QString &fileName)
{
    QStringList files = settings->value(recentFilesKey).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings->setValue(recentFilesKey, files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
#ifdef SPINSIDE
        MainSpinWindow *mainWin = qobject_cast<MainSpinWindow *>(widget);
#else
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
#endif
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainSpinWindow::updateRecentFileActions()
{
    QStringList files = settings->value(recentFilesKey).toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString estr = files.at(i);
        if(estr.length() == 0)
            continue;
        //QString filename = QFileInfo(projects[i]).fileName();
        QString text = tr("&%1 %2").arg(i + 1).arg(estr);
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(estr);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorFileAct->setVisible(numRecentFiles > 0);
}

void MainSpinWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}


void MainSpinWindow::setCurrentProject(const QString &fileName)
{
    projectFile = fileName;

    QStringList files = settings->value(recentProjectsKey).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentProjects)
        files.removeLast();

    settings->setValue(recentProjectsKey, files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
#ifdef SPINSIDE
        MainSpinWindow *mainWin = qobject_cast<MainSpinWindow *>(widget);
#else
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
#endif
        if (mainWin) {
            mainWin->updateRecentProjectActions();
            break;
        }
    }
}

void MainSpinWindow::updateRecentProjectActions()
{
    QStringList projects = settings->value(recentProjectsKey).toStringList();

    int numRecentProjects = qMin(projects.size(), (int)MaxRecentProjects);

    for (int i = 0; i < numRecentProjects; ++i) {
        QString estr = projects.at(i);
        if(estr.length() == 0)
            continue;
        QString text = tr("&%1 %2").arg(i + 1).arg(estr);
        recentProjectActs[i]->setText(text);
        recentProjectActs[i]->setData(estr);
        recentProjectActs[i]->setVisible(true);
    }
    for (int j = numRecentProjects; j < MaxRecentProjects; ++j)
        recentProjectActs[j]->setVisible(false);

    separatorProjectAct->setVisible(numRecentProjects > 0);
}

void MainSpinWindow::saveFile()
{
    try {
        bool saveas = false;
        int n = this->editorTabs->currentIndex();
        QString fileName = editorTabs->tabToolTip(n);
        QString data = editors->at(n)->toPlainText();
        if(fileName.isEmpty()) {
            fileName = fileDialog.getSaveFileName(this, tr("Save As File"), lastPath, tr(SOURCE_FILE_TYPES));
            saveas = true;
        }
        if (fileName.isEmpty())
            return;
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
        editorTabs->setTabText(n,shortFileName(fileName));
        editorTabs->setTabToolTip(n,fileName);
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            QTextStream os(&file);
            os.setCodec("UTF-8"); // for now save everything as UTF-8
            if (file.open(QFile::WriteOnly)) {
                os << data;
                file.close();
            }
            if(saveas) {
                this->closeTab(n);
                this->openFileName(fileName);
            }
        }
        saveProjectOptions();
    } catch(...) {
    }
}

void MainSpinWindow::saveFileByTabIndex(int tab)
{
    try {
        QString fileName = editorTabs->tabToolTip(tab);
        QString data = editors->at(tab)->toPlainText();
        if(fileName.length() < 1) {
            qDebug() << "saveFileByTabIndex filename invalid tooltip " << tab;
            return;
        }
        editorTabs->setTabText(tab,shortFileName(fileName));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QFile::WriteOnly)) {
                file.write(data.toUtf8());
                file.close();
            }
        }
    } catch(...) {
    }
}

void MainSpinWindow::saveAsFile(const QString &path)
{
    try {
        int n = this->editorTabs->currentIndex();
        QString data = editors->at(n)->toPlainText();
        QString fileName = path;

        if (fileName.isEmpty())
            fileName = fileDialog.getSaveFileName(this, tr("Save As File"), lastPath, tr(SOURCE_FILE_TYPES));
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);

        if (fileName.isEmpty())
            return;

        this->editorTabs->setTabText(n,shortFileName(fileName));
        editorTabs->setTabToolTip(n,fileName);

        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QFile::WriteOnly)) {
                file.write(data.toUtf8());
                file.close();
            }
            // close and reopen to make sure syntax highlighting works.
            this->closeTab(n);
            this->openFileName(fileName);
            setCurrentFile(fileName);
        }
    } catch(...) {
    }
}

void MainSpinWindow::savePexFile()
{

}

void MainSpinWindow::downloadSdCard()
{
    if(projectModel == NULL || projectFile.isNull()) {
        QMessageBox mbox(QMessageBox::Critical, tr("Error No Project"),
            "Please select a tab and press F4 to set main project file.", QMessageBox::Ok);
        mbox.exec();
        return;
    }

    if (runBuild(""))
        return;

    QString fileName = fileDialog.getOpenFileName(this, tr("Send File"), sourcePath(projectFile), "Any File (*)");
    if(fileName.length() > 0)
        lastPath = sourcePath(fileName);

    if (fileName.isEmpty())
        return;

    btnConnected->setChecked(false);
    portListener->close(); // disconnect uart before use
    term->hide();

    progress->show();
    progress->setValue(0);
    status->setText("");

    getApplicationSettings();

    // don't add fileName here since it can have spaces
    QStringList args = getLoaderParameters("");
    builder->removeArg(args, "a.out");

    //QString s = QDir::toNativeSeparators(fileName);
    args.append("-f");
    args.append(this->shortFileName(fileName));

    btnConnected->setChecked(false);
    portListener->close(); // disconnect uart before use

    builder->showBuildStart(aSideLoader,args);


    process->setProperty("Name", QVariant(aSideLoader));
    process->setProperty("IsLoader", QVariant(true));

    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(sourcePath(fileName));

    procMutex.lock();
    procDone = false;
    procMutex.unlock();

    process->start(aSideLoader,args);

    status->setText(status->text()+tr(" Loading ... "));

    while(procDone == false)
        QApplication::processEvents();

}

void MainSpinWindow::procError(QProcess::ProcessError error)
{
    QVariant name = process->property("Name");
    compileStatus->appendPlainText(name.toString() + tr(" error ... (%1)").arg(error));
    compileStatus->appendPlainText(process->readAllStandardOutput());
    procMutex.lock();
    procDone = true;
    procMutex.unlock();
}

void MainSpinWindow::procFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(procDone == true)
        return;

    procMutex.lock();
    procDone = true;
    procMutex.unlock();

    QVariant name = process->property("Name");
    builder->buildResult(exitStatus, exitCode, name.toString(), process->readAllStandardOutput());

    int len = status->text().length();
    QString s = status->text().mid(len-8);
    if(s.contains("done.",Qt::CaseInsensitive) == false)
        status->setText(status->text()+" done.");
}

/*
 * save for cat dumps
 */
void MainSpinWindow::procReadyReadCat()
{

}

/*
 * read program sizes from objdump -h
 */

void MainSpinWindow::procReadyRead()
{
    QByteArray bytes = process->readAllStandardOutput();
    if(bytes.length() == 0)
        return;

#if defined(Q_WS_WIN32)
    QString eol("\r");
#else
    QString eol("\n");
#endif

    // bstc doesn't return good exit status
    QString progname;
    QVariant pvar = process->property("Name");
    if(pvar.canConvert(QVariant::String)) {
        progname = pvar.toString();
    }
    if(progname.contains("bstc",Qt::CaseInsensitive)) {
        if(QString(bytes).contains("Error",Qt::CaseInsensitive)) {
            procResultError = true;
        }
    }

    QStringList lines = QString(bytes).split("\n",QString::SkipEmptyParts);
    if(bytes.contains("bytes")) {
        for (int n = 0; n < lines.length(); n++) {
            QString line = lines[n];
            if(line.length() > 0) {
                if(line.indexOf("\r") > -1) {
                    QStringList more = line.split("\r",QString::SkipEmptyParts);
                    lines.removeAt(n);
                    if(line.contains("Propeller Version",Qt::CaseInsensitive)) {
                        compileStatus->insertPlainText(eol+line);
                        progress->setValue(0);
                    }
                    for(int m = more.length()-1; m > -1; m--) {
                        QString ms = more.at(m);
                        if(ms.contains("bytes",Qt::CaseInsensitive))
                            lines.insert(n,more.at(m));
                        if(ms.contains("loading",Qt::CaseInsensitive))
                            lines.insert(n,more.at(m));
                    }
                }
            }
        }
    }

    for (int n = 0; n < lines.length(); n++) {
        QString line = lines[n];
        if(line.length() > 0) {
            compileStatus->moveCursor(QTextCursor::End);
            if(line.contains("Propeller Version",Qt::CaseInsensitive)) {
                compileStatus->insertPlainText(line+eol);
                progress->setValue(0);
            }
            else
            if(line.contains("loading",Qt::CaseInsensitive)) {
                progMax = 0;
                progress->setValue(0);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("writing",Qt::CaseInsensitive)) {
                progMax = 0;
                progress->setValue(0);
            }
            else
            if(line.contains("Download OK",Qt::CaseInsensitive)) {
                progress->setValue(100);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("sent",Qt::CaseInsensitive)) {
                compileStatus->moveCursor(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("remaining",Qt::CaseInsensitive)) {
                if(progMax == 0) {
                    QString bs = line.mid(0,line.indexOf(" "));
                    progMax = bs.toInt();
                    progMax /= 1024;
                    progMax++;
                    progCount = 0;
                    if(progMax == 0) {
                        progress->setValue(100);
                    }
                }
                if(progMax != 0) {
                    progCount++;
                    progress->setValue(100*progCount/progMax);
                }
                compileStatus->moveCursor(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
                compileStatus->insertPlainText(line);
            }
            else {
                compileStatus->insertPlainText(eol);
                compileStatus->insertPlainText(line);
            }
        }
    }

}

/*
 * make star go away if no changes.
 */
void MainSpinWindow::fileChanged()
{
    if(fileChangeDisable)
        return;

    int index = editorTabs->currentIndex();
    QString name = editorTabs->tabText(index);
    Editor *ed = editors->at(index);

    /* tab controls have been moved to the editor class */

    QString curtext = ed->toPlainText();
    if(curtext.length() == 0)
        return;
    QString fileName = editorTabs->tabToolTip(index);
    if(fileName.length() == 0)
        return;
    QFile file(fileName);
    if(file.exists() == false) {
        if(curtext.length() > 0) {
            if(name.at(name.length()-1) != '*') {
                name += tr(" *");
                editorTabs->setTabText(index, name);
            }
        }
        return;
    }
    QString text;
    int ret = 0;

    QChar ch = name.at(name.length()-1);
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        if(this->isFileUTF16(&file))
            in.setCodec("UTF-16");
        else
            in.setCodec("UTF-8");

        text = in.readAll();
        file.close();

        QString ctext = curtext;
        QPlainTextEdit myed(this);
        myed.setPlainText(text);
        text = myed.toPlainText();

        ret = ctext.compare(text);
        if(ret == 0) {
            if( ch == QChar('*'))
                editorTabs->setTabText(index, this->shortFileName(fileName));
            return;
        }
    }
    if( ch != QChar('*')) {
        name += tr(" *");
        editorTabs->setTabText(index, name);
    }
}

void MainSpinWindow::printFile()
{
    QPrinter printer;

    int tab = editorTabs->currentIndex();
    Editor *ed = editors->at(tab);
    QString name = editorTabs->tabText(tab);
    name = name.mid(0,name.lastIndexOf("."));

#if defined(Q_WS_WIN32) || defined(Q_WS_MAC)
    printer.setDocName(name);
#else
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(this->lastPath+name+".pdf");
#endif

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (ed->textCursor().hasSelection())
        dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    int rc = dialog->exec();

    if(rc == QDialog::Accepted) {
        ed->print(&printer);
    }
    delete dialog;
}

void MainSpinWindow::copyFromFile()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->copy();
    editor->clearCtrlPressed();
}
void MainSpinWindow::cutFromFile()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->cut();
    editor->clearCtrlPressed();
}
void MainSpinWindow::pasteToFile()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->paste();
    editor->clearCtrlPressed();
}

void MainSpinWindow::editCommand()
{

}
void MainSpinWindow::systemCommand()
{

}

void MainSpinWindow::fontDialog()
{
    bool ok = false;

    QFont font;
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor) {
        QFont edfont = editor->font();
        font = QFontDialog::getFont(&ok, edfont);
    }
    else {
        font = QFontDialog::getFont(&ok, this->font());
    }

    if(ok) {
        for(int n = editors->count()-1; n >= 0; n--) {
            Editor *ed = editors->at(n);
            ed->setFont(font);
        }
        editorFont = font;
    }
}

void MainSpinWindow::fontBigger()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor) {
        QFont font = editor->font();
        int size = font.pointSize()*10/8;
        if(size < 90)
            font.setPointSize(size);
        for(int n = editors->count()-1; n >= 0; n--) {
            Editor *ed = editors->at(n);
            ed->setFont(font);
        }
        editorFont = font;
    }
}

void MainSpinWindow::fontSmaller()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor) {
        QFont font = editor->font();
        int size = font.pointSize()*8/10;
        if(size > 3)
            font.setPointSize(size);
        for(int n = editors->count()-1; n >= 0; n--) {
            Editor *ed = editors->at(n);
            ed->setFont(font);
        }
        editorFont = font;
    }
}

void MainSpinWindow::replaceInFile()
{
    if(!replaceDialog)
        return;

    Editor *editor = editors->at(editorTabs->currentIndex());
    editor->clearCtrlPressed();

    replaceDialog->clearFindText();
    QString text = editors->at(editorTabs->currentIndex())->textCursor().selectedText();
    if(text.isEmpty() == false)
        replaceDialog->setFindText(text);
    replaceDialog->clearReplaceText();

    replaceDialog->setEditor(editor);
    replaceDialog->exec();
}

/*
 * FindHelp
 *
 * Takes a word from the Editor's user cursor or mouse
 * and looks for it in the documentation.
 */
void MainSpinWindow::findSymbolHelp(QString text)
{
    helpDialog->show(aSideDocPath, text);
}

void MainSpinWindow::findDeclaration(QPoint point)
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->cursorForPosition(point);
    findDeclaration(cur);
}

void MainSpinWindow::findDeclaration()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->textCursor();
    findDeclaration(cur);
}

void MainSpinWindow::findDeclaration(QTextCursor cur)
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor) {
        /* find word */
        QString text = cur.selectedText();
        qDebug() << "findDeclaration " << text;

        if(text.length() == 0) {
            cur.select(QTextCursor::WordUnderCursor);
            text = cur.selectedText();
        }
        qDebug() << "findDeclaration " << text;
        editor->setTextCursor(cur);

        if(text.length() > 0 && text.at(0).isLetter()) {
            if(this->isCProject())
                ctags->runCtags(projectFile);
#ifdef SPIN
            else if(this->isSpinProject())
                ctags->runSpinCtags(projectFile, propDialog->getSpinLibraryStr());
#endif
        }
        else {
            return;
        }

        /* find line for traceback */
        cur.select(QTextCursor::LineUnderCursor);

        int index = editorTabs->currentIndex();
        QString fileName = editorTabs->tabToolTip(index);
        int currentLine  = cur.blockNumber();

        /* we append :currentLine to filename for good lookup on return from stack
         */
        QString currentTag = "/\t" +
            fileName + ":" + QString("%1").arg(currentLine) +
            "\t" + cur.selectedText();

        if(text.length() == 0) {
            findDeclarationInfo();
            return;
        }
        QString tagLine = ctags->findTag(text);

        /* if we have a good declaration, push stack and enable back button
         */
        if(showDeclaration(tagLine) > -1) {
            ctags->tagPush(currentTag);
            btnBrowseBack->setEnabled(true);
        }
        else
            findDeclarationInfo();
    }
}

bool MainSpinWindow::isTagged(QString text)
{
    bool rc = false;

    if(text.length() == 0) {
        findDeclarationInfo();
        return rc;
    }
    if(text.length() > 0 && text.at(0).isLetter()) {
        if(this->isCProject())
            ctags->runCtags(projectFile);
#ifdef SPIN
        else if(this->isSpinProject())
            ctags->runSpinCtags(projectFile, propDialog->getSpinLibraryStr());
#endif
    }
    else {
        return rc;
    }

    QString tagline = ctags->findTag(text);

    if(tagline.length() == 0)
        return rc;
    QString file = ctags->getFile(tagline);
    if(file.length() == 0)
        return rc;
    int  linenum = ctags->getLine(tagline);
    if(linenum < 0)
        return rc;

    return true;
}

void MainSpinWindow::findDeclarationInfo()
{
#if defined(Q_WS_MAC)
     QMessageBox::information(this,
         tr("Browse Declaration"),
         tr("Use \"Command+]\" to find a declaration.\n" \
            "Also \"Command+Left Click\" finds a declaration.\n" \
            "Use \"Command+[\" to go back.\n\n" \
            "Library declarations will not be found.\n"),
         QMessageBox::Ok);
#else
    QMessageBox::information(this,
        tr("Browse Declaration"),
        tr("Use \"Alt+Right Arrow\" to find a declaration.\n" \
           "Also \"Ctrl+Left Click\" finds a declaration.\n" \
           "Use \"Alt+Left Arrow\" to go back.\n\n" \
           "Library declarations will not be found.\n"),
        QMessageBox::Ok);
#endif
}

void MainSpinWindow::prevDeclaration()
{
    if(ctags->tagCount() == 0) {
        btnBrowseBack->setEnabled(false);
        return;
    }
    QString tagline = ctags->tagPop();
    if(tagline.length() > 0) {
        showDeclaration(tagline);
    }
    if(ctags->tagCount() == 0) {
        btnBrowseBack->setEnabled(false);
    }
    Editor *editor = editors->at(editorTabs->currentIndex());
    QStringList list = tagline.split("\t");
    QString word;
    if(list.length() > 1) {
        word = list.at(2);
        word = word.trimmed();
        editor->find(word);
        if(word.contains("("))
            word = word.mid(0,word.indexOf("("));
        word = word.trimmed();
        if(word.contains(" "))
            word = word.mid(0,word.indexOf(" "));
        word = word.trimmed();
    }
    if(word.length()>0) {
        QTextCursor cur = editor->textCursor();
        word = cur.selectedText();
        cur.setPosition(cur.anchor());
        editor->setTextCursor(cur);
    }
}

int MainSpinWindow::showDeclaration(QString tagline)
{
    int rc = -1;
    int  linenum = 0;

    if(tagline.length() == 0)
        return rc;
    QString file = ctags->getFile(tagline);
    if(file.length() == 0)
        return rc;

    /* if the file has a line number in it, use it.
     */
    QString line = "";
    if(file.lastIndexOf(':') > 3)
        line = file.mid(file.lastIndexOf(':')+1);

    if(line.length() > 0) {
        /* got a line number. convert and trunc file */
        linenum = line.toInt();
        file = file.mid(0,file.lastIndexOf(':'));
    }
    else {
        /* get regex line from tags */
        linenum = ctags->getLine(tagline);
        if(linenum < 0)
            return rc;
    }

    this->openFileName(file);
    Editor *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->textCursor();
    cur.setPosition(0,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,linenum);
    cur.movePosition(QTextCursor::EndOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
    QString res = cur.selectedText();
    qDebug() << linenum << res;
    editor->setTextCursor(cur);
    QApplication::processEvents();
    return linenum;
}

void MainSpinWindow::redoChange()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->redo();
}

void MainSpinWindow::undoChange()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->undo();
}

void MainSpinWindow::setProject()
{
    int index = editorTabs->currentIndex();
    QString fileName = editorTabs->tabToolTip(index);

    if(fileName.length() == 0) {
        int rc = QMessageBox::question(this,
            tr("Save As"),
            tr("Would you like to save this file and set a project using it?"),
            QMessageBox::Yes, QMessageBox::No);
        if(rc == QMessageBox::Yes) {
            saveAsFile();
            updateProjectTree(fileName);
            setCurrentProject(projectFile);
        }
    }
    fileName = editorTabs->tabToolTip(index);
    if(fileName.length() > 0)
    {
        if(fileName.indexOf(SPIN_EXTENSION)) {
#ifdef SPIN
            projectOptions->setCompiler(SPIN_TEXT);
#else
            if(fileName.mid(fileName.lastIndexOf(".")+1).contains("spin",Qt::CaseInsensitive)) {
                QMessageBox::critical(
                        this,tr("SPIN Not Supported"),
                        tr("Spin projects are not supported with this version."),
                        QMessageBox::Ok);
                return;
            }
#endif
        }
        else {
            projectOptions->setCompiler("C");
        }
        updateProjectTree(fileName);
        setCurrentProject(projectFile);
    }
#ifdef SPIN
    QString extension = fileName.mid(fileName.lastIndexOf(".")+1);
    if(extension.compare(SPIN_TEXT,Qt::CaseInsensitive) == 0) {
        projectOptions->setCompiler(SPIN_TEXT);
        //projectOptions->setBoardType("HUB"); // HUB is only option for SPIN
        this->closeFile(); // do this so spin highlighter works.
        this->openFileName(fileName);
        btnDownloadSdCard->setEnabled(false);
    }
    else
#endif
    {
        btnDownloadSdCard->setEnabled(true);
    }
}

void MainSpinWindow::hardware()
{
    //hardwareDialog->loadBoards();
    //hardwareDialog->show();
    initBoardTypes();
}

void MainSpinWindow::properties()
{
    propDialog->showProperties();
}

void MainSpinWindow::propertiesAccepted()
{
    getApplicationSettings();
    initBoardTypes();
    //Editor *ed = editors->at(editorTabs->currentIndex());
    for(int n = 0; n < editorTabs->count(); n++) {
        Editor *e = editors->at(n);
        e->setTabStopWidth(propDialog->getTabSpaces()*10);
        e->setHighlights(editorTabs->tabText(n));
    }
}

void MainSpinWindow::programStopBuild()
{
    if(builder != NULL)
        builder->abortProcess();

    if(this->procDone != true) {
        this->procMutex.lock();
        this->procDone = true;
        this->procMutex.unlock();
        qDebug() << "procDone set";
        QApplication::processEvents();
        process->kill();
    }
}

void MainSpinWindow::programBuild()
{
    runBuild("");
}

void MainSpinWindow::programBurnEE()
{
    if(runBuild(""))
        return;
#ifdef Q_WS_WIN32
    portListener->close();
    btnConnected->setChecked(false);
    term->setPortEnabled(false);
#endif
    runLoader("-e -r");
}

void MainSpinWindow::programRun()
{
    // don't allow run if button is disabled
    if(btnProgramRun->isEnabled() == false)
        return;

    if(runBuild(""))
        return;
#ifdef Q_WS_WIN32
    portListener->close();
    btnConnected->setChecked(false);
    term->setPortEnabled(false);
#endif
    runLoader("-r");
}

void MainSpinWindow::programDebug()
{
    // don't allow run if button is disabled
    if(btnProgramDebugTerm->isEnabled() == false)
        return;

    if(runBuild(""))
        return;

#if !defined(Q_WS_WIN32)
    portListener->open();
    term->getEditor()->setPortEnable(false);
    if(runLoader("-r -t")) {
        portListener->close();
        return;
    }
#else
    portListener->close();
    btnConnected->setChecked(false);
    if(runLoader("-r -t"))
        return;
    portListener->open();
#endif
    btnConnected->setChecked(true);
    term->getEditor()->setPlainText("");
    term->getEditor()->setPortEnable(true);
    term->activateWindow();
    term->show();
    term->getEditor()->setFocus();
}

void MainSpinWindow::debugCompileLoad()
{
    QString gdbprog("propeller-elf-gdb");
#if defined(Q_WS_WIN32)
    gdbprog += ".exe";
#else
    gdbprog = aSideCompilerPath + gdbprog;
#endif

    /* compile for debug */
    if(runBuild("-g"))
        return;

    /* start debugger */
    QString port = cbPort->currentText();

    /* set gdb tab */
    for(int n = statusTabs->count(); n >= 0; n--) {
        if(statusTabs->tabText(n).compare(GDB_TABNAME) == 0) {
            statusTabs->setCurrentIndex(n);
            break;
        }
    }
    gdb->load(gdbprog, sourcePath(projectFile), aSideCompilerPath+"gdbstub", "a.out", port);
}

void MainSpinWindow::gdbShowLine()
{
    QString fileName = gdb->getResponseFile();
    int number = gdb->getResponseLine();
    qDebug() << "gdbShowLine" << fileName << number;

    openFileName(sourcePath(projectFile)+fileName);
    Editor *ed = editors->at(editorTabs->currentIndex());
    if(ed) ed->setLineNumber(number);
}

void MainSpinWindow::gdbKill()
{
    gdb->kill();
}

void MainSpinWindow::gdbBacktrace()
{
    gdb->backtrace();
}

void MainSpinWindow::gdbContinue()
{
    gdb->runProgram();
}

void MainSpinWindow::gdbNext()
{
    gdb->next();
}

void MainSpinWindow::gdbStep()
{
    gdb->step();
}

void MainSpinWindow::gdbFinish()
{
    gdb->finish();
}

void MainSpinWindow::gdbUntil()
{
    gdb->until();
}

void MainSpinWindow::gdbBreak()
{
    gdbShowLine();
}

void MainSpinWindow::gdbInterrupt()
{
    gdb->interrupt();
}

void MainSpinWindow::terminalClosed()
{
    portListener->close();
    btnConnected->setChecked(false);
}

void MainSpinWindow::setupHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"), this);

    aboutLanding = "<html><body><br/>"+tr("Visit ") +
        "<a href=\"http://www.parallax.com/propellergcc/\">"+
        ASideGuiKey+"</a>"+
        tr(" for more information.")+"<br/>"+
        tr("Email bug reports to")+" <a href=\"mailto:gccbeta@parallax.com\">gccbeta@parallax.com</a>"+
        "<br/></body></html>";

    menuBar()->addMenu(helpMenu);
    aboutDialog = new AboutDialog(aboutLanding, this);

    helpMenu->addAction(QIcon(":/images/about.png"), tr("&About"), this, SLOT(aboutShow()));
    helpMenu->addAction(QIcon(":/images/helphint.png"), tr("&Credits"), this, SLOT(creditShow()));
    helpMenu->addAction(QIcon(":/images/UserHelp.png"), tr("&Help"), this, SLOT(userguideShow()));
    //helpMenu->addAction(QIcon(":/images/Library.png"), tr("&Library"), this, SLOT(libraryShow()));

    /* new Help class */
    helpDialog = new Help();
}

void MainSpinWindow::aboutShow()
{
    aboutDialog->show();
}

void MainSpinWindow::creditShow()
{
    QString license(ASideGuiKey+tr(" is an MIT Licensed Open Source IDE. It was developed with Open Source QT and uses QT shared libraries under LGPLv2.1.<br/><br/>"));
    QString propgcc(ASideGuiKey+tr(" uses <a href=\"http://propgcc.googlecode.com\">Propeller GCC tool chain</a> based on GCC 4.6.1 under GPLv3. ")+"<p>");
    QString ctags(tr("It uses the <a href=\"http://ctags.sourceforge.net\">ctags</a> binary program built from sources under GPLv2 for source browsing. ")+"<p>");
    QString icons(tr("Most icons used are from <a href=\"http://www.small-icons.com/packs/24x24-free-application-icons.htm\">www.aha-soft.com 24x24 Free Application Icons</a> " \
                     "and used according to Creative Commons Attribution 3.0 License.<br/><br/>"));
    QString sources(tr("All IDE sources are available at <a href=\"http://propside.googlecode.com\">repository</a>. " \
                       "Licenses are in this package."));

    QString translations("<p><b>"+tr("Translations:")+"</b><p>");
    QString simplechinese(tr("Simplified Chinese by ")+tr("Kenichi Kato, designer of Matanya") + " <a href=\"http://estory.com.sg/\">eStory.com.sg</a>");

    QMessageBox::information(this, ASideGuiKey+tr(" Credits"),"<html><body>"+
        license+propgcc+ctags+icons+sources+translations+simplechinese+"</body></html>",
        QMessageBox::Ok);

}

void MainSpinWindow::helpShow()
{
    QString address = "file:///"+aSideDocPath+"/index.html";
    QString helplib("<a href=\""+address+"\">"+tr("Library help")+"</a> "+
                 tr("is context sensitive.")+" "+tr("Press F1 with mouse over words.")+"<br/>");
    QMessageBox::information(this, ASideGuiKey+tr(" Help"),
        tr("<p><b>")+ASideGuiKey+tr("</b> is an integrated C development environment "\
           "which can build and load Propeller GCC " \
           "programs to Propeller for many board types.") + "<p>" +
        helplib+aboutLanding, QMessageBox::Ok);
}

void MainSpinWindow::libraryShow()
{
    findSymbolHelp("");
}


void MainSpinWindow::userguideShow()
{
    QString address = "http://www.parallax.com/propellergcc";
    QDesktopServices::openUrl(QUrl(address));
}

void MainSpinWindow::setCurrentBoard(int index)
{
    boardName = cbBoard->itemText(index);
    cbBoard->setCurrentIndex(index);
}

void MainSpinWindow::setCurrentPort(int index)
{
    if(index < 0)
        return;
    portName = cbPort->itemText(index);
    cbPort->setCurrentIndex(index);
    if(friendlyPortName.length() > 0)
        cbPort->setToolTip(friendlyPortName.at(index));
    if(portName.length()) {
        portListener->init(portName, BAUD115200);  // signals get hooked up internally
    }
}

void MainSpinWindow::checkAndSaveFiles()
{
    if(projectModel == NULL)
        return;

    /* check for project file changes
     */
    QString title = projectModel->getTreeName();
    QString modTitle = title + " *";
    for(int tab = editorTabs->count()-1; tab > -1; tab--)
    {
        QString tabName = editorTabs->tabText(tab);
        if(tabName == modTitle)
        {
            saveFileByTabIndex(tab);
            editorTabs->setTabText(tab,title);
        }
    }

    /* check for project files name changes
     */
    int len = projectModel->rowCount();
    for(int n = 0; n < len; n++)
    {
        QModelIndex root = projectModel->index(n,0);
        QVariant vs = projectModel->data(root, Qt::DisplayRole);
        if(!vs.canConvert(QVariant::String))
            continue;
        QString name = vs.toString();
        if(name.indexOf(FILELINK) > 0)
            name = name.mid(0,name.indexOf(FILELINK));
        QString modName = name + " *";
        for(int tab = editorTabs->count()-1; tab > -1; tab--)
        {
            QString tabName = editorTabs->tabText(tab);
            if(tabName == modName)
            {
                saveFileByTabIndex(tab);
                editorTabs->setTabText(tab,name);
            }
        }
    }

    /* check for tabs out of project scope that are
     * not saved and warn user.
     */
    for(int tab = editorTabs->count()-1; tab > -1; tab--)
    {
        QString tabName = editorTabs->tabText(tab);
        if(tabName.at(tabName.length()-1) == '*')
        {
#ifdef SPIN
            if(!isSpinProject()) {
                QMessageBox::information(this,
                    tr("Not a Project File"),
                    tr("The file \"")+tabName+tr("\" is not part of the current project.\n") +
                    tr("Please save and add the file to the project to build it."),
                    QMessageBox::Ok);
            }
            else
#endif
            {
                QMessageBox::information(this,
                    tr("Not a Project File"),
                    tr("The file \"")+tabName+tr("\" is not part of the current project.\n"),
                    QMessageBox::Ok);
            }
        }
    }

    saveProjectOptions();

}

QString MainSpinWindow::sourcePath(QString srcpath)
{
    srcpath = QDir::fromNativeSeparators(srcpath);
    srcpath = srcpath.mid(0,srcpath.lastIndexOf(aSideSeparator)+1);
    return srcpath;
}

bool MainSpinWindow::isSpinProject()
{
#ifdef SPIN
    QString compiler = projectOptions->getCompiler();
    if(compiler.compare(SPIN_TEXT, Qt::CaseInsensitive) == 0) {
        btnDownloadSdCard->setEnabled(false);
        return true;
    }
#endif
    btnDownloadSdCard->setEnabled(true);
    return false;
}

bool MainSpinWindow::isCProject()
{
    QString compiler = projectOptions->getCompiler();
    if(compiler.compare("C", Qt::CaseInsensitive) == 0) {
        btnDownloadSdCard->setEnabled(true);
        return true;
    }
    else if(compiler.compare("C++", Qt::CaseInsensitive) == 0) {
        btnDownloadSdCard->setEnabled(true);
        return true;
    }
    btnDownloadSdCard->setEnabled(false);
    return false;
}

void MainSpinWindow::selectBuilder()
{
#ifdef SPIN
    if(isSpinProject())
        builder = buildSpin;
    else if(isCProject())
#endif
        builder = buildC;
}

int  MainSpinWindow::runBuild(QString option)
{
    checkAndSaveFiles();
    selectBuilder();
    return builder->runBuild(option, projectFile, aSideCompiler);
}

QStringList MainSpinWindow::getLoaderParameters(QString copts)
{
    // use the projectFile instead of the current tab file
    // QString srcpath = sourcePath(projectFile);

    int compileType = ProjectOptions::TAB_OPT; // 0
#ifdef SPIN
    if(isSpinProject()) {
        builder = buildSpin;
        compileType = ProjectOptions::TAB_SPIN_COMP;
    }
    else if(isCProject())
#endif
    {
        builder = buildC;
        compileType = ProjectOptions::TAB_C_COMP;
    }

    portName = cbPort->itemText(cbPort->currentIndex());
    boardName = cbBoard->itemText(cbBoard->currentIndex());
    QString sdrun = ASideConfig::UserDelimiter+ASideConfig::SdRun;
    QString sdload = ASideConfig::UserDelimiter+ASideConfig::SdLoad;

    sdrun = sdrun.toUpper();
    sdload = sdload.toUpper();
    if(boardName.contains(sdrun))
        boardName = boardName.mid(0,boardName.indexOf(sdrun));
    if(boardName.contains(sdload,Qt::CaseInsensitive))
        boardName = boardName.mid(0,boardName.indexOf(sdload));

    QStringList args;

    if(this->propDialog->getLoadDelay() > 0) {
        args.append(QString("-S%1").arg(this->propDialog->getLoadDelay()));
    }
    QString bname = this->cbBoard->currentText();
    ASideBoard* board = aSideConfig->getBoardData(bname);
    QString reset("DTR");
    if(board != NULL)
        reset = board->get(ASideBoard::reset);

    if(this->propDialog->getResetType() == Properties::CFG) {
        if(reset.contains("RTS",Qt::CaseInsensitive))
            args.append("-Dreset=rts");
        else
        if(reset.contains("DTR",Qt::CaseInsensitive))
            args.append("-Dreset=dtr");
    }
    else
    if(this->propDialog->getResetType() == Properties::RTS) {
        args.append("-Dreset=rts");
    }
    else
    if(this->propDialog->getResetType() == Properties::DTR) {
        args.append("-Dreset=dtr");
    }
    // always include user's propeller-load path.
    if(compileType == ProjectOptions::TAB_C_COMP) {
        args.append("-I");
        args.append(aSideIncludes);
        args.append("-b");
        args.append(boardName);
    }
    args.append("-p");
    args.append(portName);

    builder->appendLoaderParameters(copts, projectFile, &args);

    //qDebug() << args;
    return args;
}

int  MainSpinWindow::runLoader(QString copts)
{
    if(projectModel == NULL || projectFile.isNull()) {
        QMessageBox mbox(QMessageBox::Critical, "Error No Project",
            "Please select a tab and press F4 to set main project file.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    progress->show();
    progress->setValue(0);

    getApplicationSettings();

    if(copts.indexOf(" -t") > 0) {
        copts = copts.mid(0,copts.indexOf(" -t"));
    }

    QString loadtype = cbBoard->currentText();
    if(loadtype.isEmpty() || loadtype.length() == 0) {
        QMessageBox::critical(this,tr("Can't Load"),tr("Can't load an empty board type."),QMessageBox::Ok);
        return -1;
    }

    if(loadtype.contains(ASideConfig::UserDelimiter+ASideConfig::SdRun, Qt::CaseInsensitive)) {
        copts.append(" -z a.out");
        qDebug() << loadtype << copts;
    }
    else
        if(loadtype.contains(ASideConfig::UserDelimiter+ASideConfig::SdLoad, Qt::CaseInsensitive)) {
        copts.append(" -l a.out");
        qDebug() << loadtype << copts;
    }

    QStringList args = getLoaderParameters(copts);

    builder->showBuildStart(aSideLoader,args);

    process->setProperty("Name", QVariant(aSideLoader));
    process->setProperty("IsLoader", QVariant(true));

    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(sourcePath(projectFile));

    procMutex.lock();
    procDone = false;
    procMutex.unlock();

    process->start(aSideLoader,args);

    status->setText(status->text()+tr(" Loading ... "));

    while(procDone == false)
        QApplication::processEvents();

    QTextCursor cur = compileStatus->textCursor();
    cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    compileStatus->setTextCursor(cur);

    progress->hide();
    return process->exitCode();
}

void MainSpinWindow::compilerError(QProcess::ProcessError error)
{
    qDebug() << error;
}

void MainSpinWindow::compilerFinished(int exitCode, QProcess::ExitStatus status)
{
    qDebug() << exitCode << status;
}

void MainSpinWindow::closeTab(int tab)
{
    if(editors->count() < 1)
        return;

    QMessageBox mbox(QMessageBox::Question, "Save File?", "",
                     QMessageBox::Discard | QMessageBox::Save, this);

    fileChangeDisable = true;

    QString tabName = editorTabs->tabText(tab);
    if(tabName.at(tabName.length()-1) == '*')
    {
        if(editorTabs->tabText(tab).contains(untitledstr)) {
            saveAsFile(editors->at(tab)->toolTip());
        }
        else {
            mbox.setInformativeText(tr("Save File? ") + tabName.mid(0,tabName.indexOf(" *")));
            int ret = mbox.exec();
            if(ret == QMessageBox::Save)
                saveFileByTabIndex(tab);
        }
    }

    editors->at(tab)->setPlainText("");
    editors->remove(tab);
    if(editorTabs->count() == 1)
        newFile();
    editorTabs->removeTab(tab);

    fileChangeDisable = false;
}

/* no menu yet, just right click to reload
*/
void MainSpinWindow::editorTabMenu(QPoint pt)
{
    if(pt.isNull())
        return;
    int tab = editorTabs->currentIndex();
    if(tab < 0)
        return;
    QString file = editorTabs->tabToolTip(tab);
    QString name = editorTabs->tabText(tab);
    if(file.length() > 0 && name.indexOf("*") < 0)
        openFile(file);
}

void MainSpinWindow::changeTab(bool checked)
{
    /* checked is a QAction menu state.
     * we don't really care about it
     */
    if(checked) return; // compiler happy :)

    int n = editorTabs->currentIndex();
    if(n < editorTabs->count()-1)
        editorTabs->setCurrentIndex(n+1);
    else
        editorTabs->setCurrentIndex(0);
}

void MainSpinWindow::addToolButton(QToolBar *bar, QToolButton *btn, QString imgfile)
{
    const QSize buttonSize(24, 24);
    btn->setIcon(QIcon(QPixmap(imgfile.toAscii())));
    btn->setMinimumSize(buttonSize);
    btn->setMaximumSize(buttonSize);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bar->addWidget(btn);
}

void MainSpinWindow::setupProjectTools(QSplitter *vsplit)
{
    int adjust = 100;

    /* container for project, etc... */
    leftSplit = new QSplitter(this);
    leftSplit->setMinimumHeight(APPWINDOW_MIN_HEIGHT-adjust);
    leftSplit->setOrientation(Qt::Vertical);
    vsplit->addWidget(leftSplit);

    /* project tree */
    QTabWidget *projectTab = new QTabWidget(this);
    projectTree = new ProjectTree(this);
    projectTree->setMinimumWidth(PROJECT_WIDTH-1);
    projectTree->setMaximumWidth(PROJECT_WIDTH-1);
    projectTree->setToolTip(tr("Current Project"));
    connect(projectTree,SIGNAL(clicked(QModelIndex)),this,SLOT(projectTreeClicked(QModelIndex)));
    projectTab->addTab(projectTree,tr("Project Manager"));
    leftSplit->addWidget(projectTab);

    // projectMenu is popup for projectTree
    projectMenu = new QMenu(QString("Project Menu"));
    projectMenu->addAction(tr("Add File Copy"), this,SLOT(addProjectFile()));
    projectMenu->addAction(tr("Add File Link"), this,SLOT(addProjectLink()));
    projectMenu->addAction(tr("Add Include Path"), this,SLOT(addProjectIncPath()));
    projectMenu->addAction(tr("Add Library File"), this,SLOT(addProjectLibFile()));
    projectMenu->addAction(tr("Add Library Path"), this,SLOT(addProjectLibPath()));
    projectMenu->addAction(tr("Delete"), this,SLOT(deleteProjectFile()));
    projectMenu->addAction(tr("Show Assembly"), this,SLOT(showAssemblyFile()));
    projectMenu->addAction(tr("Show Map File"), this,SLOT(showMapFile()));
    projectMenu->addAction(tr("Show File"), this,SLOT(showProjectFile()));

    projectOptions = new ProjectOptions(this);
    projectOptions->setMinimumWidth(PROJECT_WIDTH);
    projectOptions->setMaximumWidth(PROJECT_WIDTH);
    projectOptions->setToolTip(tr("Project Options"));
    connect(projectOptions,SIGNAL(compilerChanged()),this,SLOT(compilerChanged()));

    cbBoard = projectOptions->getHardwareComboBox();
    cbBoard->setToolTip(tr("Board Type Select"));
    connect(cbBoard,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentBoard(int)));
    QToolButton *hardwareButton = projectOptions->getHardwareButton();
    connect(hardwareButton,SIGNAL(clicked()),this,SLOT(initBoardTypes()));

    leftSplit->addWidget(projectOptions);

    QList<int> lsizes = leftSplit->sizes();
    lsizes[0] = leftSplit->height()*60/100;
    lsizes[1] = leftSplit->height()*40/100;
    leftSplit->setSizes(lsizes);

    leftSplit->adjustSize();

    rightSplit = new QSplitter(this);
    rightSplit->setMinimumHeight(APPWINDOW_MIN_HEIGHT-adjust);
    rightSplit->setOrientation(Qt::Vertical);
    vsplit->addWidget(rightSplit);

    /* project editors */
    editors = new QVector<Editor*>();

    /* project editor tabs */
    editorTabs = new QTabWidget(this);
    editorTabs->setTabsClosable(true);
    editorTabs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(editorTabs,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));
    connect(editorTabs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(editorTabMenu(QPoint)));
    rightSplit->addWidget(editorTabs);

    statusTabs = new QTabWidget(this);

    compileStatus = new QPlainTextEdit(this);
    compileStatus->setLineWrapMode(QPlainTextEdit::NoWrap);
    compileStatus->setReadOnly(true);
    connect(compileStatus,SIGNAL(selectionChanged()),this,SLOT(compileStatusClicked()));
    statusTabs->addTab(compileStatus,tr("Build Status"));

#if defined(IDEDEBUG)
    debugStatus = new QPlainTextEdit(this);
    debugStatus->setLineWrapMode(QPlainTextEdit::NoWrap);
    //debugStatus->setReadOnly(true);
    statusTabs->addTab(debugStatus,tr("IDE Debug"));
#endif

#if defined(GDBENABLE)
    gdbStatus = new QPlainTextEdit(this);
    gdbStatus->setLineWrapMode(QPlainTextEdit::NoWrap);
    /* setup the gdb class */
    gdb = new GDB(gdbStatus, this);

    statusTabs->addTab(gdbStatus,tr(GDB_TABNAME));
#endif

    rightSplit->addWidget(statusTabs);

    QList<int> rsizes = rightSplit->sizes();
    rsizes[0] = rightSplit->height()*63/100;
    rsizes[1] = rightSplit->height()*37/100;
    rightSplit->setSizes(rsizes);

    rightSplit->adjustSize();

    /* status bar for progressbar */
    QStatusBar *statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);
    progress = new QProgressBar();
    progress->setMaximumSize(90,20);
    progress->hide();

    btnShowProjectPane = new QPushButton(">");
    btnShowProjectPane->setCheckable(true);
    btnShowProjectPane->setMaximumWidth(20);
    connect(btnShowProjectPane,SIGNAL(clicked(bool)),this,SLOT(showProjectPane(bool)));

    btnShowStatusPane = new QPushButton("^");
    btnShowStatusPane->setCheckable(true);
    btnShowStatusPane->setMaximumWidth(20);
    connect(btnShowStatusPane,SIGNAL(clicked(bool)),this,SLOT(showStatusPane(bool)));

    programSize = new QLabel();
    programSize->setMinimumWidth(PROJECT_WIDTH+6);

    status = new QLabel();

    statusBar->addPermanentWidget(progress);
    statusBar->addWidget(btnShowProjectPane);
    statusBar->addWidget(programSize);
    statusBar->addWidget(btnShowStatusPane);
    statusBar->addWidget(status);
    statusBar->setMaximumHeight(22);

    this->setMinimumHeight(APPWINDOW_MIN_HEIGHT);

}

void MainSpinWindow::cStatusClicked(QString line)
{
    int n = 0;
    QRegExp regx(":[0-9]");
    QStringList fileList = line.split(regx);
    if(fileList.count() < 2)
        return;

    QString file = fileList[0];
    file = file.mid(file.lastIndexOf(":")+1);

    /* open file in tab if not there already */
    for(n = 0; n < editorTabs->count();n++) {
        if(editorTabs->tabText(n).contains(file)) {
            editorTabs->setCurrentIndex(n);
            break;
        }
        if(editors->at(n)->toolTip().contains(file)) {
            editorTabs->setCurrentIndex(n);
            break;
        }
    }

    if(n > editorTabs->count()-1) {
        if(QFile::exists(fileList[0])) {
            file = fileList[0];
            openFileName(file);
        }
        else
        if(QFile::exists(sourcePath(projectFile))) {
            file = sourcePath(projectFile)+fileList[0];
            openFileName(file);
        }
        else {
            return;
        }
    }

    line = line.mid(file.length());
    if(line.length() == 0)
        return;
    QStringList list = line.split(":",QString::SkipEmptyParts);

    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor != NULL)
    {
        n = QString(list[0]).toInt();
        QTextCursor c = editor->textCursor();
        c.movePosition(QTextCursor::Start);
        if(n > 0) {
            c.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,n-1);
            c.movePosition(QTextCursor::StartOfLine);
            c.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor,1);
        }
        editor->setTextCursor(c);
        editor->setFocus();
    }
}

void MainSpinWindow::spinStatusClicked(QString line)
{
    int n = 0;

    if(line.contains("error",Qt::CaseInsensitive) == false)
        return;
    if(line.indexOf("(") < 0)
        return;

    QString file = line.mid(0,line.indexOf("("));
    if(file.contains("..."))
        file = file.mid(file.indexOf("...")+3);

    // this is useful for C and SPIN
    if(file.contains(SPIN_EXTENSION, Qt::CaseInsensitive) == false)
        file += SPIN_EXTENSION;

    /* open file in tab if not there already */
    for(n = 0; n < editorTabs->count();n++) {
        if(editorTabs->tabText(n).contains(file)) {
            editorTabs->setCurrentIndex(n);
            break;
        }
        if(editors->at(n)->toolTip().contains(file)) {
            editorTabs->setCurrentIndex(n);
            break;
        }
    }

    if(n > editorTabs->count()-1) {
        if(QFile::exists(file)) {
            openFileName(file);
        }
        else
        if(QFile::exists(sourcePath(projectFile))) {
            file = sourcePath(projectFile)+file;
            openFileName(file);
        }
        else {
            return;
        }
    }

    QStringList list = line.split(QRegExp("[(,:)]"));
    // list should contain
    // 0 filename
    // 1 row
    // 2 column
    // rest of line
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor != NULL)
    {
        n = QString(list[1]).toInt();
        QTextCursor c = editor->textCursor();
        c.movePosition(QTextCursor::Start);
        if(n > 0) {
            c.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,n-1);
            c.movePosition(QTextCursor::StartOfLine);
            c.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor,1);
        }
        editor->setTextCursor(c);
        editor->setFocus();
    }
}

/*
 * Find error or warning in a file
 */
void MainSpinWindow::compileStatusClicked(void)
{
    QTextCursor cur = compileStatus->textCursor();
    QString line = cur.selectedText();
    /* if more than one line, we have a select all */
    QStringList lines = line.split(QChar::ParagraphSeparator);
    if(lines.length()>1)
        return;
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    compileStatus->setTextCursor(cur);
    line = cur.selectedText();

    if(isCProject()) {
        cStatusClicked(line);
    }
#ifdef SPIN
    else if(isSpinProject()) {
        spinStatusClicked(line);
    }
#endif
}

void MainSpinWindow::compilerChanged()
{
    if(isCProject()) {
        projectMenu->setEnabled(true);
        cbBoard->setEnabled(true);
    }
#ifdef SPIN
    else if(isSpinProject()) {
        projectMenu->setEnabled(false);
        cbBoard->setEnabled(false);
        int n = cbBoard->findText("NONE");
        if(n > -1) cbBoard->setCurrentIndex(n);
    }
#endif
}

void MainSpinWindow::projectTreeClicked(QModelIndex index)
{
    if(projectModel == NULL)
        return;
    projectIndex = index; // same as projectTree->currentIndex();
    if(projectTree->rightClick(false) && projectMenu->isEnabled())
        projectMenu->popup(QCursor::pos());
    else
        showProjectFile();
}

/*
 * don't allow adding output files
 */
bool MainSpinWindow::isOutputFile(QString file)
{
    bool rc = false;

    QString ext = file.mid(file.lastIndexOf("."));
    if(ext.length()) {
        ext = ext.toLower();
        if(ext == ".cog") {
            // don't copy .cog files
            rc = true;
        }
        else if(ext == ".dat") {
            // don't copy .dat files
            rc = true;
        }
        else if(ext == ".o") {
            // don't copy .o files
            rc = true;
        }
        else if(ext == ".out") {
            // don't copy .out files
            rc = true;
        }
        else if(ext == SIDE_EXTENSION) {
            // don't copy .side files
            rc = true;
        }
    }
    return rc;
}

/*
 * fileName can be short name or link name
 */
void MainSpinWindow::addProjectListFile(QString fileName)
{
    QString projstr = "";
    QStringList list;
    QString mainFile;

    QFile file(projectFile);
    if(file.exists()) {
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            projstr = file.readAll();
            file.close();
        }
        list = projstr.split("\n");
        mainFile = list[0];
        projstr = "";
        for(int n = 0; n < list.length(); n++) {
            QString arg = list[n];
            if(!arg.length())
                continue;
            if(arg.at(0) == '>')
                continue;
            projstr += arg + "\n";
        }
        projstr += fileName + "\n";
        list.clear();
#ifdef SPIN
        if(isSpinProject())
            list = projectOptions->getSpinOptions();
        else if(isCProject())
#endif
            list = projectOptions->getOptions();

        foreach(QString arg, list) {
            projstr += ">"+arg+"\n";
        }
        // save project file in english
        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }
    }
    updateProjectTree(sourcePath(projectFile)+mainFile);
}

/*
 * add a new project file
 * save new filelist and options to project.side file
 */
void MainSpinWindow::addProjectFile()
{
    fileDialog.setDirectory(sourcePath(projectFile));

    // this is on the wish list and not finished yet
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList files = fileDialog.getOpenFileNames(this, tr("Add File"), lastPath, tr(SOURCE_FILE_TYPES));

    foreach(QString fileName, files) {
        //fileName = fileDialog.getOpenFileName(this, tr("Add File"), lastPath, tr(SOURCE_FILE_TYPES));
        /*
         * Cancel makes filename blank. If fileName is blank, don't add.
         */
        if(fileName.length() == 0)
            return;
        /*
         * Don't let users add *.* as a file name.
         */
        if(fileName.contains('*'))
            return;

        lastPath = sourcePath(fileName);

        /* hmm, should be check for source files */
        if(isOutputFile(fileName) == false) {
            QFile reader(fileName);
            reader.copy(sourcePath(projectFile)+this->shortFileName(fileName));
            addProjectListFile(this->shortFileName(fileName));
        }
    }
}

/*
 * add a new project link
 * save new filelist and options to project.side file
 */
void MainSpinWindow::addProjectLink()
{
    fileDialog.setDirectory(sourcePath(projectFile));

    // this is on the wish list and not finished yet
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList files = fileDialog.getOpenFileNames(this, tr("Add Link to File"), lastPath, tr(SOURCE_FILE_TYPES));

    foreach(QString fileName, files) {
        //fileName = fileDialog.getOpenFileName(this, tr("Add File"), lastPath, tr(SOURCE_FILE_TYPES));
        /*
         * Cancel makes filename blank. If fileName is blank, don't add.
         */
        if(fileName.length() == 0)
            return;
        /*
         * Don't let users add *.* as a file name.
         */
        if(fileName.contains('*'))
            return;

        lastPath = sourcePath(fileName);

        if(isOutputFile(fileName) == false) {
            if(sourcePath(fileName).compare(sourcePath(this->projectFile)) == 0)
                fileName = this->shortFileName(fileName);
            else
                fileName = this->shortFileName(fileName)+FILELINK+fileName;
            addProjectListFile(fileName);
        }
    }
}


void MainSpinWindow::addProjectLibFile()
{
    // this is on the wish list and not finished yet
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList files = fileDialog.getOpenFileNames(this, tr("Add Library File"), lastPath, tr("Library *.a Files (*.a)"));

    foreach(QString fileName, files) {
        //fileName = fileDialog.getOpenFileName(this, tr("Add File"), lastPath, tr(SOURCE_FILE_TYPES));
        /*
         * Cancel makes filename blank. If fileName is blank, don't add.
         */
        if(fileName.length() == 0)
            return;
        /*
         * Don't let users add *.* as a file name.
         */
        if(fileName.contains('*'))
            return;

        lastPath = sourcePath(fileName);

        if(isOutputFile(fileName) == false) {
            QString ext = fileName.mid(fileName.lastIndexOf("."));
            if(ext.length()) {
                ext = ext.toLower();
                if(ext.compare(".a") == 0) {
                    if(isOutputFile(fileName) == false) {
                        if(sourcePath(fileName).compare(sourcePath(this->projectFile)) == 0)
                            fileName = this->shortFileName(fileName);
                        else
                            fileName = this->shortFileName(fileName)+FILELINK+fileName;
                    }
                    //fileName = this->shortFileName(fileName)+FILELINK+fileName;
                    addProjectListFile(fileName);
                }
            }
        }
    }
}

void MainSpinWindow::addProjectIncPath()
{
    QString fileName = QFileDialog::getExistingDirectory(this,tr("Select Include Folder"),lastPath,QFileDialog::ShowDirsOnly);
    if(fileName.length() < 1)
        return;

    /*
     * Cancel makes filename blank. If fileName is blank, don't add.
     */
    if(fileName.length() == 0)
        return;
    /*
     * Don't let users add *.* as a file name.
     */
    if(fileName.contains('*'))
        return;

    lastPath = sourcePath(fileName);

    QString s = QDir::fromNativeSeparators(fileName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    fileName = QDir::convertSeparators(s);

    if(isOutputFile(fileName) == false) {
        if(fileName.indexOf(".") < 0)  {
            fileName = "-I " + fileName;
            addProjectListFile(fileName);
        }
    }
}

void MainSpinWindow::addProjectLibPath()
{
    QString fileName = QFileDialog::getExistingDirectory(this,tr("Select Library Folder"),lastPath,QFileDialog::ShowDirsOnly);
    if(fileName.length() < 1)
        return;
    /*
     * Cancel makes filename blank. If fileName is blank, don't add.
     */
    if(fileName.length() == 0)
        return;
    /*
     * Don't let users add *.* as a file name.
     */
    if(fileName.contains('*'))
        return;

    lastPath = sourcePath(fileName);

    QString s = QDir::fromNativeSeparators(fileName);
    if(s.length() == 0)
        return;
    if(s.indexOf('/') > -1) {
        if(s.mid(s.length()-1) != "/")
            s += "/";
    }
    fileName = QDir::convertSeparators(s);

    if(isOutputFile(fileName) == false) {
        if(fileName.indexOf(".") < 0)  {
            fileName = "-L "+fileName;
            addProjectListFile(fileName);
        }
    }
}

/*
 * delete project source file.
 * save new filelist and options to project.side file
 */
void MainSpinWindow::deleteProjectFile()
{
    QString projstr = "";
    QString fileName = "";
    QStringList list;

    QVariant vs = projectModel->data(projectIndex, Qt::DisplayRole);
    if(vs.canConvert(QVariant::String))
    {
        fileName = vs.toString();
    }

    QString mainFile;

    QFile file(projectFile);
    if(file.exists()) {
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            projstr = file.readAll();
            file.close();
        }
        list = projstr.split("\n");
        mainFile = list[0];
        projstr = "";
        for(int n = 0; n < list.length(); n++) {
            QString arg = list[n];
            if(!arg.length())
                continue;
            if(arg.at(0) == '>')
                continue;
            if(!n || fileName != arg)
                projstr += arg + "\n";
        }
        list.clear();
#ifdef SPIN
        if(isSpinProject())
            list = projectOptions->getSpinOptions();
        else if(isCProject())
#endif
            list = projectOptions->getOptions();

        foreach(QString arg, list) {
            projstr += ">"+arg+"\n";
        }
        // save project file in english
        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }
    }
    updateProjectTree(sourcePath(projectFile)+mainFile);
}

void MainSpinWindow::showProjectFile()
{
    QString fileName;

    QVariant vs = projectModel->data(projectIndex, Qt::DisplayRole);
    if(vs.canConvert(QVariant::String))
    {
        fileName = vs.toString();
        fileName = fileName.trimmed();

        /* .a libraries are allowed in project list, but not .o, etc...
         */
        if(fileName.contains(".a"))
            return;

        /*
         * if tab is already opened, just display it
         */
        int tabs = editorTabs->count();
        for(int n = 0; n < tabs; n++) {
            QString tab = editorTabs->tabText(n);
            if(tab.contains("*")) {
                tab = tab.mid(0,tab.indexOf("*"));
                tab = tab.trimmed();
            }
            if(tab.compare(this->shortFileName(fileName)) == 0) {
                editorTabs->setCurrentIndex(n);
                return;
            }
        }

        /* openFileName knows how to read spin files
         * If name has FILELINK it's a link.
         */
        if(fileName.contains(FILELINK)) {
            fileName = fileName.mid(fileName.indexOf(FILELINK)+QString(FILELINK).length());
            openFileName(fileName);
        }
        else {
            if(isCProject()) {
                openFileName(sourcePath(projectFile)+fileName);
            }
#ifdef SPIN
            else if(isSpinProject()) {

                /* Spin files can be case-insensitive. Deal with it.
                 */
                QString lib = propDialog->getSpinLibraryStr();
                QString fs = sourcePath(projectFile)+fileName;

                if(QFile::exists(fs))
                    openFileName(sourcePath(projectFile)+fileName);
                else if(QFile::exists(lib+fileName))
                    openFileName(lib+fileName);
                else {
                    QDir dir;
                    QStringList list;
                    QString shortfile = this->shortFileName(fs);
                    dir.setPath(sourcePath(projectFile));
                    list = dir.entryList();
                    foreach(QString s, list) {
                        if(s.contains(shortfile,Qt::CaseInsensitive)) {
                            openFileName(sourcePath(projectFile)+s);
                            return;
                        }
                    }
                    dir.setPath(lib);
                    list = dir.entryList();
                    foreach(QString s, list) {
                        if(s.contains(shortfile,Qt::CaseInsensitive)) {
                            openFileName(lib+s);
                            return;
                        }
                    }
                }

            }
#endif
        }
    }
}

/*
 * save project file with options.
 */
void MainSpinWindow::saveProjectOptions()
{
    if(projectModel == NULL)
        return;

    if(projectFile.length() > 0)
        setWindowTitle(QString(ASideGuiKey)+" "+QDir::convertSeparators(projectFile));
#ifdef SPIN
    if(isSpinProject())
        saveSpinProjectOptions();
    else if(isCProject())
#endif
        saveManagedProjectOptions();
}

void MainSpinWindow::saveSpinProjectOptions()
{
    QString projstr = "";
    QStringList list;

    QFile file(projectFile);
    if(file.exists()) {

        /* read project file
         */
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            projstr = file.readAll();
            file.close();
        }

        /* get the spin project */
        QString fileName = sourcePath(projectFile)+projstr.mid(0,projstr.indexOf("\n"));
        projstr = "";

        QString projName = shortFileName(projectFile);
        if(projectModel != NULL) delete projectModel;
        projectModel = new CBuildTree(projName, this);
#ifdef SPIN
        /* for spin-side we always parse the program and stuff the file list */
        list = spinParser.spinFileTree(fileName, propDialog->getSpinLibraryStr());
        for(int n = 0; n < list.count(); n ++) {
            QString arg = list[n];
            qDebug() << arg;
            projstr += arg + "\n";
            projectModel->addRootItem(arg);
        }

        list.clear();
        list = projectOptions->getSpinOptions();
#endif
        /* add options */
        foreach(QString arg, list) {
            if(arg.contains(ProjectOptions::board+"::"))
                projstr += ">"+ProjectOptions::board+"::"+cbBoard->currentText()+"\n";
            else
                projstr += ">"+arg+"\n";
        }

        /* save project file in english only ok */
        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }

        projectTree->setWindowTitle(projName);
        projectTree->setModel(projectModel);
        projectTree->hide();
        projectTree->show();
    }
}

void MainSpinWindow::saveManagedProjectOptions()
{
    QString projstr = "";
    QStringList list;
    QFile file(projectFile);
    if(file.exists()) {
        /* read project file
         */
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            projstr = file.readAll();
            file.close();
        }
        /* make a new project file text string.
         * first add the source files, then add options.
         * finally save file.
         */
        list = projstr.split("\n");
        projstr = "";
        /* add source files */
        for(int n = 0; n < list.length(); n++) {
            QString arg = list[n];
            if(!arg.length())
                continue;
            if(arg.at(0) == '>')
                continue;
            else
                projstr += arg + "\n";
        }

        list.clear();
        list = projectOptions->getOptions();

        /* add options */
        foreach(QString arg, list) {
            if(arg.contains(ProjectOptions::board+"::"))
                projstr += ">"+ProjectOptions::board+"::"+cbBoard->currentText()+"\n";
            else
                projstr += ">"+arg+"\n";
        }

        /* save project file in english only ok */
        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }
    }
}
/*
 * update project tree and options by reading from project.side file
 */
void MainSpinWindow::updateProjectTree(QString fileName)
{
    QString projName = this->shortFileName(fileName);
    projName = projName.mid(0,projName.lastIndexOf("."));
    projName += SIDE_EXTENSION;

    basicPath = sourcePath(fileName);
    projectFile = basicPath+projName;
    setWindowTitle(QString(ASideGuiKey)+" "+QDir::convertSeparators(projectFile));

    if(projectModel != NULL) delete projectModel;
    projectModel = new CBuildTree(projName, this);
#ifdef SPIN
    if(fileName.contains(SPIN_EXTENSION,Qt::CaseInsensitive)) {
        projectOptions->setCompiler(SPIN_TEXT);
    }
    else {
        QFile proj(projectFile);
        QString type;
        if(proj.open(QFile::ReadOnly | QFile::Text)) {
            type = proj.readAll();
            proj.close();
            if(type.contains(">compiler=SPIN",Qt::CaseInsensitive))
                projectOptions->setCompiler(SPIN_TEXT);
            else if(type.contains(">compiler=C++",Qt::CaseInsensitive))
                projectOptions->setCompiler("C++");
            else
                projectOptions->setCompiler("C");
        }
    }
#else
    QFile proj(projectFile);
    QString type;
    if(proj.open(QFile::ReadOnly | QFile::Text)) {
        type = proj.readAll();
        proj.close();
        if(type.contains(">compiler=C++",Qt::CaseInsensitive))
            projectOptions->setCompiler("C++");
        else
            projectOptions->setCompiler("C");
    }
#endif

    /* in the case of a spin project, we need to parse a tree.
     * in other project cases, we use the project manager.
     */
#ifdef SPIN
    if(isSpinProject())
        updateSpinProjectTree(fileName, projName);
    else if(isCProject())
#endif
        updateManagedProjectTree(fileName, projName);
}

void MainSpinWindow::updateSpinProjectTree(QString fileName, QString projName)
{
#ifdef SPIN
    QFile file(projectFile);

    /* for spin-side we always parse the program and stuff the file list */

    QStringList flist = spinParser.spinFileTree(fileName, propDialog->getSpinLibraryStr());
    for(int n = 0; n < flist.count(); n ++) {
        QString s = flist[n];
        qDebug() << s;
        projectModel->addRootItem(s);
    }

    if(!file.exists()) {
        /* no pre-existing file, make one with source as top/main entry.
         * project file is in english.
         */
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            for(int n = 0; n < flist.count(); n ++) {
                QString s = QString(flist[n]).trimmed();
                file.write((shortFileName(s)+"\n").toAscii());
            }
            QStringList list = projectOptions->getSpinOptions();
            for(int n = 0; n < list.count(); n ++) {
                file.write(">"+QString(list[n]).toAscii()+"\n");
            }
            file.close();
            //projectModel->addRootItem(this->shortFileName(fileName));
        }
    }
    else {

        /* pre-existing side file. read and modify it.
         * project file is in english
         */
        QString s = "";
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            s = file.readAll();
            file.close();
        }

        QStringList list = s.split("\n");

        /*
         * add sorting feature - parameters get sorted too, but placement is not important.
         */
        QString main = list.at(0);
        QString mains = main.mid(0,main.lastIndexOf("."));
        QString projs = projName.mid(0,projName.lastIndexOf("."));

        if(mains.compare(projs) == 0 && list.length() > 1) {
            for(int n = list.count()-1; n > -1; n--) {
                QString s = list[n];
                if(s.indexOf(">") != 0)
                    list.removeAt(n);
            }
            for(int n = flist.count()-1; n > -1; n--) {
                QString s = QString(flist[n]).trimmed();
                list.insert(0,s);
            }
        }

        /* replace options */
        projectOptions->clearOptions();
        foreach(QString arg, list) {
            if(!arg.length())
                continue;
            if(arg.at(0) == '>') {
                if(arg.contains(ProjectOptions::board+"::")) {
                    QStringList barr = arg.split("::");
                    if(barr.count() > 0) {
                        QString board = barr.at(1);
                        projectOptions->setBoardType(board);
                        for(int n = cbBoard->count()-1; n >= 0; n--) {
                            QString cbstr = cbBoard->itemText(n);
                            if(board.compare(cbstr) == 0) {
                                cbBoard->setCurrentIndex(n);
                                break;
                            }
                        }
                    }
                }
                else {
                    projectOptions->setSpinOptions(arg);
                }
            }
        }

        file.remove();
        if(file.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
            for(int n = 0; n < list.count(); n++) {
                QString s = list[n];
                file.write(s.toAscii()+"\n");
            }
            file.close();
        }
    }

    projectTree->setWindowTitle(projName);
    projectTree->setModel(projectModel);
    projectTree->hide();
    projectTree->show();
#endif
}

void MainSpinWindow::updateManagedProjectTree(QString fileName, QString projName)
{
    QFile file(projectFile);
    if(!file.exists()) {
        /* no pre-existing file, make one with source as top/main entry.
         * project file is in english.
         */
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(this->shortFileName(fileName).toAscii());
            projectModel->addRootItem(this->shortFileName(fileName));
            file.close();
        }
    }
    else {
        /* pre-existing side file. read and modify it.
         * project file is in english
         */
        QString s = "";
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            s = file.readAll();
            file.close();
        }
        QStringList list = s.split("\n");

        /*
         * add sorting feature - parameters get sorted too, but placement is not important.
         */
        QString main = list.at(0);
        QString mains = main.mid(0,main.lastIndexOf("."));
        QString projs = projName.mid(0,projName.lastIndexOf("."));
        if(mains.compare(projs) == 0 && list.length() > 1) {
            QString s;
            list.removeAt(0);
            list.sort();
            s = list.at(0);
            if(s.length() == 0)
                list.removeAt(0);
            list.insert(0,main);
        }
        /* replace options */
        projectOptions->clearOptions();
        foreach(QString arg, list) {
            if(!arg.length())
                continue;
            if(arg.at(0) == '>') {
                if(arg.contains(ProjectOptions::board+"::")) {
                    QStringList barr = arg.split("::");
                    if(barr.count() > 0) {
                        QString board = barr.at(1);
                        projectOptions->setBoardType(board);
                        for(int n = cbBoard->count()-1; n >= 0; n--) {
                            QString cbstr = cbBoard->itemText(n);
                            if(board.compare(cbstr) == 0) {
                                cbBoard->setCurrentIndex(n);
                                break;
                            }
                        }
                    }
                }
                else {
                    projectOptions->setOptions(arg);
                }
            }
            else {
                projectModel->addRootItem(arg);
            }
        }
    }

    projectTree->setWindowTitle(projName);
    projectTree->setModel(projectModel);
    projectTree->hide();
    projectTree->show();
}

/*
 * show assembly for a .c file
 */
void MainSpinWindow::showAssemblyFile()
{
    QString fileName;
    QVariant vs = projectModel->data(projectIndex, Qt::DisplayRole);
    if(vs.canConvert(QVariant::String))
        fileName = vs.toString();

    if(makeDebugFiles(fileName))
        return;
    QString outfile = fileName.mid(0,fileName.lastIndexOf("."));
    if(outfile.contains(FILELINK)) {
        outfile = outfile.mid(outfile.indexOf(FILELINK)+QString(FILELINK).length());
        openFileName(outfile+SHOW_ASM_EXTENTION);
    }
    else {
        openFileName(sourcePath(projectFile)+outfile+SHOW_ASM_EXTENTION);
    }
}

void MainSpinWindow::showMapFile()
{
    QString fileName;
    QVariant vs = projectModel->data(projectIndex, Qt::DisplayRole);
    if(vs.canConvert(QVariant::String))
        fileName = vs.toString();

    QString outfile = fileName.mid(0,fileName.lastIndexOf("."));
    runBuild("-Xlinker -Map="+outfile+SHOW_MAP_EXTENTION);

    openFileName(sourcePath(projectFile)+outfile+SHOW_MAP_EXTENTION);
}

/*
 * make debug info for a .c file
 */
int MainSpinWindow::makeDebugFiles(QString fileName)
{
    int rc = -1;

    if(fileName.length() == 0)
        return rc;

    /* save files before compiling debug info */
    checkAndSaveFiles();
    selectBuilder();
    rc = builder->makeDebugFiles(fileName, projectFile, aSideCompiler);

    return rc;
}

void MainSpinWindow::enumeratePorts()
{
    if(cbPort != NULL) cbPort->clear();

    friendlyPortName.clear();
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    QStringList stringlist;
    QString name;
    stringlist << "List of ports:";
    for (int i = 0; i < ports.size(); i++) {
        stringlist << "port name:" << ports.at(i).portName;
        stringlist << "friendly name:" << ports.at(i).friendName;
        stringlist << "physical name:" << ports.at(i).physName;
        stringlist << "enumerator name:" << ports.at(i).enumName;
        stringlist << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
        stringlist << "product ID:" << QString::number(ports.at(i).productID, 16);
        stringlist << "===================================";
#if defined(Q_WS_WIN32)
        name = ports.at(i).portName;
        if(name.lastIndexOf('\\') > -1)
            name = name.mid(name.lastIndexOf('\\')+1);
        if(name.contains(QString("LPT"),Qt::CaseInsensitive) == false) {
            friendlyPortName.append(ports.at(i).friendName);
            cbPort->addItem(name);
        }
#elif defined(Q_WS_MAC)
        name = ports.at(i).portName;
        if(name.indexOf("usbserial") > -1) {
            friendlyPortName.append(ports.at(i).friendName);
            cbPort->addItem(name);
        }
#else
        name = ports.at(i).physName;
        friendlyPortName.append(ports.at(i).friendName);
        if(name.indexOf("usb",0,Qt::CaseInsensitive) > 0)
            cbPort->insertItem(0,name);
        else
            cbPort->addItem(name);
#endif
    }
    cbPort->setCurrentIndex(0);
}

void MainSpinWindow::connectButton()
{
    if(btnConnected->isChecked()) {
        term->getEditor()->setPortEnable(true);
        term->show();
        term->setPortEnabled(true);
        term->activateWindow();
        term->getEditor()->setFocus();
        portListener->open();
    }
    else {
        portListener->close();
        term->hide();
    }
}

void MainSpinWindow::portResetButton()
{
    bool rts = false;
    QString port = cbPort->currentText();
    if(port.length() == 0) {
        QMessageBox::information(this, tr("Port Required"), tr("Please select a port"), QMessageBox::Ok);
        return;
    }

    if(this->propDialog->getResetType() == Properties::CFG) {
        QString bname = this->cbBoard->currentText();
        ASideBoard* board = aSideConfig->getBoardData(bname);
        if(board == NULL) {
            QMessageBox::critical(this,tr("Can't Reset"),tr("Can't reset by CFG with an empty board type."),QMessageBox::Ok);
            return;
        }
        QString reset = board->get(ASideBoard::reset);
        if(reset.length() == 0)
            rts = false;
        else
        if(reset.contains("RTS",Qt::CaseInsensitive))
            rts = true;
        else
        if(reset.contains("DTR",Qt::CaseInsensitive))
            rts = false;
        else
            rts = false;
    }
    else
    if(this->propDialog->getResetType() == Properties::RTS) {
        rts = true;
    }
    else
    if(this->propDialog->getResetType() == Properties::DTR) {
        rts = false;
    }

    bool isopen = portListener->isOpen();
    if(isopen == false)
        portListener->open();
    if(rts) {
        portListener->setRts(true);
        Sleeper::ms(50);
        portListener->setRts(false);
    }
    else {
        portListener->setDtr(true);
        Sleeper::ms(50);
        portListener->setDtr(false);
    }
    if(isopen == false)
        portListener->close();
}

QString MainSpinWindow::shortFileName(QString fileName)
{
    QString rets;
    if(fileName.indexOf('/') > -1)
        rets = fileName.mid(fileName.lastIndexOf('/')+1);
    else if(fileName.indexOf('\\') > -1)
        rets = fileName.mid(fileName.lastIndexOf('\\')+1);
    else
        rets = fileName.mid(fileName.lastIndexOf(aSideSeparator)+1);
    return rets;
}

void MainSpinWindow::initBoardTypes()
{
    cbBoard->clear();

    QFile file;
    if(file.exists(aSideCfgFile))
    {
        /* load boards in case there were changes */
        aSideConfig->loadBoards(aSideCfgFile);
    }

    /* get board types */
    QStringList boards = aSideConfig->getBoardNames();
    boards.sort();
    for(int n = 0; n < boards.count(); n++)
        cbBoard->addItem(boards.at(n));

    QVariant lastboardv = settings->value(lastBoardNameKey);
    /* read last board/port to make user happy */
    if(lastboardv.canConvert(QVariant::String))
        boardName = lastboardv.toString();

    /* setup the first board displayed in the combo box */
    if(cbBoard->count() > 0) {
        int ndx = 0;
        if(boardName.length() != 0) {
            for(int n = cbBoard->count()-1; n > -1; n--)
                if(cbBoard->itemText(n) == boardName)
                {
                    ndx = n;
                    break;
                }
        }
        setCurrentBoard(ndx);
    }
}

void MainSpinWindow::setupEditor()
{
    Editor *editor = new Editor(gdb, &spinParser, this);
    editor->setTabStopWidth(propDialog->getTabSpaces()*10);

    /* font is user's preference */
    editor->setFont(editorFont);
    editor->setLineWrapMode(Editor::NoWrap);
    connect(editor,SIGNAL(textChanged()),this,SLOT(fileChanged()));
    editors->append(editor);
}

void MainSpinWindow::setEditorTab(int num, QString shortName, QString fileName, QString text)
{
    Editor *editor = editors->at(num);
    fileChangeDisable = true;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    disconnect(editor,SIGNAL(textChanged()),this,SLOT(fileChanged()));
    editor->setPlainText(text);
    editor->setHighlights(shortName);
    connect(editor,SIGNAL(textChanged()),this,SLOT(fileChanged()));
    QApplication::restoreOverrideCursor();
    fileChangeDisable = false;
    editorTabs->setTabText(num,shortName);
    editorTabs->setTabToolTip(num,fileName);
    editorTabs->setCurrentIndex(num);
}

/*
 * TODO: Why don't icons show up in linux? deferred.
 * QtCreator has the same problem Windows OK, Linux not OK.
 */
void MainSpinWindow::setupFileMenu()
{
    fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(QIcon(":/images/newfile.png"), tr(NewFile), this, SLOT(newFile()), QKeySequence::New);
    fileMenu->addAction(QIcon(":/images/openfile.png"), tr(OpenFile), this, SLOT(openFile()), QKeySequence::Open);
    fileMenu->addAction(QIcon(":/images/savefile.png"), tr(SaveFile), this, SLOT(saveFile()), QKeySequence::Save);
    fileMenu->addAction(QIcon(":/images/saveasfile2.png"), tr(SaveAsFile), this, SLOT(saveAsFile()),QKeySequence::SaveAs);

    fileMenu->addAction(QIcon(":/images/Delete.png"),tr("Close"), this, SLOT(closeFile()));
    fileMenu->addAction(tr("Close All"), this, SLOT(closeAll()));
    fileMenu->addAction(QIcon(":/images/print.png"), tr("Print"), this, SLOT(printFile()), QKeySequence::Print);

    // recent file actions
    separatorFileAct = fileMenu->addSeparator();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),this, SLOT(openRecentFile()));
    }

    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);

    updateRecentFileActions();

    fileMenu->addSeparator();

    fileMenu->addAction(QIcon(":/images/exit.png"), tr("E&xit"), this, SLOT(quitProgram()), QKeySequence::Quit);

    projMenu = new QMenu(tr("&Project"), this);
    menuBar()->addMenu(projMenu);

    projMenu->addAction(QIcon(":/images/newproj.png"), tr("New Project"), this, SLOT(newProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_N);
    projMenu->addAction(QIcon(":/images/openproj.png"), tr("Open Project"), this, SLOT(openProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_O);
    projMenu->addAction(QIcon(":/images/saveasproj.png"), tr(SaveAsProject), this, SLOT(saveAsProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_S);
    //projMenu->addAction(QIcon(":/images/cloneproj.png"), tr(CloneProject), this, SLOT(cloneProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_C);
    projMenu->addAction(tr(CloneProject), this, SLOT(cloneProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_C);
    projMenu->addAction(QIcon(":/images/closeproj.png"), tr("Save and Close Project"), this, SLOT(closeProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_X);
    projMenu->addAction(QIcon(":/images/project.png"), tr("Set Project"), this, SLOT(setProject()), Qt::Key_F4);
    //projMenu->addAction(QIcon(":/images/hardware.png"), tr("Load Board Types"), this, SLOT(hardware()), Qt::Key_F6);

    // recent project actions
    separatorProjectAct = projMenu->addSeparator();

    for (int i = 0; i < MaxRecentProjects; ++i) {
        recentProjectActs[i] = new QAction(this);
        recentProjectActs[i]->setVisible(false);
        connect(recentProjectActs[i], SIGNAL(triggered()),this, SLOT(openRecentProject()));
    }

    for (int i = 0; i < MaxRecentProjects; ++i)
        projMenu->addAction(recentProjectActs[i]);

    updateRecentProjectActions();

    projMenu->addSeparator();
    projMenu->addAction(QIcon(":/images/properties.png"), tr("Properties"), this, SLOT(properties()), Qt::Key_F6);

    QMenu *editMenu = new QMenu(tr("&Edit"), this);
    menuBar()->addMenu(editMenu);

    editMenu->addAction(QIcon(":/images/copy.png"), tr("Copy"), this, SLOT(copyFromFile()), QKeySequence::Copy);
    editMenu->addAction(QIcon(":/images/cut.png"), tr("Cut"), this, SLOT(cutFromFile()), QKeySequence::Cut);
    editMenu->addAction(QIcon(":/images/paste.png"), tr("Paste"), this, SLOT(pasteToFile()), QKeySequence::Paste);

/*
    editMenu->addSeparator();
    editMenu->addAction(tr("Edit Command"), this, SLOT(editCommand()), Qt::CTRL + Qt::Key_Colon);
    editMenu->addAction(tr("System Command"), this, SLOT(systemCommand()), Qt::CTRL + Qt::Key_Semicolon);
*/
    editMenu->addSeparator();
    editMenu->addAction(QIcon(":/images/find.png"), tr("&Find and Replace"), this, SLOT(replaceInFile()), QKeySequence::Find);

    editMenu->addSeparator();
    editMenu->addAction(QIcon(":/images/redo.png"), tr("&Redo"), this, SLOT(redoChange()), QKeySequence::Redo);
    editMenu->addAction(QIcon(":/images/undo.png"), tr("&Undo"), this, SLOT(undoChange()), QKeySequence::Undo);

    toolsMenu = new QMenu(tr("&Tools"), this);
    menuBar()->addMenu(toolsMenu);

    QString viewstr = this->simpleViewType ? tr(ProjectView) : tr(SimpleView);
    toolsMenu->addAction(viewstr,this,SLOT(toggleSimpleView()));

#if defined(SD_TOOLS)
    //toolsMenu->addAction(QIcon(":/images/flashdrive.png"), tr("Save .PEX to Local SD Card"), this, SLOT(savePexFile()));
    toolsMenu->addAction(QIcon(":/images/download.png"), tr("File to SD Card"), this, SLOT(downloadSdCard()));
#endif

    if(ctags->enabled()) {
        toolsMenu->addSeparator();
        toolsMenu->addAction(QIcon(":/images/back.png"),tr("Go &Back"), this, SLOT(prevDeclaration()), QKeySequence::Back);
        toolsMenu->addAction(QIcon(":/images/forward.png"),tr("Browse Declaration"), this, SLOT(findDeclaration()), QKeySequence::Forward);
    }

    toolsMenu->addSeparator();
    toolsMenu->addAction(QIcon(":/images/Brush.png"), tr("Font"), this, SLOT(fontDialog()));
    //toolsMenu->addAction(QIcon(":/images/resize-plus.png"), tr("Bigger Font"), this, SLOT(fontBigger()), QKeySequence::ZoomIn);
    //toolsMenu->addAction(QIcon(":/images/resize-plus.png"), tr("Bigger Font"), this, SLOT(fontBigger()), QKeySequence(Qt::CTRL+Qt::Key_Equal));
    toolsMenu->addAction(QIcon(":/images/resize-minus.png"), tr("Smaller Font"), this, SLOT(fontSmaller()), QKeySequence::ZoomOut);

    /* special provision for bigger fonts to use default ZoomIn or Ctrl+= */
    QAction *bigger = new QAction(QIcon(":/images/resize-plus.png"), tr("Bigger Font"), this);
    QList<QKeySequence> biggerKeys;
    biggerKeys.append(QKeySequence::ZoomIn);
    biggerKeys.append(QKeySequence(Qt::CTRL+Qt::Key_Equal));
    bigger->setShortcuts(biggerKeys);
    connect(bigger,SIGNAL(triggered()),this,SLOT(fontBigger()));

    /* insert action before smaller font action */
    QList<QAction*> alist = toolsMenu->actions();
    QAction *last = alist.last();
    toolsMenu->insertAction(last,bigger);

    toolsMenu->addSeparator();
    toolsMenu->addAction(tr("Next Tab"),this,SLOT(changeTab(bool)),QKeySequence::NextChild);

    QMenu *programMenu = new QMenu(tr("&Program"), this);
    menuBar()->addMenu(programMenu);

    programMenu->addAction(QIcon(":/images/runconsole.png"), tr("Run Console"), this, SLOT(programDebug()), Qt::Key_F8);
    programMenu->addAction(QIcon(":/images/build.png"), tr("Build Project"), this, SLOT(programBuild()), Qt::Key_F9);
    programMenu->addAction(QIcon(":/images/run.png"), tr("Run Project"), this, SLOT(programRun()), Qt::Key_F10);
    programMenu->addAction(QIcon(":/images/burnee.png"), tr("Burn Project"), this, SLOT(programBurnEE()), Qt::Key_F11);

#if defined(GDBENABLE)
    QMenu *debugMenu = new QMenu(tr("&Debug"), this);
    menuBar()->addMenu(debugMenu);

    debugMenu->addAction(tr("&Debug Start"), this, SLOT(debugCompileLoad()), Qt::Key_F5);
    debugMenu->addAction(tr("&Continue"), this, SLOT(gdbContinue()), Qt::ALT+Qt::Key_R);
    debugMenu->addAction(tr("&Next Line"), this, SLOT(gdbNext()), Qt::ALT+Qt::Key_N);
    debugMenu->addAction(tr("&Step In"), this, SLOT(gdbStep()), Qt::ALT+Qt::Key_S);
    debugMenu->addAction(tr("&Finish Function"), this, SLOT(gdbFinish()), Qt::ALT+Qt::Key_F);
    debugMenu->addAction(tr("&Backtrace"), this, SLOT(gdbBacktrace()), Qt::ALT+Qt::Key_B);
    debugMenu->addAction(tr("&Until"), this, SLOT(gdbUntil()), Qt::ALT+Qt::Key_U);
    debugMenu->addAction(tr("&Interrupt"), this, SLOT(gdbInterrupt()), Qt::ALT+Qt::Key_I);
    debugMenu->addAction(tr("&Kill"), this, SLOT(gdbKill()), Qt::ALT+Qt::Key_K);
    connect(gdb,SIGNAL(breakEvent()),this,SLOT(gdbBreak()));
#endif

    /* add editor popup context menu */
    edpopup = new QMenu(tr("Editor Popup"), this);
    edpopup->addAction(QIcon(":/images/undo.png"),tr("Undo"),this,SLOT(undoChange()));
    edpopup->addAction(QIcon(":/images/redo.png"),tr("Redo"),this,SLOT(redoChange()));
    edpopup->addSeparator();
    edpopup->addAction(QIcon(":/images/copy.png"),tr("Copy"),this,SLOT(copyFromFile()));
    edpopup->addAction(QIcon(":/images/cut.png"),tr("Cut"),this,SLOT(cutFromFile()));
    edpopup->addAction(QIcon(":/images/paste.png"),tr("Paste"),this,SLOT(pasteToFile()));
}


void MainSpinWindow::setupToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    QToolButton *btnFileNew = new QToolButton(this);
    QToolButton *btnFileOpen = new QToolButton(this);
    QToolButton *btnFileSave = new QToolButton(this);
    QToolButton *btnFileSaveAs = new QToolButton(this);
    QToolButton *btnFilePrint = new QToolButton(this);

    addToolButton(fileToolBar, btnFileNew, QString(":/images/newfile.png"));
    addToolButton(fileToolBar, btnFileOpen, QString(":/images/openfile.png"));
    addToolButton(fileToolBar, btnFileSave, QString(":/images/savefile.png"));
    addToolButton(fileToolBar, btnFileSaveAs, QString(":/images/saveasfile2.png"));
    addToolButton(fileToolBar, btnFilePrint, QString(":/images/print.png"));

    connect(btnFileNew,SIGNAL(clicked()),this,SLOT(newFile()));
    connect(btnFileOpen,SIGNAL(clicked()),this,SLOT(openFile()));
    connect(btnFileSave,SIGNAL(clicked()),this,SLOT(saveFile()));
    connect(btnFileSaveAs,SIGNAL(clicked()),this,SLOT(saveAsFile()));
    connect(btnFilePrint,SIGNAL(clicked()),this,SLOT(printFile()));

    btnFileNew->setToolTip(tr("New File"));
    btnFileOpen->setToolTip(tr("Open File"));
    btnFileSave->setToolTip(tr("Save File"));
    btnFileSaveAs->setToolTip(tr("Save As File"));
    btnFilePrint->setToolTip(tr("Print"));
    //btnFileZip->setToolTip(tr("Archive"));

    /*
     * Project Toobar
     */
    projToolBar = addToolBar(tr("Project"));
    QToolButton *btnProjectNew = new QToolButton(this);
    QToolButton *btnProjectOpen = new QToolButton(this);
    //QToolButton *btnProjectClone = new QToolButton(this);
    QToolButton *btnProjectSaveAs = new QToolButton(this);
    QToolButton *btnProjectClose = new QToolButton(this);

    propToolBar = addToolBar(tr("Misc. Project"));
    QToolButton *btnProjectApp = new QToolButton(this);


    addToolButton(projToolBar, btnProjectNew, QString(":/images/newproj.png"));
    addToolButton(projToolBar, btnProjectOpen, QString(":/images/openproj.png"));
    addToolButton(projToolBar, btnProjectSaveAs, QString(":/images/saveasproj.png"));
    //addToolButton(projToolBar, btnProjectClone, QString(":/images/cloneproj2.png"));
    addToolButton(projToolBar, btnProjectClose, QString(":/images/closeproj.png"));
    addToolButton(propToolBar, btnProjectApp, QString(":/images/project.png"));

    connect(btnProjectNew,SIGNAL(clicked()),this,SLOT(newProject()));
    connect(btnProjectOpen,SIGNAL(clicked()),this,SLOT(openProject()));
    connect(btnProjectSaveAs,SIGNAL(clicked()),this,SLOT(saveAsProject()));
    //connect(btnProjectClone,SIGNAL(clicked()),this,SLOT(cloneProject()));
    connect(btnProjectClose,SIGNAL(clicked()),this,SLOT(closeProject()));
    connect(btnProjectApp,SIGNAL(clicked()),this,SLOT(setProject()));

    btnProjectNew->setToolTip(tr("New Project"));
    btnProjectOpen->setToolTip(tr("Open Project"));
    //btnProjectClone->setToolTip(tr("Clone Project"));
    btnProjectSaveAs->setToolTip(tr("Save As Project"));
    btnProjectClose->setToolTip(tr("Save and Close Project"));
    btnProjectApp->setToolTip(tr("Set Project to Current File"));

    //propToolBar = addToolBar(tr("Properties"));
/*
    QToolButton *btnProjectBoard = new QToolButton(this);
    addToolButton(propToolBar, btnProjectBoard, QString(":/images/hardware.png"));
    connect(btnProjectBoard,SIGNAL(clicked()),this,SLOT(hardware()));
    btnProjectBoard->setToolTip(tr("Configuration"));
*/
    QToolButton *btnProjectProperties = new QToolButton(this);
    addToolButton(propToolBar, btnProjectProperties, QString(":/images/properties.png"));
    connect(btnProjectProperties,SIGNAL(clicked()),this,SLOT(properties()));
    btnProjectProperties->setToolTip(tr("Properties"));

    if(ctags->enabled()) {
        browseToolBar = addToolBar(tr("Browser"));
        btnBrowseBack = new QToolButton(this);
        addToolButton(browseToolBar, btnBrowseBack, QString(":/images/back.png"));
        connect(btnBrowseBack,SIGNAL(clicked()),this,SLOT(prevDeclaration()));
        btnBrowseBack->setToolTip(tr("Back"));
        btnBrowseBack->setEnabled(false);

        btnFindDef = new QToolButton(this);
        addToolButton(browseToolBar, btnFindDef, QString(":/images/forward.png"));
        connect(btnFindDef,SIGNAL(clicked()),this,SLOT(findDeclaration()));
        btnFindDef->setToolTip(tr("Browse (Ctrl+Left Click)"));
    }

#if defined(SD_TOOLS)
    QToolBar *toolsToolBar = addToolBar(tr("Tools"));
    //QToolButton *btnSaveToSdCard = new QToolButton(this);
    //addToolButton(toolsToolBar, btnSaveToSdCard, QString(":/images/flashdrive.png"));
    //connect(btnSaveToSdCard, SIGNAL(clicked()),this,SLOT(savePexFile()));
    //btnSaveToSdCard->setToolTip(tr("Save AUTOEXEC.PEX to Local SD Card."));

    btnDownloadSdCard = new QToolButton(this);
    addToolButton(toolsToolBar, btnDownloadSdCard, QString(":/images/download.png"));
    connect(btnDownloadSdCard, SIGNAL(clicked()),this,SLOT(downloadSdCard()));
    btnDownloadSdCard->setToolTip(tr("File to SD Card."));

#endif

    programToolBar = addToolBar(tr("Program"));
    btnProgramDebugTerm = new QToolButton(this);
    btnProgramRun = new QToolButton(this);
    QToolButton *btnProgramStopBuild = new QToolButton(this);
    QToolButton *btnProgramBuild = new QToolButton(this);
    QToolButton *btnProgramBurnEEP = new QToolButton(this);

    addToolButton(programToolBar, btnProgramStopBuild, QString(":/images/Abort.png"));
    addToolButton(programToolBar, btnProgramBuild, QString(":/images/build.png"));
    addToolButton(programToolBar, btnProgramBurnEEP, QString(":/images/burnee.png"));
    addToolButton(programToolBar, btnProgramRun, QString(":/images/run.png"));
    addToolButton(programToolBar, btnProgramDebugTerm, QString(":/images/runconsole.png"));

    connect(btnProgramStopBuild,SIGNAL(clicked()),this,SLOT(programStopBuild()));
    connect(btnProgramBuild,SIGNAL(clicked()),this,SLOT(programBuild()));
    connect(btnProgramBurnEEP,SIGNAL(clicked()),this,SLOT(programBurnEE()));
    connect(btnProgramDebugTerm,SIGNAL(clicked()),this,SLOT(programDebug()));
    connect(btnProgramRun,SIGNAL(clicked()),this,SLOT(programRun()));

    btnProgramStopBuild->setToolTip(tr("Stop Build or Loader"));
    btnProgramBuild->setToolTip(tr("Build Project"));
    btnProgramBurnEEP->setToolTip(tr("Burn Project to EEPROM"));
    btnProgramRun->setToolTip(tr("Run Project"));
    btnProgramDebugTerm->setToolTip(tr("Run Project with Console"));

    ctrlToolBar = addToolBar(tr("Hardware"));
    ctrlToolBar->setLayoutDirection(Qt::RightToLeft);
#ifdef BOARD_TOOLBAR
    cbBoard = new QComboBox(this);
    cbBoard->setLayoutDirection(Qt::LeftToRight);
    cbBoard->setToolTip(tr("Board Type Select"));
    cbBoard->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(cbBoard,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentBoard(int)));
#endif
    cbPort = new QPortComboBox(this);
    cbPort->setEditable(true);
    cbPort->setLayoutDirection(Qt::LeftToRight);
    cbPort->setToolTip(tr("Serial Port Select"));
    cbPort->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(cbPort,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentPort(int)));
    connect(cbPort,SIGNAL(clicked()),this,SLOT(enumeratePorts()));

    btnConnected = new QToolButton(this);
    btnConnected->setToolTip(tr("Serial Port Console"));
    btnConnected->setCheckable(true);
    connect(btnConnected,SIGNAL(clicked()),this,SLOT(connectButton()));

    QToolButton *reset = new QToolButton(this);
    reset->setToolTip(tr("Reset Port"));
    connect(reset,SIGNAL(clicked()),this,SLOT(portResetButton()));
#ifdef BUTTON_PORT_SCAN
    QToolButton *btnPortScan = new QToolButton(this);
    btnPortScan->setToolTip(tr("Rescan Serial Ports"));
    connect(btnPortScan,SIGNAL(clicked()),this,SLOT(enumeratePorts()));
#endif
#ifdef BOARD_TOOLBAR
    QToolButton *btnLoadBoards = new QToolButton(this);
    btnLoadBoards->setToolTip(tr("Reload Board List"));
    connect(btnLoadBoards,SIGNAL(clicked()),this,SLOT(initBoardTypes()));
#endif
    addToolButton(ctrlToolBar, btnConnected, QString(":/images/console.png"));
    addToolButton(ctrlToolBar, reset, QString(":/images/reset.png"));
#ifdef BUTTON_PORT_SCAN
    addToolButton(ctrlToolBar, btnPortScan, QString(":/images/refresh.png"));
#endif
    ctrlToolBar->addWidget(cbPort);
#ifdef BOARD_TOOLBAR
    addToolButton(ctrlToolBar, btnLoadBoards, QString(":/images/hardware.png"));
    ctrlToolBar->addWidget(cbBoard);
#endif
    ctrlToolBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    showSimpleView(simpleViewType);

}

QPlainTextEdit* MainSpinWindow::getDebugEditor()
{
    return this->debugStatus;
}

void MainSpinWindow::toggleSimpleView()
{
     simpleViewType = !simpleViewType;
     QList<QAction*> list = toolsMenu->actions();
     QAction *action = list.at(0);
     action->setText(this->simpleViewType ? tr(ProjectView) : tr(SimpleView));
     showSimpleView(simpleViewType);
}

/*
 * show simple/project view
 */
void MainSpinWindow::showSimpleView(bool simple)
{
    /* in simple view, we don't show:
     * selected items in fileMenu and projMenu
     * project manager
     * status tabs
     * file toolbar
     * btn Project Set App
     */
    QList <QAction*> fileMenuList = fileMenu->actions();
    QList <QAction*> projMenuList = projMenu->actions();

    if(simple)
    {
        leftSplit->hide();
        statusTabs->hide();
        fileToolBar->hide();
        propToolBar->hide();
        btnShowProjectPane->show();
        btnShowStatusPane->show();
        btnShowProjectPane->setChecked(false);
        btnShowStatusPane->setChecked(false);
        showProjectPane(false);
        showStatusPane(false);
        foreach(QAction *fa, fileMenuList) {
            QString txt = fa->text();
            if(txt != NULL) {
                if(txt.contains(NewFile) ||
                   txt.contains(OpenFile) ||
                   txt.contains(SaveFile) ||
                   txt.contains(SaveAsFile))
                    fa->setVisible(false);
            }
        }
        foreach(QAction *pa, projMenuList) {
            QString txt = pa->text();
            if(txt != NULL) {
                if(txt.contains(CloneProject) || txt.contains(SaveAsProject))
                    pa->setVisible(false);
            }
        }
    }
    else
    {
        leftSplit->show();
        statusTabs->show();
        fileToolBar->show();
        propToolBar->show();
        btnShowProjectPane->hide();
        btnShowStatusPane->hide();
        btnShowProjectPane->setChecked(true);
        btnShowStatusPane->setChecked(true);
        showProjectPane(true);
        showStatusPane(true);
        foreach(QAction *fa, fileMenuList) {
            QString txt = fa->text();
            if(txt != NULL) {
                if(txt.contains(NewFile) ||
                   txt.contains(OpenFile) ||
                   txt.contains(SaveFile) ||
                   txt.contains(SaveAsFile))
                    fa->setVisible(true);
            }
        }
        foreach(QAction *pa, projMenuList) {
            QString txt = pa->text();
            if(txt != NULL) {
                if(txt.contains(CloneProject) || txt.contains(SaveAsProject))
                    pa->setVisible(true);
            }
        }
    }
    QVariant viewv = simple;
    settings->setValue(simpleViewKey, viewv);
}

void MainSpinWindow::showProjectPane(bool show)
{
    leftSplit->setVisible(show);
    btnShowProjectPane->setText(show ? "<" : ">");
}

void MainSpinWindow::showStatusPane(bool show)
{
    statusTabs->setVisible(show);
    btnShowStatusPane->setText(show ? "v" : "^");
}
