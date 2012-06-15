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

#include "mainwindow.h"
#include "qextserialenumerator.h"
#include "Sleeper.h"

#define SD_TOOLS
#define APPWINDOW_MIN_HEIGHT 480
#define APPWINDOW_MIN_WIDTH 780
#define EDITOR_MIN_WIDTH 500
#define PROJECT_WIDTH 270

#define SOURCE_FILE_TYPES "Source Files (*.c *.ccp *.h *.cogc *.ecogc *.spin *.espin);; All (*)"
#define PROJECT_FILE_FILTER "SIDE Project (*.side);; All (*)"

#define GDB_TABNAME "GDB Output"

#define SHOW_ASM_EXTENTION ".asm"
#define SHOW_ASMC_EXTENTION ".asmc"
#define SHOW_MAP_EXTENTION ".map"
#define SIDE_EXTENSION ".side"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
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

    /* setup loader and port listener */
    /* setup the terminal dialog box */
    term = new Terminal(this);
    termEditor = term->getEditor();

    /* tell port listener to use terminal editor for i/o */
    portListener = new PortListener(this, termEditor);
    portListener->setTerminalWindow(termEditor);

    term->setPortListener(portListener);

    term->setWindowTitle(QString(ASideGuiKey)+" Simple Terminal");
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

    /* load the last file into the editor to make user happy */
    QVariant lastfilev = settings->value(lastFileNameKey);
    if(!lastfilev.isNull()) {
        if(lastfilev.canConvert(QVariant::String)) {
            QString fileName = lastfilev.toString();
            if(fileName.length() > 0 && QFile::exists(fileName)) {
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

    /* show help dialog */
    QVariant helpStartup = settings->value(helpStartupKey,true);
    if(helpStartup.canConvert(QVariant::Bool)) {
        if(helpStartup == true)
            aboutShow();
    }

}

void MainWindow::keyHandler(QKeyEvent* event)
{
    //qDebug() << "MainWindow::keyHandler";
    int key = event->key();
    switch(key)
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        key = '\n';
        break;
    case Qt::Key_Backspace:
        key = '\b';
        break;
    default:
        if(key & Qt::Key_Escape)
            return;
        if(event->text().length() > 0) {
            QChar c = event->text().at(0);
            key = (int)c.toAscii();
        }
        break;
    }
    QByteArray barry;
    barry.append((char)key);
    portListener->send(barry);
}

void MainWindow::sendPortMessage(QString s)
{
    QByteArray barry;
    foreach(QChar c, s) {
        barry.append(c);
        portListener->send(barry);
        barry.clear();
        this->thread()->yieldCurrentThread();
    }
}

void MainWindow::terminalEditorTextChanged()
{
    //QString text = termEditor->toPlainText();
}

/*
 * get the application settings from the registry for compile/startup
 */
void MainWindow::getApplicationSettings()
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

void MainWindow::exitSave()
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(event->type()) {}; // silence compiler
    quitProgram();
}

void MainWindow::quitProgram()
{
    /* never leave port open */
    portListener->close();
    term->accept(); // just in case serial terminal is open

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

void MainWindow::newFile()
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

void MainWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull()) {
//        fileName = fileDialog.getOpenFileName(this, tr("Open File"), lastPath, tr(SOURCE_FILE_TYPES));
//#endif
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), lastPath, SOURCE_FILE_TYPES); //"All (*)");
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
    }
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
        updateProjectTree(fileName);
    }
    else {
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

void MainWindow::openFileName(QString fileName)
{
    QString data;
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly))
        {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            data = in.readAll();
            file.close();
            data.replace('\t',"    ");
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
void MainWindow::closeFile()
{
    int tab = editorTabs->currentIndex();
    if(tab > -1)
        closeTab(tab);
}

/*
 * close project if open and close all tabs.
 * do exitSave function and close all windows.
 */
void MainWindow::closeAll()
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
void MainWindow::newProject()
{
    newProjDialog->showDialog();
}

void MainWindow::newProjectAccepted()
{
    QString name = newProjDialog->getName();
    QString path = newProjDialog->getPath();
    QDir dir(path);

    QString maintemplate("/**\n" \
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
    if(dir.exists(path) == 0)
        dir.mkdir(path);
    QString mainName(path+"/"+name+".c");
    QFile mainfile(mainName);
    if(mainfile.exists() == false) {
        if(mainfile.open(QFile::ReadWrite)) {
            mainfile.write(maintemplate.toAscii());
            mainfile.close();
        }
    }
    projectFile = path+"/"+name+SIDE_EXTENSION;
    setCurrentProject(projectFile);
    updateProjectTree(mainName);
    openFile(projectFile);
}

void MainWindow::openProject(const QString &path)
{
    QString fileName = path;

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
QString MainWindow::pathDiff(QString s2, QString s1)
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
QString MainWindow::saveAsProjectLinkFix(QString srcPath, QString dstPath, QString link)
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
void MainWindow::saveAsProject(const QString &inputProjFile)
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
void MainWindow::cloneProject()
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
void MainWindow::closeProject()
{
    /* ask to save options
     */
    int rc = QMessageBox::No;

    if(projectFile.length() > 0)
        rc = QMessageBox::question(this,tr("Save Project?"),tr("Save project manager settings before close?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

    /* abort?
     */
    if(rc == QMessageBox::Cancel)
        return;

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


void MainWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        closeProject();
        openFile(action->data().toString());
    }
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    QStringList files = settings->value(recentFilesKey).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings->setValue(recentFilesKey, files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()
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

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}


void MainWindow::setCurrentProject(const QString &fileName)
{
    projectFile = fileName;

    QStringList files = settings->value(recentProjectsKey).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentProjects)
        files.removeLast();

    settings->setValue(recentProjectsKey, files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentProjectActions();
    }
}

void MainWindow::updateRecentProjectActions()
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

void MainWindow::saveFile()
{
    try {
        int n = this->editorTabs->currentIndex();
        QString fileName = editorTabs->tabToolTip(n);
        QString data = editors->at(n)->toPlainText();
        if(fileName.isEmpty())
            fileName = fileDialog.getSaveFileName(this, tr("Save As File"), lastPath, tr(SOURCE_FILE_TYPES));
        if (fileName.isEmpty())
            return;
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
        editorTabs->setTabText(n,shortFileName(fileName));
        editorTabs->setTabToolTip(n,fileName);
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QFile::WriteOnly)) {
                file.write(data.toUtf8());
                file.close();
            }
        }
        saveProjectOptions();
    } catch(...) {
    }
}

void MainWindow::saveFileByTabIndex(int tab)
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

void MainWindow::saveAsFile(const QString &path)
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
            setCurrentFile(fileName);
        }
    } catch(...) {
    }
}

void MainWindow::savePexFile()
{

}

void MainWindow::downloadSdCard()
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
    removeArg(args, "a.out");

    //QString s = QDir::toNativeSeparators(fileName);
    args.append("-f");
    args.append(this->shortFileName(fileName));

    btnConnected->setChecked(false);
    portListener->close(); // disconnect uart before use

    showBuildStart(aSideLoader,args);


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

/*
 * make star go away if no changes.
 */
void MainWindow::fileChanged()
{
    if(fileChangeDisable)
        return;

    int index = editorTabs->currentIndex();
    QString name = editorTabs->tabText(index);
    Editor *ed = editors->at(index);

    /* tab controls have been moved to the editor class */

    QString curtext = ed->toPlainText();
    QString fileName = editorTabs->tabToolTip(index);
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
    QTextStream in(&file);
    in.setCodec("UTF-8");
    if(file.open(QFile::ReadOnly))
    {
        text = file.readAll();
        file.close();
        ret = text.compare(curtext);
        if(ret == 0) {
            if(name.at(name.length()-1) == '*')
                editorTabs->setTabText(index, this->shortFileName(fileName));
            return;
        }
    }
    if(name.at(name.length()-1) != '*') {
        name += tr(" *");
        editorTabs->setTabText(index, name);
    }
}

void MainWindow::printFile()
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

void MainWindow::copyFromFile()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->copy();
}
void MainWindow::cutFromFile()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->cut();
}
void MainWindow::pasteToFile()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->paste();
}
void MainWindow::editCommand()
{

}
void MainWindow::systemCommand()
{

}

void MainWindow::fontDialog()
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

void MainWindow::fontBigger()
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

void MainWindow::fontSmaller()
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

void MainWindow::replaceInFile()
{
    if(!replaceDialog)
        return;

    Editor *editor = editors->at(editorTabs->currentIndex());

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
void MainWindow::findSymbolHelp(QString text)
{
    helpDialog->show(aSideDocPath, text);
}

void MainWindow::findDeclaration(QPoint point)
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->cursorForPosition(point);
    findDeclaration(cur);
}

void MainWindow::findDeclaration()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->textCursor();
    findDeclaration(cur);
}

void MainWindow::findDeclaration(QTextCursor cur)
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
            ctags->runCtags(projectFile);
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

bool MainWindow::isTagged(QString text)
{
    bool rc = false;

    if(text.length() == 0) {
        findDeclarationInfo();
        return rc;
    }
    if(text.length() > 0 && text.at(0).isLetter()) {
        ctags->runCtags(projectFile);
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

void MainWindow::findDeclarationInfo()
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

void MainWindow::prevDeclaration()
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

int MainWindow::showDeclaration(QString tagline)
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

void MainWindow::redoChange()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->redo();
}

void MainWindow::undoChange()
{
    Editor *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->undo();
}

void MainWindow::setProject()
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
        updateProjectTree(fileName);
        setCurrentProject(projectFile);
    }
}

void MainWindow::hardware()
{
    //hardwareDialog->loadBoards();
    //hardwareDialog->show();
    initBoardTypes();
}

void MainWindow::properties()
{
    propDialog->showProperties();
}

void MainWindow::propertiesAccepted()
{
    getApplicationSettings();
    initBoardTypes();
    for(int n = 0; n < editors->count(); n++) {
        Editor *e = editors->at(n);
        e->setTabStopWidth(propDialog->getTabSpaces()*10);
        e->setHighlights();
    }
}

void MainWindow::programBuild()
{
    runBuild("");
}

void MainWindow::programBurnEE()
{
    if(runBuild(""))
        return;
    runLoader("-e -r");
}

void MainWindow::programRun()
{
    // don't allow run if button is disabled
    if(btnProgramRun->isEnabled() == false)
        return;

    if(runBuild(""))
        return;
#ifdef Q_WS_WIN32
    portListener->close();
    btnConnected->setChecked(false);
#endif
    runLoader("-r");
}

void MainWindow::programDebug()
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

void MainWindow::debugCompileLoad()
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

void MainWindow::gdbShowLine()
{
    QString fileName = gdb->getResponseFile();
    int number = gdb->getResponseLine();
    qDebug() << "gdbShowLine" << fileName << number;

    openFileName(sourcePath(projectFile)+fileName);
    Editor *ed = editors->at(editorTabs->currentIndex());
    if(ed) ed->setLineNumber(number);
}

void MainWindow::gdbKill()
{
    gdb->kill();
}

void MainWindow::gdbBacktrace()
{
    gdb->backtrace();
}

void MainWindow::gdbContinue()
{
    gdb->runProgram();
}

void MainWindow::gdbNext()
{
    gdb->next();
}

void MainWindow::gdbStep()
{
    gdb->step();
}

void MainWindow::gdbFinish()
{
    gdb->finish();
}

void MainWindow::gdbUntil()
{
    gdb->until();
}

void MainWindow::gdbBreak()
{
    gdbShowLine();
}

void MainWindow::gdbInterrupt()
{
    gdb->interrupt();
}

void MainWindow::terminalClosed()
{
    portListener->close();
    btnConnected->setChecked(false);
}

void MainWindow::setupHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"), this);

    aboutLanding = "<html><body><br/>"+tr("Visit ") +
        "<a href=\"http://www.parallax.com/propellergcc/\">"+
        ASideGuiKey+"</a>"+
        tr(" for more information.")+"&nbsp; &nbsp;"+
        "Email bug reports to <a href=\"mailto:gccbeta@parallax.com\">gccbeta@parallax.com</a>"+
        "</body></html>";

    menuBar()->addMenu(helpMenu);
    aboutDialog = new AboutDialog(aboutLanding, this);

    helpMenu->addAction(QIcon(":/images/about.png"), tr("&About"), this, SLOT(aboutShow()));
    helpMenu->addAction(QIcon(":/images/helphint.png"), tr("&Credits"), this, SLOT(creditShow()));
    helpMenu->addAction(QIcon(":/images/UserHelp.png"), tr("&Help"), this, SLOT(userguideShow()));
    helpMenu->addAction(QIcon(":/images/Library.png"), tr("&Library"), this, SLOT(libraryShow()));

    /* new Help class */
    helpDialog = new Help();
}

void MainWindow::aboutShow()
{
    aboutDialog->show();
}

void MainWindow::creditShow()
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

void MainWindow::helpShow()
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

void MainWindow::libraryShow()
{
    findSymbolHelp("");
}


void MainWindow::userguideShow()
{
    QString path = QApplication::applicationDirPath();
    QString address = "file:///"+path+"/../userguide/index.html";
    bool rc = QDesktopServices::openUrl(QUrl(address));
    if(rc != true) {
        // just for debug purposes
        address = "file:///"+path+"/../../propside/userguide/index.html";
        QDesktopServices::openUrl(QUrl(address));
    }
}

void MainWindow::setCurrentBoard(int index)
{
    boardName = cbBoard->itemText(index);
    cbBoard->setCurrentIndex(index);
}

void MainWindow::setCurrentPort(int index)
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

void MainWindow::checkAndSaveFiles()
{
    if(projectModel == NULL)
        return;

    saveProjectOptions();

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
     * not saved and war user.
     */
    for(int tab = editorTabs->count()-1; tab > -1; tab--)
    {
        QString tabName = editorTabs->tabText(tab);
        if(tabName.at(tabName.length()-1) == '*')
        {
            QMessageBox::information(this,
                tr("Not a Project File"),
                tr("The file \"")+tabName+tr("\" is not part of the current project.\n") +
                tr("Please save and add the file to the project to build it."),
                QMessageBox::Ok);
        }
    }

}

int  MainWindow::checkCompilerInfo()
{
    QMessageBox mbox(QMessageBox::Critical,tr("Build Error"),"",QMessageBox::Ok);
    if(aSideCompiler.length() == 0) {
        mbox.setInformativeText(tr("Please specify compiler application in properties."));
        mbox.exec();
        return -1;
    }
    if(aSideIncludes.length() == 0) {
        mbox.setInformativeText(tr("Please specify loader folder in properties."));
        mbox.exec();
        return -1;
    }
    return 0;
}

QString MainWindow::sourcePath(QString srcpath)
{
    srcpath = QDir::fromNativeSeparators(srcpath);
    srcpath = srcpath.mid(0,srcpath.lastIndexOf(aSideSeparator)+1);
    return srcpath;
}

int  MainWindow::runBuild(QString option)
{
    int rc = 0;

#if defined(GDBENABLE)
    /* stop debugger */
    gdb->stop();
#endif

    QStringList clist;
    QFile file(projectFile);
    QString proj = "";
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        proj = file.readAll();
        file.close();
    }

    proj = proj.trimmed(); // kill extra white space
    QStringList list = proj.split("\n");

    /* add option to build */
    if(option.length() > 0)
        clist.append(option);

    /* Calculate the number of compile steps for progress.
     * Skip empty lines and don't count ">" parameters.
     */
    int maxprogress = list.length();

    /* If we don't have a list we can't compile!
     */
    if(maxprogress < 1)
        return -1;

    checkAndSaveFiles();

    progress->show();
    programSize->setText("");

    compileStatus->setPlainText(tr("Project Directory: ")+sourcePath(projectFile)+"\r\n");
    compileStatus->moveCursor(QTextCursor::End);
    status->setText(tr("Building ..."));

    foreach(QString item, list) {
        if(item.length() == 0) {
            maxprogress--;
            continue;
        }

        if(item.at(0) == '>')
            maxprogress--;
    }
    maxprogress++;

    /* choose some global options
    if(this->projectOptions->getEECOG().length() > 0)
        eecog = true;
    */

    /* remove a.out before build
     */
    QFile aout(sourcePath(projectFile)+"a.out");
    if(aout.exists()) {
        if(aout.remove() == false) {
            rc = QMessageBox::question(this,
                tr("Can't Remove File"),
                tr("Can't Remove output file before build.\n"\
                   "Please close any program using the file \"a.out\".\n"\
                   "Continue?"),
                QMessageBox::No, QMessageBox::Yes);
            if(rc == QMessageBox::No)
                return -1;
        }
    }

    /* remove projectFile.pex before build
     */
    QString pexFile = projectFile;
    pexFile = pexFile.mid(0,pexFile.lastIndexOf("."))+".pex";
    QFile pex(pexFile);
    if(pex.exists()) {
        if(pex.remove() == false) {
            int rc = QMessageBox::question(this,
                tr("Can't Remove File"),
                tr("Can't Remove output file before build.\n"\
                   "Please close any program using the file \"")+pexFile+"\".\n" \
                   "Continue?",
                QMessageBox::No, QMessageBox::Yes);
            if(rc == QMessageBox::No)
                return -1;
        }
    }

    /* Run through file list and compile according to extension.
     * Add main file after going through the list. i.e start at list[1]
     */
    for(int n = 1; rc == 0 && n < list.length(); n++) {
        progress->setValue(100*n/maxprogress);
        QString name = list[n];
        if(name.length() == 0)
            continue;
        if(name.at(0) == '>')
            continue;
        QString base = shortFileName(name.mid(0,name.lastIndexOf(".")));
        if(name.contains(FILELINK)) {
            name = name.mid(name.indexOf(FILELINK)+QString(FILELINK).length());
            base = name.mid(0,name.lastIndexOf("."));
        }

        QString suffix = name.mid(name.lastIndexOf("."));
        suffix = suffix.toLower();

        if(suffix.compare(".spin") == 0) {
            if(runBstc(name))
                rc = -1;
            if(proj.toLower().lastIndexOf(".dat") < 0) // intermediate
                list.append(name.mid(0,name.lastIndexOf(".spin"))+".dat");
        }
        else if(suffix.compare(".espin") == 0) {
            QString basepath = sourcePath(projectFile);
            this->compileStatus->appendPlainText("Copying "+name+" to tmp.spin for spin compiler.");
            if(QFile::exists(basepath+"tmp.spin"))
                QFile::remove(basepath+"tmp.spin");
            if(QFile::copy(basepath+base+".spin",basepath+"tmp.spin") != true) {
                rc = -1;
                continue;
            }
            if(runBstc("tmp.spin")) {
                rc = -1;
                continue;
            }
            if(QFile::exists(basepath+base+".edat"))
                QFile::remove(basepath+base+".edat");
            if(QFile::copy(basepath+"tmp.dat",basepath+base+".edat") != true) {
                rc = -1;
                continue;
            }
            if(proj.toLower().lastIndexOf(".edat") < 0) // intermediate
                list.append(name.mid(0,name.lastIndexOf(".espin"))+".edat");
        }
        else if(suffix.compare(".dat") == 0) {
            name = shortFileName(name);
            if(runObjCopy(name))
                rc = -1;
            if(proj.toLower().lastIndexOf("_firmware.o") < 0)
                clist.append(name.mid(0,name.lastIndexOf(".dat"))+"_firmware.o");
        }

        else if(suffix.compare(".edat") == 0) {
            name = shortFileName(name);
            if(runObjCopy(name))
                rc = -1;
            if(runCogObjCopy(base+"_firmware.ecog",base+"_firmware.o"))
                rc = -1;
            if(proj.toLower().lastIndexOf("_firmware.o") < 0)
                clist.append(base+"_firmware.o");
        }
        else if(suffix.compare(".s") == 0) {
            if(runGAS(name))
                rc = -1;
            if(proj.toLower().lastIndexOf(".o") < 0)
                clist.append(name.mid(0,name.lastIndexOf(".s"))+".o");
        }
        /* .cogc also does COG specific objcopy */
        else if(suffix.compare(".cogc") == 0) {
            if(runCOGC(name,".cog"))
                rc = -1;
            clist.append(shortFileName(base)+".cog");
        }
        /* .cogc also does COG specific objcopy */
        else if(suffix.compare(".ecogc") == 0) {
            if(runCOGC(name,".ecog"))
                rc = -1;
            clist.append(shortFileName(base)+".ecog");
        }
        /* dont add .a yet */
        else if(suffix.compare(".a") == 0) {
        }
        /* add all others */
        else {
            clist.append(name);
        }

    }

    /* add main file */
    clist.append(list[0]);

    /* add library .a files to the end of the list
     */
    for(int n = 0; n < list.length(); n++) {
        progress->setValue(100*n/maxprogress);
        QString name = list[n];
        if(name.length() == 0)
            continue;
        if(name.at(0) == '>')
            continue;
        if(name.contains(FILELINK))
            name = name.mid(name.indexOf(FILELINK)+QString(FILELINK).length());

        /* add .a */
        if(name.toLower().lastIndexOf(".a") > 0) {
            clist.append(name);
        }
    }

    QString loadtype;
    QTextCursor cur;
    bool runpex = false;

    if(rc != 0) {
        QVariant vname = process->property("Name");
        QString name = "Build";
        if(vname.canConvert(QVariant::String)) {
            name = vname.toString();
        }
        buildResult(rc, rc, name, name);
    }
    else {
        rc = runCompiler(clist);

        cur = compileStatus->textCursor();

        loadtype = cbBoard->currentText();
        if(loadtype.contains(ASideConfig::UserDelimiter+ASideConfig::SdRun, Qt::CaseInsensitive)) {
            runpex = true;
        }
        else
        if(loadtype.contains(ASideConfig::UserDelimiter+ASideConfig::SdLoad, Qt::CaseInsensitive)) {
            runpex = true;
        }
        if(runpex) {
            rc = runPexMake("a.out");
            if(rc != 0)
                compileStatus->appendPlainText("Could not make AUTORUN.PEX\n");
        }

        if(rc == 0) {
            compileStatus->appendPlainText("Done. Build Succeeded!\n");
            cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
            compileStatus->setTextCursor(cur);
        }
        else {
            compileStatus->appendPlainText("Done. Build Failed!\n");
            if(compileStatus->toPlainText().indexOf("error:",Qt::CaseInsensitive) > 0) {
                compileStatus->appendPlainText("Click error or warning messages above to debug.\n");
            }
            if(compileStatus->toPlainText().indexOf("undefined reference",Qt::CaseInsensitive) > 0) {
                QStringList ssplit = compileStatus->toPlainText().split("undefined reference to ", QString::SkipEmptyParts, Qt::CaseInsensitive);
                if(ssplit.count() > 1) {
                    QString msg = ssplit.at(1);
                    QStringList msplit = msg.split("collect");
                    if(msplit.count() > 0) {
                        QString mstr = msplit.at(0);
                        if(mstr.indexOf("`__") == 0) {
                            mstr = mstr.mid(2);
                            mstr = mstr.trimmed();
                            mstr = mstr.mid(0,mstr.length()-1);
                        }
                        compileStatus->appendPlainText("Check source for bad function call or global variable name "+mstr+"\n");
                    }
                    else {
                        compileStatus->appendPlainText("Check source for bad function call or global variable name "+ssplit.at(1)+"\n");
                    }
                    cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
                    compileStatus->setTextCursor(cur);
                    return rc;
                }
            }
            if(compileStatus->toPlainText().indexOf("overflowed by", Qt::CaseInsensitive) > 0) {
                compileStatus->appendPlainText("Your program is too big for the memory model selected in the project.");
                cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
                compileStatus->setTextCursor(cur);
                return rc;
            }
            if(compileStatus->toPlainText().indexOf("Error: Relocation overflows", Qt::CaseInsensitive) > 0) {
                compileStatus->appendPlainText("Your program is too big for the memory model selected in the project.");
                cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
                compileStatus->setTextCursor(cur);
                return rc;
            }
        }
    }

    Sleeper::ms(250);
    progress->hide();

    cur = compileStatus->textCursor();
    cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    compileStatus->setTextCursor(cur);

    return rc;
}

int  MainWindow::runCOGC(QString name, QString outext)
{
    int rc = 0; // return code

    QString compstr = shortFileName(aSideCompiler);
    QString objcopy = "propeller-elf-objcopy";

    QString base = shortFileName(name.mid(0,name.lastIndexOf(".")));
    // copy .cog to .c
    // QFile::copy(sourcePath(projectFile)+name,sourcePath(projectFile)+base+".c");
    // run C compiler on new file
    QStringList args;
    args.append("-r");  // relocatable ?
    args.append("-Os"); // default optimization for -mcog
    args.append("-mcog"); // compile for cog
    args.append("-o"); // create a .cog object
    args.append(base+outext);
    args.append("-xc"); // code to compile is C code
    //args.append("-c");
    args.append(name);

    /* run compiler */
    rc = startProgram(compstr, sourcePath(projectFile),args);
    if(rc) return rc;

    /* now do objcopy */
    args.clear();
    args.append("--localize-text");
    args.append("--rename-section");
    args.append(".text="+base+outext);
    args.append(base+outext);

    /* run object copy to localize fix up .cog object */
    rc = startProgram(objcopy, sourcePath(projectFile), args);

    return rc;
}

int  MainWindow::runBstc(QString spinfile)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args;
    args.append("-c");
    args.append(spinfile); // using shortname limits us to files in the project directory.

    /* run the bstc program */
#if defined(Q_WS_WIN32)
    QString bstc = "bstc";
#elif defined(Q_WS_MAC)
    QString bstc = aSideCompilerPath+"bstc.osx";
#else
    QString bstc = aSideCompilerPath+"bstc.linux";
#endif
    rc = startProgram(bstc, sourcePath(projectFile), args);

    return rc;
}


int  MainWindow::runCogObjCopy(QString datfile, QString tarfile)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args;

    //args.append("--localize-text");
    args.append("--rename-section");
    args.append(".data="+datfile);
    args.append(tarfile);

    /* run objcopy */
    QString objcopy = "propeller-elf-objcopy";
    rc = startProgram(objcopy, sourcePath(projectFile), args);

    return rc;
}

int  MainWindow::runObjCopyRedefineSym(QString oldsym, QString newsym, QString file)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args;

    args.append("--redefine-sym");
    args.append(oldsym+"="+newsym);
    args.append(file);

    /* run objcopy */
    QString objcopy = "propeller-elf-objcopy";
    rc = startProgram(objcopy, sourcePath(projectFile), args);

    return rc;
}

int  MainWindow::runObjCopy(QString datfile)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QString objfile = datfile.mid(0,datfile.lastIndexOf("."))+"_firmware.o";
    QStringList args;
    args.append("-I");
    args.append("binary");
    args.append("-B");
    args.append("propeller");
    args.append("-O");
    args.append("propeller-elf-gcc");
    args.append(datfile);
    args.append(objfile);

    /* run objcopy to make a spin .dat file into an object file */
    QString objcopy = "propeller-elf-objcopy";
    rc = startProgram(objcopy, sourcePath(projectFile), args);

    return rc;
}

int  MainWindow::runGAS(QString gasfile)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QString objfile = gasfile.mid(0,gasfile.lastIndexOf("."))+".o";
    QStringList args;
    args.append("-o");
    args.append(objfile);
    args.append(gasfile);

    /* run the as program */
    QString gas = "propeller-elf-as";
    rc = startProgram(gas, sourcePath(projectFile), args);

    return rc;
}

int  MainWindow::runPexMake(QString fileName)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QString pexFile = projectFile;
    pexFile = sourcePath(projectFile) + "a.pex";
    QString autoexec = sourcePath(projectFile) + "AUTORUN.PEX";
    QFile pex(pexFile);
    if(pex.exists()) {
        pex.remove();
    }
    QFile apex(autoexec);
    if(apex.exists()) {
        apex.remove();
    }

    QStringList args;
    args.append("-x");
    args.append(fileName);

    /* run the as program */
    QString prog = "propeller-load";
    rc = startProgram(prog, sourcePath(projectFile), args);

    if(rc == 0) {
        QFile npex(pexFile);
        if(npex.exists()) {
            npex.copy(autoexec);
            npex.remove();
        }
    }
    return rc;
}

QStringList MainWindow::getCompilerParameters(QStringList copts)
{
    QStringList args;
    getCompilerParameters(copts, &args);
    return args;
}

int MainWindow::getCompilerParameters(QStringList copts, QStringList *args)
{
    // use the projectFile instead of the current tab file
    //QString srcpath = sourcePath(projectFile);

    portName = cbPort->itemText(cbPort->currentIndex());
    boardName = cbBoard->itemText(cbBoard->currentIndex());

    QString model = projectOptions->getMemModel();

    if(copts.length() > 0) {
        QString s = copts.at(0);
        if(s.compare("-g") == 0)
            args->append(s);
    }
    args->append("-o");
    args->append("a.out");
    args->append(projectOptions->getOptimization());
    args->append("-m"+model);

    args->append("-I");
    args->append("."); // just in case for a project configuration header

    if(projectOptions->getWarnAll().length())
        args->append(projectOptions->getWarnAll());
    if(projectOptions->get32bitDoubles().length())
        args->append(projectOptions->get32bitDoubles());
    if(projectOptions->getExceptions().length())
        args->append(projectOptions->getExceptions());
    if(projectOptions->getNoFcache().length())
        args->append(projectOptions->getNoFcache());

    if(projectOptions->getSimplePrintf().length()) {
        /* don't use simple printf flag for COG model programs. */
        if(model.contains("cog",Qt::CaseInsensitive) == false)
            args->append(projectOptions->getSimplePrintf());
        else {
            this->compileStatus->insertPlainText(tr("Ignoring")+" \"Simple printf\""+tr(" flag in COG mode program.")+"\n");
            this->compileStatus->moveCursor(QTextCursor::End);
        }
    }

    if(projectOptions->getCompiler().indexOf("++") > -1)
        args->append("-fno-rtti");

    /* other compiler options */
    if(projectOptions->getCompOptions().length()) {
        QStringList complist = projectOptions->getCompOptions().split(" ",QString::SkipEmptyParts);
        foreach(QString compopt, complist) {
            args->append(compopt);
        }
    }

    /* files */
    for(int n = 0; n < copts.length(); n++) {
        QString parm = copts[n];
        if(parm.length() == 0)
            continue;
        if(parm.indexOf(" ") > 0 && parm[0] == '-') {
            // handle stuff like -I path
            QStringList sp = parm.split(" ");
            args->append(sp.at(0));
            QString join = "";
            int m;
            for(m = 1; m < sp.length()-1; m++)
                join += sp.at(m) + " ";
            join += sp.at(m);
            args->append(join);
        }
        else if (parm.indexOf(".cfg",Qt::CaseInsensitive) > -1){
            // don't append .cfg parameter
        }
        else {
            args->append(parm);
        }
    }

    /* libraries */
    if(projectOptions->getMathLib().length())
        args->append(projectOptions->getMathLib());
    if(projectOptions->getPthreadLib().length())
        args->append(projectOptions->getPthreadLib());

    /* other linker options */
    if(projectOptions->getLinkOptions().length()) {
        QStringList linklist = projectOptions->getLinkOptions().split(" ",QString::SkipEmptyParts);
        foreach(QString linkopt, linklist) {
            args->append(linkopt);
        }
    }

    /* strip */
    if(projectOptions->getStripElf().length())
        args->append(projectOptions->getStripElf());

    return args->length();
}

int  MainWindow::runCompiler(QStringList copts)
{
    int rc = 0;

    if(projectModel == NULL || projectFile.isNull()) {
        QMessageBox mbox(QMessageBox::Critical, "Error No Project",
            "Please select a tab and press F4 to set main project file.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args = getCompilerParameters(copts);
    QString compstr;

#if defined(Q_WS_WIN32)
    compstr = shortFileName(aSideCompiler);
#else
    compstr = aSideCompiler;
#endif

    if(projectOptions->getCompiler().indexOf("++") > -1) {
        compstr = compstr.mid(0,compstr.lastIndexOf("-")+1);
        compstr+="c++";
    }

    /* this is the final compile/link */
    rc = startProgram(compstr,sourcePath(projectFile),args);
    if(rc != 0)
        return rc;

    args.clear();
    args.append("-h");
    args.append("a.out");
    rc = startProgram("propeller-elf-objdump",sourcePath(projectFile),args,this->DumpReadSizes);

    /*
     * Report program size
     * Use the projectFile instead of the current tab file
     */
    if(codeSize == 0) codeSize = memorySize;
    QString ssize = QString("Code Size %L1 bytes (%L2 total)").arg(codeSize).arg(memorySize);
    programSize->setText(ssize);

    return rc;
}

QStringList MainWindow::getLoaderParameters(QString copts)
{
    // use the projectFile instead of the current tab file
    // QString srcpath = sourcePath(projectFile);

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
    // always include user's propeller-load path.
    args.append("-I");
    args.append(aSideIncludes);

    if(this->propDialog->getLoadDelay() > 0) {
        args.append(QString("-S%1").arg(this->propDialog->getLoadDelay()));
    }
    QString bname = this->cbBoard->currentText();
    ASideBoard* board = aSideConfig->getBoardData(bname);
    QString reset = board->get(ASideBoard::reset);

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
    args.append("-b");
    args.append(boardName);
    args.append("-p");
    args.append(portName);
#if 0
    args.append("-I");
    args.append(aSideIncludes);
#endif

    /* if propeller-load parameters -l or -z in copts, don't append a.out */
    if((copts.indexOf("-l") > 0 || copts.indexOf("-z") > 0) == false)
        args.append("a.out");

    QStringList olist = copts.split(" ",QString::SkipEmptyParts);
    for (int n = 0; n < olist.length(); n++)
        args.append(olist[n]);

    //qDebug() << args;
    return args;
}

int  MainWindow::runLoader(QString copts)
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

    showBuildStart(aSideLoader,args);

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

int  MainWindow::startProgram(QString program, QString workpath, QStringList args, DumpType dump)
{
    /*
     * this is the asynchronous method.
     */
    showBuildStart(program,args);

#if !defined(Q_WS_WIN32)
    if(program.contains(aSideCompilerPath) == false)
        program = aSideCompilerPath + program;
#endif

    process->setProperty("Name", QVariant(program));
    process->setProperty("IsLoader", QVariant(false));

    if(dump == this->DumpReadSizes) {
        disconnect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
        connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyReadSizes()));
    }
    else {
        connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    }
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(workpath);

    procDone = false;
    procResultError = false;
    process->start(program,args);

    /* process Qt application events until procDone
     */
    while(procDone == false)
        QApplication::processEvents();

    disconnect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyReadSizes()));

    progress->hide();

    if(procResultError)
        return 1;
    return process->exitCode();
}

void MainWindow::procError(QProcess::ProcessError error)
{
    QVariant name = process->property("Name");
    compileStatus->appendPlainText(name.toString() + tr(" error ... (%1)").arg(error));
    compileStatus->appendPlainText(process->readAllStandardOutput());
    procMutex.lock();
    procDone = true;
    procMutex.unlock();
}

void MainWindow::procFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(procDone == true)
        return;

    procMutex.lock();
    procDone = true;
    procMutex.unlock();

    QVariant name = process->property("Name");
    buildResult(exitStatus, exitCode, name.toString(), process->readAllStandardOutput());

    int len = status->text().length();
    QString s = status->text().mid(len-8);
    if(s.contains("done.",Qt::CaseInsensitive) == false)
        status->setText(status->text()+" done.");
}

/*
 * save for cat dumps
 */
void MainWindow::procReadyReadCat()
{

}

/*
 * read program sizes from objdump -h
 */
void MainWindow::procReadyReadSizes()
{
    int rc;
    bool ok;
    QByteArray bytes = process->readAllStandardOutput();
    if(bytes.length() == 0)
        return;

    this->codeSize = 0;
    this->memorySize = 0;
    QStringList lines = QString(bytes).split("\n",QString::SkipEmptyParts);
    int len = lines.length();
    for (int n = 0; n < len; n++) {
        QString line = lines[n];
        if(line.length() > 0) {
            QString ms = line.mid(line.indexOf("."));
            QRegExp regex("[ \t]");
            QStringList more = line.split(regex,QString::SkipEmptyParts);
            if(ms.contains(".bss",Qt::CaseInsensitive)) {
                if(more.length() > 2) {
                    rc = more.at(2).toInt(&ok,16);
                    if(ok) {
                        this->memorySize += rc;
                    }
                }
            }
            else
            if(ms.contains("heap",Qt::CaseInsensitive)) {
                //this->codeSize += 4;
                //this->memorySize += 4;
                break;
            }
            else if (n < len-1){
                if(QString(lines.at(n+1)).contains("load", Qt::CaseInsensitive)) {
                    if(more.length() > 2) {
                        rc = more.at(2).toInt(&ok,16);
                        if(ok) {
                            this->codeSize += rc;
                            this->memorySize += rc;
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::procReadyRead()
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
#if 0
            // can't do this otherwise error info gets lost.
            else
            if(line.contains("error:",Qt::CaseInsensitive)) {
                status->setText(status->text()+line+" ");
            }
#endif
            else {
                compileStatus->insertPlainText(eol);
                compileStatus->insertPlainText(line);
            }
        }
    }

}

int  MainWindow::checkBuildStart(QProcess *proc, QString progName)
{
    QMessageBox mbox;
    mbox.setStandardButtons(QMessageBox::Ok);
    //qDebug() << QDir::currentPath();
    if(!proc->waitForStarted()) {
        mbox.setInformativeText(progName+tr(" Could not start."));
        mbox.exec();
        return -1;
    }
    if(!proc->waitForFinished()) {
        mbox.setInformativeText(progName+tr(" Error waiting for program to finish."));
        mbox.exec();
        return -1;
    }
    return 0;
}

void MainWindow::showBuildStart(QString progName, QStringList args)
{
    QString argstr = "";
    for(int n = 0; n < args.length(); n++)
        argstr += " "+args[n];
    //qDebug() << progName+argstr;
    compileStatus->appendPlainText(shortFileName(progName)+argstr);
}

int  MainWindow::buildResult(int exitStatus, int exitCode, QString progName, QString result)
{
    QMessageBox mbox;
    mbox.setStandardButtons(QMessageBox::Ok);
    mbox.setInformativeText(result);

    if(exitStatus == QProcess::CrashExit)
    {
        status->setText(status->text()+" "+tr("Compiler Crashed"));
        mbox.setText(tr("Compiler Crashed"));
        mbox.exec();
    }
    else if(result.toLower().indexOf("error") > -1)
    { // just in case we get an error without exitCode
        status->setText(status->text()+" "+progName+tr(" Error:")+result);
        if(progName.contains("load",Qt::CaseInsensitive))
            mbox.setText(tr("Load Error"));
        else {
            if(result.contains("port",Qt::CaseInsensitive))
                mbox.setText(tr("Serial Port Error"));
            else
                mbox.setText(tr("Build Error"));
        }
        mbox.exec();
    }
    else if(exitCode != 0)
    {
        status->setText(status->text()+" "+progName+tr(" Error: ")+QString("%1").arg(exitCode));
    }
    else if(result.toLower().indexOf("warning") > -1)
    {
        status->setText(status->text()+" "+progName+tr(" Compiled OK with Warning(s)."));
        return 0;
    }
    else
    {
        /* we can show progress of individual build steps, but that makes status unreasonable. */
        return 0;
    }
    return -1;
}

void MainWindow::compilerError(QProcess::ProcessError error)
{
    qDebug() << error;
}

void MainWindow::compilerFinished(int exitCode, QProcess::ExitStatus status)
{
    qDebug() << exitCode << status;
}


void MainWindow::closeTab(int tab)
{
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
void MainWindow::editorTabMenu(QPoint pt)
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

void MainWindow::changeTab(bool checked)
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

void MainWindow::addToolButton(QToolBar *bar, QToolButton *btn, QString imgfile)
{
    const QSize buttonSize(24, 24);
    btn->setIcon(QIcon(QPixmap(imgfile.toAscii())));
    btn->setMinimumSize(buttonSize);
    btn->setMaximumSize(buttonSize);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bar->addWidget(btn);
}

void MainWindow::setupProjectTools(QSplitter *vsplit)
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

    programSize = new QLabel();
    programSize->setMinimumWidth(PROJECT_WIDTH+6);
    status = new QLabel();

    statusBar->addPermanentWidget(progress);
    statusBar->addWidget(programSize);
    statusBar->addWidget(status);
    statusBar->setMaximumHeight(22);

    this->setMinimumHeight(APPWINDOW_MIN_HEIGHT);
}

/*
 * Find error or warning in a file
 */
void MainWindow::compileStatusClicked(void)
{
    int n = 0;
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

void MainWindow::projectTreeClicked(QModelIndex index)
{
    if(projectModel == NULL)
        return;
    projectIndex = index; // same as projectTree->currentIndex();
    if(projectTree->rightClick(false))
        projectMenu->popup(QCursor::pos());
    else
        showProjectFile();
}

/*
 * don't allow adding output files
 */
bool MainWindow::isOutputFile(QString file)
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
void MainWindow::addProjectListFile(QString fileName)
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
void MainWindow::addProjectFile()
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
void MainWindow::addProjectLink()
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


void MainWindow::addProjectLibFile()
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

void MainWindow::addProjectIncPath()
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

void MainWindow::addProjectLibPath()
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
void MainWindow::deleteProjectFile()
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

void MainWindow::showProjectFile()
{
    QString fileName;

    QVariant vs = projectModel->data(projectIndex, Qt::DisplayRole);
    if(vs.canConvert(QVariant::String))
    {
        fileName = vs.toString();

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
            if(tab.contains(this->shortFileName(fileName))) {
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
            openFileName(sourcePath(projectFile)+fileName);
        }
    }
}

/*
 * save project file with options.
 */
void MainWindow::saveProjectOptions()
{
    QString projstr = "";
    QStringList list;

    if(projectModel == NULL)
        return;

    if(projectFile.length() > 0)
        setWindowTitle(QString(ASideGuiKey)+" "+QDir::convertSeparators(projectFile));

    QFile file(projectFile);
    if(file.exists()) {
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            projstr = file.readAll();
            file.close();
        }
        list = projstr.split("\n");
        projstr = "";
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
        foreach(QString arg, list) {
            if(arg.contains(ProjectOptions::board+"::"))
                projstr += ">"+ProjectOptions::board+"::"+cbBoard->currentText()+"\n";
            else
                projstr += ">"+arg+"\n";
        }
        // save project file in english
        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }
    }
}

/*
 * update project tree and options by reading from project.side file
 */
void MainWindow::updateProjectTree(QString fileName)
{
    QString projName = this->shortFileName(fileName);
    projName = projName.mid(0,projName.lastIndexOf("."));
    projName += SIDE_EXTENSION;

    basicPath = sourcePath(fileName);
    projectFile = basicPath+projName;
    setWindowTitle(QString(ASideGuiKey)+" "+QDir::convertSeparators(projectFile));

    if(projectModel != NULL) delete projectModel;
    projectModel = new CBuildTree(projName, this);

    QFile file(projectFile);
    if(!file.exists()) {
        // project file is in english
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(this->shortFileName(fileName).toAscii());
            projectModel->addRootItem(this->shortFileName(fileName));
            file.close();
        }
    }
    else {
        QString s = "";
        // project file is in english
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
void MainWindow::showAssemblyFile()
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

void MainWindow::showMapFile()
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
int MainWindow::makeDebugFiles(QString fileName)
{
    if(fileName.length() == 0)
        return -1;

    /* save files before compiling debug info */
    checkAndSaveFiles();

    if(
       fileName.contains(".h",Qt::CaseInsensitive) ||
       fileName.contains(".spin",Qt::CaseInsensitive) ||
       fileName.contains("-I",Qt::CaseInsensitive) ||
       fileName.contains("-L",Qt::CaseInsensitive)
       ) {
        QMessageBox mbox(QMessageBox::Information, "Can't Show That",
            "Can't show assembly file info on this entry.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    if(projectModel == NULL || projectFile.isNull()) {
        QMessageBox mbox(QMessageBox::Critical, "Error. No Project",
            "Please select a tab and press F4 to set main project file.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    if(fileName.contains(FILELINK))
        fileName = fileName.mid(fileName.indexOf(FILELINK)+QString(FILELINK).length());

    getApplicationSettings();
    if(checkCompilerInfo()) {
        QMessageBox mbox(QMessageBox::Critical, "Error. No Compiler",
            "Please open propertes and set the compiler, loader path, and workspace.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    QString name = fileName.mid(0,fileName.lastIndexOf('.'));
    QStringList copts;
    if(fileName.contains(".cogc",Qt::CaseInsensitive)) {
        copts.append("-xc");
    }
    copts.append("-S");

    copts.append(fileName);

    QFile file(projectFile);
    QString proj = "";
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        proj = file.readAll();
        file.close();
    }

    proj = proj.trimmed(); // kill extra white space
    QStringList list = proj.split("\n");
    foreach(QString name, list) {
        if(name.contains("-I"))
            copts.append(name);
    }

    QStringList args = getCompilerParameters(copts);
    QString compstr;

    if(fileName.contains(".cogc",Qt::CaseInsensitive)) {
        for(int n = 0; n < args.length(); n++) {
            QString item = args.at(n);
            if(item.indexOf("-m") == 0) {
                args[n] = QString("-mcog");
                break;
            }
        }
    }

#if defined(Q_WS_WIN32)
    compstr = shortFileName(aSideCompiler);
#else
    compstr = aSideCompiler;
#endif

    if(projectOptions->getCompiler().indexOf("++") > -1) {
        compstr = compstr.mid(0,compstr.lastIndexOf("-")+1);
        compstr+="c++";
    }

    removeArg(args,"-S"); // peward++ Thanks! C & asm
    removeArg(args,"-o");
    removeArg(args,"a.out");
    args.insert(0,"-Wa,-ahdnl="+name+SHOW_ASM_EXTENTION); // peward++
    args.insert(0,"-c"); // peward++
    args.insert(0,"-g"); // peward++

    /* this is the final compile/link */
    compileStatus->setPlainText("");
    int rc = startProgram(compstr,sourcePath(projectFile),args);
    if(rc) {
        QMessageBox mbox(QMessageBox::Critical, tr("Compile Error"),
                         tr("Please check the compiler, loader path, and workspace."), QMessageBox::Ok);
        mbox.exec();
        compileStatus->appendPlainText("Compile Debug Error.");
        return -1;
    }
    compileStatus->appendPlainText("Done. Compile Debug Ok.");

    return 0;
}

void MainWindow::removeArg(QStringList &list, QString arg)
{
    int len = list.length();
    for(int n = 0; n < len; n++) {
        QString s = list.at(n);
        if(s.compare(arg) == 0) {
            list.removeAt(n);
            return;
        }
    }
}

void MainWindow::enumeratePorts()
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
        cbPort->addItem(name);
#endif
    }
}

void MainWindow::connectButton()
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

void MainWindow::portResetButton()
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

QString MainWindow::shortFileName(QString fileName)
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

void MainWindow::initBoardTypes()
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

void MainWindow::setupEditor()
{
    Editor *editor = new Editor(gdb, this);
    editor->setTabStopWidth(propDialog->getTabSpaces()*10);

    /* font is user's preference */
    editor->setFont(editorFont);
    editor->setLineWrapMode(Editor::NoWrap);
    connect(editor,SIGNAL(textChanged()),this,SLOT(fileChanged()));
    editors->append(editor);
}

void MainWindow::setEditorTab(int num, QString shortName, QString fileName, QString text)
{
    Editor *editor = editors->at(num);
    fileChangeDisable = true;
    editor->setPlainText(text);

    fileChangeDisable = false;
    editorTabs->setTabText(num,shortName);
    editorTabs->setTabToolTip(num,fileName);
    editorTabs->setCurrentIndex(num);
}

/*
 * TODO: Why don't icons show up in linux? deferred.
 * QtCreator has the same problem Windows OK, Linux not OK.
 */
void MainWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(QIcon(":/images/newfile.png"), tr("&New"), this, SLOT(newFile()), QKeySequence::New);
    fileMenu->addAction(QIcon(":/images/openfile.png"), tr("&Open"), this, SLOT(openFile()), QKeySequence::Open);
    fileMenu->addAction(QIcon(":/images/savefile.png"), tr("&Save"), this, SLOT(saveFile()), QKeySequence::Save);
    fileMenu->addAction(QIcon(":/images/saveasfile2.png"), tr("Save &As"), this, SLOT(saveAsFile()),QKeySequence::SaveAs);

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

    QMenu *projMenu = new QMenu(tr("&Project"), this);
    menuBar()->addMenu(projMenu);

    projMenu->addAction(QIcon(":/images/newproj.png"), tr("New Project"), this, SLOT(newProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_N);
    projMenu->addAction(QIcon(":/images/openproj.png"), tr("Open Project"), this, SLOT(openProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_O);
    projMenu->addAction(QIcon(":/images/saveasproj.png"), tr("Save As Project"), this, SLOT(saveAsProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_S);
    //projMenu->addAction(QIcon(":/images/cloneproj.png"), tr("Clone Project"), this, SLOT(cloneProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_C);
    projMenu->addAction(tr("Clone Project"), this, SLOT(cloneProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_C);
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

    QMenu *toolsMenu = new QMenu(tr("&Tools"), this);
    menuBar()->addMenu(toolsMenu);

#if defined(SD_TOOLS)
    //toolsMenu->addAction(QIcon(":/images/flashdrive.png"), tr("Save .PEX to Local SD Card"), this, SLOT(savePexFile()));
    toolsMenu->addAction(QIcon(":/images/download.png"), tr("Send File to Target SD Card"), this, SLOT(downloadSdCard()));
#endif

    if(ctags->enabled()) {
        toolsMenu->addSeparator();
        toolsMenu->addAction(QIcon(":/images/back.png"),tr("Go &Back"), this, SLOT(prevDeclaration()), QKeySequence::Back);
        toolsMenu->addAction(QIcon(":/images/forward.png"),tr("Browse Declaration"), this, SLOT(findDeclaration()), QKeySequence::Forward);
    }

    toolsMenu->addSeparator();
    toolsMenu->addAction(QIcon(":/images/Brush.png"), tr("Font"), this, SLOT(fontDialog()));
    toolsMenu->addAction(QIcon(":/images/resize-plus.png"), tr("Bigger Font"), this, SLOT(fontBigger()), QKeySequence::ZoomIn);
    toolsMenu->addAction(QIcon(":/images/resize-plus.png"), tr("Bigger Font"), this, SLOT(fontBigger()), QKeySequence(Qt::CTRL+Qt::Key_Equal));
    toolsMenu->addAction(QIcon(":/images/resize-minus.png"), tr("Smaller Font"), this, SLOT(fontSmaller()), QKeySequence::ZoomOut);

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


void MainWindow::setupToolBars()
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
    QToolButton *btnProjectApp = new QToolButton(this);


    addToolButton(projToolBar, btnProjectNew, QString(":/images/newproj.png"));
    addToolButton(projToolBar, btnProjectOpen, QString(":/images/openproj.png"));
    addToolButton(projToolBar, btnProjectSaveAs, QString(":/images/saveasproj.png"));
    //addToolButton(projToolBar, btnProjectClone, QString(":/images/cloneproj2.png"));
    addToolButton(projToolBar, btnProjectClose, QString(":/images/closeproj.png"));
    addToolButton(projToolBar, btnProjectApp, QString(":/images/project.png"));

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
    addToolButton(projToolBar, btnProjectProperties, QString(":/images/properties.png"));
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

    QToolButton *btnDownloadSdCard = new QToolButton(this);
    addToolButton(toolsToolBar, btnDownloadSdCard, QString(":/images/download.png"));
    connect(btnDownloadSdCard, SIGNAL(clicked()),this,SLOT(downloadSdCard()));
    btnDownloadSdCard->setToolTip(tr("Send File to Target SD Card."));

#endif

    programToolBar = addToolBar(tr("Program"));
    btnProgramDebugTerm = new QToolButton(this);
    btnProgramRun = new QToolButton(this);
    QToolButton *btnProgramBuild = new QToolButton(this);
    QToolButton *btnProgramBurnEEP = new QToolButton(this);

    addToolButton(programToolBar, btnProgramBuild, QString(":/images/build.png"));
    addToolButton(programToolBar, btnProgramBurnEEP, QString(":/images/burnee.png"));
    addToolButton(programToolBar, btnProgramRun, QString(":/images/run.png"));
    addToolButton(programToolBar, btnProgramDebugTerm, QString(":/images/runconsole.png"));

    connect(btnProgramBuild,SIGNAL(clicked()),this,SLOT(programBuild()));
    connect(btnProgramBurnEEP,SIGNAL(clicked()),this,SLOT(programBurnEE()));
    connect(btnProgramDebugTerm,SIGNAL(clicked()),this,SLOT(programDebug()));
    connect(btnProgramRun,SIGNAL(clicked()),this,SLOT(programRun()));

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
    cbPort = new QComboBox(this);
    cbPort->setLayoutDirection(Qt::LeftToRight);
    cbPort->setToolTip(tr("Serial Port Select"));
    cbPort->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(cbPort,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentPort(int)));

    btnConnected = new QToolButton(this);
    btnConnected->setToolTip(tr("Serial Port Console"));
    btnConnected->setCheckable(true);
    connect(btnConnected,SIGNAL(clicked()),this,SLOT(connectButton()));

    QToolButton *reset = new QToolButton(this);
    reset->setToolTip(tr("Reset Port"));
    connect(reset,SIGNAL(clicked()),this,SLOT(portResetButton()));

    QToolButton *btnPortScan = new QToolButton(this);
    btnPortScan->setToolTip(tr("Rescan Serial Ports"));
    connect(btnPortScan,SIGNAL(clicked()),this,SLOT(enumeratePorts()));
#ifdef BOARD_TOOLBAR
    QToolButton *btnLoadBoards = new QToolButton(this);
    btnLoadBoards->setToolTip(tr("Reload Board List"));
    connect(btnLoadBoards,SIGNAL(clicked()),this,SLOT(initBoardTypes()));
#endif
    addToolButton(ctrlToolBar, btnConnected, QString(":/images/console.png"));
    addToolButton(ctrlToolBar, reset, QString(":/images/reset.png"));
    addToolButton(ctrlToolBar, btnPortScan, QString(":/images/refresh.png"));
    ctrlToolBar->addWidget(cbPort);
#ifdef BOARD_TOOLBAR
    addToolButton(ctrlToolBar, btnLoadBoards, QString(":/images/hardware.png"));
    ctrlToolBar->addWidget(cbBoard);
#endif
    ctrlToolBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}

