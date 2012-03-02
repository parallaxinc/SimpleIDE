#include "mainwindow.h"
#include "qextserialenumerator.h"
#include "Sleeper.h"

#define APPWINDOW_MIN_HEIGHT 530
#define APPWINDOW_MIN_WIDTH 780
#define EDITOR_MIN_WIDTH 500
#define PROJECT_WIDTH 230

#define SOURCE_FILE_TYPES "Source Files (*.c | *.cpp | *.h | *.cogc | *.spin | *.*)"
#define ENABLE_CTRL_RIGHTCLICK_FIND 0

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    /* setup application registry info */
    QCoreApplication::setOrganizationName(publisherKey);
    QCoreApplication::setOrganizationDomain(publisherComKey);
    QCoreApplication::setApplicationName(ASideGuiKey);

    /* global settings */
    settings = new QSettings(publisherKey, ASideGuiKey, this);

    /* setup properties dialog */
    propDialog = new Properties(this);
    connect(propDialog,SIGNAL(accepted()),this,SLOT(propertiesAccepted()));

    /* setup new project dialog */
    newProjDialog = new NewProject(this);
    connect(newProjDialog,SIGNAL(accepted()),this,SLOT(newProjectAccepted()));

    /* setup find dialog */
    findDialog = new FindDialog(this);

    /* setup find dialog */
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

    /* setup gui components */
    setupFileMenu();
    setupHelpMenu();
    setupToolBars();

    /* show gui */
    QApplication::processEvents();

    /* get app settings at startup and before any compiler call */
    getApplicationSettings();

    /* get user's last open path */
    QVariant  lastfile = settings->value(lastFileNameKey);
    if(lastfile.canConvert(QVariant::String)) {
        lastPath = sourcePath(lastfile.toString());
    }

    /* set up ctag tool */
    ctags = new CTags(aSideCompilerPath);

    initBoardTypes();

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

    /* start a process object for the loader to use */
    process = new QProcess(this);

    /* setup the terminal dialog box */
    term = new Terminal(this, portListener);
    term->setWindowTitle(QString(ASideGuiKey)+" Terminal");
    connect(term,SIGNAL(accepted()),this,SLOT(terminalClosed()));
    connect(term,SIGNAL(rejected()),this,SLOT(terminalClosed()));

    /* tell port listener to use terminal editor for i/o */
    termEditor = term->getEditor();

    /* setup the port listener */
    portListener = new PortListener(this, termEditor);

    /* get available ports at startup */
    enumeratePorts();

    portListener->setTerminalWindow(termEditor);

    /* load the last file into the editor to make user happy */
    QVariant lastfilev = settings->value(lastFileNameKey);
    if(!lastfilev.isNull()) {
        if(lastfilev.canConvert(QVariant::String)) {
            QString fileName = lastfilev.toString();
            openFileName(fileName);
            setProject(); // last file is always first project
        }
    }

    // old hardware dialog configuration feature
    //  hardwareDialog = new Hardware(this);
    //  connect(hardwareDialog,SIGNAL(accepted()),this,SLOT(initBoardTypes()));
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
        QChar c = event->text().at(0);
        key = (int)c.toAscii();
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

    /* get the separator used at startup */
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
    QMessageBox mbox(QMessageBox::Question, "Save File?", "",
                     QMessageBox::Discard | QMessageBox::Save | QMessageBox::SaveAll, this);

    saveProjectOptions();

    for(int tab = editorTabs->count()-1; tab > -1; tab--)
    {
        QString tabName = editorTabs->tabText(tab);
        if(tabName.at(tabName.length()-1) == '*')
        {
            mbox.setInformativeText(tr("Save File: ") + tabName.mid(0,tabName.indexOf(" *")) + tr(" ?"));
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
    /* never leave port open */
    portListener->close();

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

    delete findDialog;
    delete replaceDialog;
    delete propDialog;
    delete projectOptions;
    delete term;
}

void MainWindow::newFile()
{
    fileChangeDisable = true;
    setupEditor();
    int tab = editors->count()-1;
    editorTabs->addTab(editors->at(tab),(const QString&)untitledstr);
    editorTabs->setCurrentIndex(tab);
    QPlainTextEdit *ed = editors->at(tab);
    ed->setFocus();
    fileChangeDisable = false;
}

void MainWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull()) {
        fileName = fileDialog.getOpenFileName(this, tr("Open File"), lastPath, tr(SOURCE_FILE_TYPES));
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
    }
    if(fileName.indexOf(".side") > 0) {
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
        updateProjectTree(fileName,"");
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
            data = file.readAll();
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
            if(editorTabs->tabText(0) == untitledstr) {
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
    this->projectOptions->clearOptions();
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
    projectFile = path+"/"+name+".side";
    setCurrentProject(projectFile);
    updateProjectTree(mainName,"");
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
    if(fileName.indexOf(".side") > 0) {
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
        updateProjectTree(fileName,"");
    }
    openFileName(fileName);
    if(projectFile.length() == 0) {
        setProject();
    }
    else if(editorTabs->count() == 1) {
        setProject();
    }
}

/*
 * not used. close project automatically saves
 */
void MainWindow::saveProject()
{

}

/*
 * close project runs through project file list and closes files.
 * finally it closes the project manager side bar.
 */
void MainWindow::closeProject()
{
    /* save options
     */
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
            // close exact tab
            if(editorTabs->tabToolTip(tab).compare(sourcePath(projectFile)+list.at(n)) == 0)
                closeTab(tab);
        }
    }

    /* clean project manager selections
     */
    this->projectOptions->clearOptions();

    /* close project manager side bar
     */
    setWindowTitle(QString(ASideGuiKey));
    if(projectModel != NULL) {
        delete projectModel;
        projectModel = NULL;
    }
}


void MainWindow::openRecentProject(const QString &proj)
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    QStringList files = settings->value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings->setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()
{
    QStringList files = settings->value("recentFileList").toStringList();

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

    QStringList files = settings->value("recentProjectList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentProjects)
        files.removeLast();

    settings->setValue("recentProjectList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentProjectActions();
    }
}

void MainWindow::updateRecentProjectActions()
{
    QStringList projects = settings->value("recentProjectList").toStringList();

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

void MainWindow::saveFile(const QString &path)
{
    try {
        int n = this->editorTabs->currentIndex();
        QString fileName = editorTabs->tabToolTip(n);
        QString data = editors->at(n)->toPlainText();
        if(fileName.isEmpty())
            fileName = fileDialog.getSaveFileName(this, tr("Save As File"), lastPath, tr(SOURCE_FILE_TYPES));
        if(fileName.length() > 0)
            lastPath = sourcePath(fileName);
        if (fileName.isEmpty())
            return;
        this->editorTabs->setTabText(n,shortFileName(fileName));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(data.toAscii());
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

        this->editorTabs->setTabText(tab,shortFileName(fileName));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(data.toAscii());
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
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(data.toAscii());
                file.close();
            }
        }
    } catch(...) {
    }
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
    QPlainTextEdit *ed = editors->at(index);
    QTextCursor cur = ed->textCursor();
    int cpos = cur.position();
    if(cpos > 0) {
        QString txt = ed->toPlainText();
        char ch = txt[cpos-1].toAscii();
        if(ch == '\t') {
            int n = cur.columnNumber() % 4;
            fileChangeDisable = true;
            if(n) {
                cur.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
                cur.removeSelectedText();
                for(; n <= 4; n++) {
                    ed->insertPlainText(" ");
                }
            }
            fileChangeDisable = false;
            return; // just do tabs
        }
    }

    QString curtext = ed->toPlainText().trimmed();
    QString fileName = editorTabs->tabToolTip(index);
    QFile file(fileName);
    if(file.exists() == false) {
        return;
    }
    QString text;
    int ret = 0;
    if(file.open(QFile::ReadOnly | QFile::Text))
    {
        text = file.readAll().trimmed();
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

void MainWindow::printFile(const QString &path)
{
    /*
    QString fileName = path;
    QString data = editor->toPlainText();

    if (!fileName.isEmpty()) {
    }
    */
}

void MainWindow::zipFile(const QString &path)
{
    /*
    QString fileName = path;
    QString data = editor->toPlainText();

    if (!fileName.isEmpty()) {
    }
    */
}

void MainWindow::copyFromFile()
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->copy();
}
void MainWindow::cutFromFile()
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->cut();
}
void MainWindow::pasteToFile()
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->paste();
}
void MainWindow::editCommand()
{

}
void MainWindow::systemCommand()
{

}

void MainWindow::findInEditor(QPlainTextEdit *editor, QTextDocument::FindFlag flag)
{
    if(editor) {
        findDialog->show();
        findDialog->raise();
        findDialog->activateWindow();

        if (findDialog->exec() == QDialog::Accepted) {
            QString text = findDialog->getFindText();
            QString edtext = editor->toPlainText();
            if (edtext.contains(text,Qt::CaseInsensitive)) {
                editor->setCenterOnScroll(true);
                editor->find(text, flag);
            } else {
                QMessageBox::information(this, tr("Text Not Found"),
                    tr("Can't find text: \"%1\"").arg(text));
                return;
            }
        }
    }
}

void MainWindow::findInFile()
{
    if(!findDialog)
        return;
    findDialog->clearFindText();
    QString text = editors->at(editorTabs->currentIndex())->textCursor().selectedText();
    if(text.isEmpty() == false)
        findDialog->setFindText(text);
    findInEditor(editors->at(editorTabs->currentIndex()));
}

void MainWindow::findNextInFile()
{
    if(!findDialog)
        return;
    findInEditor(editors->at(editorTabs->currentIndex()));
}

void MainWindow::findPrevInFile()
{
    if(!findDialog)
        return;
    findInEditor(editors->at(editorTabs->currentIndex()),QTextDocument::FindBackward);
}

void MainWindow::replaceInFile()
{
    if(!replaceDialog)
        return;

    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());

    replaceDialog->clearFindText();
    QString text = editors->at(editorTabs->currentIndex())->textCursor().selectedText();
    if(text.isEmpty() == false)
        replaceDialog->setFindText(text);
    replaceDialog->clearReplaceText();

    replaceDialog->show();
    replaceDialog->raise();
    replaceDialog->activateWindow();
    replaceDialog->setEditor(editor);
}

void MainWindow::replaceNextInFile()
{
}
void MainWindow::replacePrevInFile()
{
}
void MainWindow::replaceAllInFile()
{
}

void MainWindow::findDeclaration(QPoint point)
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->cursorForPosition(point);
    findDeclaration(cur);
}

void MainWindow::findDeclaration()
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->textCursor();
    findDeclaration(cur);
}

void MainWindow::findDeclaration(QTextCursor cur)
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    if(editor) {
        ctags->runCtags(sourcePath(projectFile));
        /* find word */
        cur.movePosition(QTextCursor::StartOfWord,QTextCursor::MoveAnchor);
        cur.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor);
        editor->setTextCursor(cur);
        QString text = cur.selectedText();

        /* find line for traceback */
        cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
        cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);

        int index = editorTabs->currentIndex();
        QString fileName = editorTabs->tabToolTip(index);
        QString currentTag = "/\t" + fileName + "\t" + cur.selectedText();

        if(text.length() == 0) {
            findDeclarationInfo();
            return;
        }
        QString tagLine = ctags->findTag(text);

        if(showDeclaration(tagLine) > -1) {
            ctags->tagPush(currentTag);
        }
        else
            findDeclarationInfo();
    }
}

void MainWindow::findDeclarationInfo()
{
#if ENABLE_CTRL_RIGHTCLICK_FIND
    QMessageBox::information(this,
        tr("Find Function Declaration"),
        tr("Please note: library functions will not be found\n\n" \
           "One of these methods should find a function declaration.\n"\
           "1) Use \"Ctrl+Right Mouse Click\" over the symbol name.\n" \
           "2) Put the cursor on the symbol and use the keyboard shortcut for " \
           "\"Menu->Edit->Find Declaration\".\n"),
        QMessageBox::Ok);
#else
    QMessageBox::information(this,
        tr("Find Function Declaration"),
        tr("Please note: library functions will not be found.\n\n" \
           "To find a function declaration, "\
           "put the cursor on the symbol and use the keyboard shortcut for " \
           "\"Menu->Edit->Find Declaration\".\n"),
        QMessageBox::Ok);
#endif
}

void MainWindow::prevDeclaration()
{
    QString tagline = ctags->tagPop();
    if(tagline.indexOf("/ ") == 0) {
        QMessageBox::information(this,
            tr("Back from Declaration"),tr("Can't go back. The \"Find Function Declaration\" stack is empty."),
            QMessageBox::Ok);
        return;
    }
    if(tagline.length() > 0) {
        showDeclaration(tagline);
    }
    else {
        QMessageBox::information(this,
            tr("Back from Declaration"),tr("Can't go back. The \"Find Function Declaration\" stack is empty."),
            QMessageBox::Ok);
    }
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    QTextCursor cur = editor->textCursor();
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    //editor->setTextCursor(cur);
}

int MainWindow::showDeclaration(QString tagline)
{
    int rc = -1;
    if(tagline.length() == 0)
        return rc;
    QString file = ctags->getFile(tagline);
    if(file.length() == 0)
        return rc;
    int  linenum = ctags->getLine(tagline);
    if(linenum < 0)
        return rc;
    this->openFileName(file);
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    editor->setCenterOnScroll(true);
    QTextCursor cur = editor->textCursor();
    //cur.setPosition(linenum,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::Start);
    cur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,linenum);
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    //cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    editor->setTextCursor(cur);
    QString res = cur.selectedText();
    qDebug() << res;
    return linenum;
}

// TODO add redoAllChanges
void MainWindow::redoChange()
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->redo();
}
// TODO add undoAllChanges
void MainWindow::undoChange()
{
    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
    if(editor)
        editor->undo();
}

void MainWindow::setProject()
{
    int index = editorTabs->currentIndex();
    QString fileName = editorTabs->tabToolTip(index);
    if(fileName.length() != 0)
    {
        updateProjectTree(fileName,"");
        setCurrentProject(projectFile);
    }
    else {
        delete projectModel;
        projectModel = NULL;
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
}

void MainWindow::programBuild()
{
    runBuild();
}

void MainWindow::programBurnEE()
{
    if(runBuild())
        return;
    runLoader("-e");
}

void MainWindow::programRun()
{
    if(runBuild())
        return;
    runLoader("-r");
}

void MainWindow::programDebug()
{
    if(runBuild())
        return;

    if(runLoader("-r -t"))
        return;

    btnConnected->setChecked(true);
    term->getEditor()->setPortEnable(true);
    term->getEditor()->setPlainText("");
    portListener->open();
    term->activateWindow();
    term->show();
    term->getEditor()->setFocus();
}

void MainWindow::terminalClosed()
{
    portListener->close();
    btnConnected->setChecked(false);
}

void MainWindow::setupHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(helpMenu);
    aboutDialog = new AboutDialog(this);

    helpMenu->addAction(QIcon(":/images/about.png"), tr("&About"), this, SLOT(aboutShow()));
    helpMenu->addAction(QIcon(":/images/helphint.png"), tr("&Help"), this, SLOT(helpShow()));
}

void MainWindow::aboutShow()
{
    aboutDialog->show();
}

void MainWindow::helpShow()
{
    QString license(tr("SimpleIDE is an MIT License open-source IDE.<br/>SimpleIDE is built Qt libraries under LGPLv2.1<br/><br/>"));
    QString icons(tr("Most Icons: <a href=\"http://www.small-icons.com/packs/24x24-free-application-icons.htm\">www.aha-soft.com 24x24 Free Application Icons</a>" \
                     "<br/>with Creative Commons Attribution-Share Alike 3.0 License.<br/>"));
    QString ctags(tr("Uses <a href=\"http://ctags.sourceforge.net\">ctags</a> binary program under GPLv2 for source browsing.<br/>"));
    QString propgcc(tr("Uses <a href=\"http://propgcc.googlecode.com\">Propeller GCC tool chain</a> GCC 4.6.1 base under GPLv3<br/>"));
    QString sources(tr("All IDE, ctags, and icon sources kept in <a href=\"http://propside.googlecode.com\">SimpleIDE repository</a>.<br/><br/>" \
                       "All license documents included in the Simple-IDE package."));

    QMessageBox::about(this, ASideGuiKey+tr(" help"),
        tr("<p><b>")+ASideGuiKey+tr("</b> is an integrated C development environment <br/>"\
           "which manages Propeller GCC program builds, and <br/>" \
           "loads programs to Propeller for many board types.</p>") +
        tr("Visit <a href=\"https://sites.google.com/site/propellergcc/simpleide\">")+
        ASideGuiKey+tr("</a> on the web for help and more information.<br/><br/>")+
        license+icons+ctags+propgcc+sources);
    //QMessageBox::aboutQt(this,tr("About Qt"));
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

    int len = projectModel->rowCount();
    for(int n = 0; n < len; n++)
    {
        QModelIndex root = projectModel->index(n,0);
        QVariant vs = projectModel->data(root, Qt::DisplayRole);
        if(!vs.canConvert(QVariant::String))
            continue;
        QString name = vs.toString();
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

int  MainWindow::runBuild(void)
{
    int rc = 0;
    QStringList clist;
    QFile file(projectFile);
    QString proj = "";
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        proj = file.readAll();
        file.close();
    }

    proj = proj.trimmed(); // kill extra white space
    QStringList list = proj.split("\n");

    /* Calculate the number of compile steps for progress.
     * Skip empty lines and don't count ">" parameters.
     */
    int maxprogress = list.length()-1;

    /* If we don't have a list we can't compile!
     */
    if(maxprogress < 1)
        return -1;

    btnConnected->setChecked(false);
    portListener->close(); // disconnect uart before use

    progress->show();
    programSize->setText("");

    compileStatus->setPlainText(tr("Project Directory: ")+sourcePath(projectFile)+"\r\n\n");
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

    /* Run through file list and compile according to extension.
     */
    for(int n = 0; n < list.length(); n++) {
        progress->setValue(100*n/maxprogress);
        QString name = list[n];
        if(name.length() == 0)
            continue;
        if(name.at(0) == '>')
            continue;
        QString base = shortFileName(name.mid(0,name.lastIndexOf(".")));
        if(name.toLower().lastIndexOf(".spin") > 0) {
            if(runBstc(name))
                return -1;
            if(proj.toLower().lastIndexOf(".dat") < 0) // intermediate
                list.append((name.mid(0,name.lastIndexOf(".spin"))+".dat"));
        }
        else if(name.toLower().lastIndexOf(".dat") > 0) {
            if(runObjCopy(name))
                return -1;
            if(proj.toLower().lastIndexOf("_firmware.o") < 0)
                clist.append(name.mid(0,name.lastIndexOf(".dat"))+"_firmware.o");
        }
        else if(name.toLower().lastIndexOf(".s") > 0) {
            if(runGAS(name))
                return -1;
            if(proj.toLower().lastIndexOf(".o") < 0)
                clist.append(name.mid(0,name.lastIndexOf(".s"))+".o");
        }
        /* .cogc also does COG specific objcopy */
        else if(name.toLower().lastIndexOf(".cogc") > 0) {
            if(runCOGC(name))
                return -1;
            clist.append(base+".cog");
        }
        else {
            clist.append(name);
        }
    }

    rc = runCompiler(clist);
    Sleeper::ms(250);
    progress->hide();

    return rc;
}

int  MainWindow::runCOGC(QString name)
{
    int rc = 0; // return code

    QString base = shortFileName(name.mid(0,name.lastIndexOf(".")));
    // copy .cog to .c
    // QFile::copy(sourcePath(projectFile)+name,sourcePath(projectFile)+base+".c");
    // run C compiler on new file
    QStringList args;
    args.append("-r");  // relocatable ?
    args.append("-Os"); // default optimization for -mcog
    args.append("-mcog"); // compile for cog
    args.append("-o"); // create a .cog object
    args.append(base+".cog");
    args.append("-xc"); // code to compile is C code
    //args.append("-c");
    args.append(name);

    /* run compiler */
    QString compstr = shortFileName(aSideCompiler);
    rc = startProgram(compstr, sourcePath(projectFile),args);
    if(rc) return rc;

    /* now do objcopy */
    args.clear();
    args.append("--localize-text");
    args.append("--rename-section");
    args.append(".text="+base+".cog");
    args.append(base+".cog");


    /* run object copy to localize fix up .cog object */

    QString objcopy = "propeller-elf-objcopy";
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
    args.append(shortFileName(spinfile));

    checkAndSaveFiles();

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


int  MainWindow::runCogObjCopy(QString datfile)
{
    int rc = 0;

    getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args;

    args.append("--localize-text");
    args.append("--rename-section");
    args.append(".text="+datfile);
    args.append(datfile);

    checkAndSaveFiles();

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

    checkAndSaveFiles();

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

    checkAndSaveFiles();

    /* run the as program */
    QString gas = "propeller-elf-as";
    rc = startProgram(gas, sourcePath(projectFile), args);

    return rc;
}

QStringList MainWindow::getCompilerParameters(QStringList copts)
{
    // use the projectFile instead of the current tab file
    QString srcpath = sourcePath(projectFile);

    portName = cbPort->itemText(cbPort->currentIndex());
    boardName = cbBoard->itemText(cbBoard->currentIndex());

    QString model = projectOptions->getMemModel();

    QStringList args;
    args.append("-o");
    args.append("a.out");
    args.append(projectOptions->getOptimization());
    args.append("-m"+model);

    if(projectOptions->getWarnAll().length())
        args.append(projectOptions->getWarnAll());
    if(projectOptions->get32bitDoubles().length())
        args.append(projectOptions->get32bitDoubles());
    if(projectOptions->getExceptions().length())
        args.append(projectOptions->getExceptions());
    if(projectOptions->getNoFcache().length())
        args.append(projectOptions->getNoFcache());

    if(projectOptions->getSimplePrintf().length()) {
        /* don't use simple printf flag for COG model programs. */
        if(model.contains("cog",Qt::CaseInsensitive) == false)
            args.append(projectOptions->getSimplePrintf());
        else {
            this->compileStatus->insertPlainText(tr("Ignoring")+" \"Simple printf\""+tr(" flag in COG mode program.")+"\n");
            this->compileStatus->moveCursor(QTextCursor::End);
        }
    }

    if(projectOptions->getCompiler().indexOf("++") > -1)
        args.append("-fno-rtti");

    /* other compiler options */
    if(projectOptions->getCompOptions().length())
        args.append(projectOptions->getCompOptions());

    /* files */
    for(int n = 0; n < copts.length(); n++) {
        if(copts[n].length() > 0) {
            if(copts[n].indexOf(".c") > 0) // x.c
                args.append(copts[n]);
            else if(copts[n].indexOf(".o") > 0) // x.o
                args.append(copts[n]);
        }
    }

    /* libraries */
    if(projectOptions->getMathLib().length())
        args.append(projectOptions->getMathLib());
    if(projectOptions->getPthreadLib().length())
        args.append(projectOptions->getPthreadLib());

    /* other linker options */
    if(projectOptions->getLinkOptions().length())
        args.append(projectOptions->getLinkOptions());

    /* strip */
    if(projectOptions->getStripElf().length())
        args.append(projectOptions->getStripElf());

    qDebug() << args;
    return args;
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

    checkAndSaveFiles();

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

    /*
     * Report program size
     * Use the projectFile instead of the current tab file
     */
    QString srcpath = sourcePath(projectFile);
    QFile aout(srcpath+"a.out");
    QString ssize = QString("Compiled %L1 Bytes").arg(aout.size());
    programSize->setText(ssize);

    return rc;
}

QStringList MainWindow::getLoaderParameters(QString copts)
{
    // use the projectFile instead of the current tab file
    // QString srcpath = sourcePath(projectFile);

    portName = cbPort->itemText(cbPort->currentIndex());
    boardName = cbBoard->itemText(cbBoard->currentIndex());

    QStringList args;
    args.append(tr("-b"));
    args.append(boardName);
    args.append(tr("-p"));
    args.append(portName);
    args.append(tr("-I"));
    args.append(aSideIncludes);
    /* set working directory in process makes this unnecessary */
    //args.append(srcpath+"a.out");
    args.append("a.out");
    QStringList olist = copts.split(" ");
    for (int n = 0; n < olist.length(); n++)
        args.append(olist[n]);

    qDebug() << args;
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

    process->setProperty("Terminal", QVariant(false));
    if(copts.indexOf(" -t") > 0) {
        copts = copts.mid(0,copts.indexOf(" -t"));
        process->setProperty("Terminal", QVariant(true));
    }

    process->setProperty("Name", QVariant(aSideLoader));
    process->setProperty("IsLoader", QVariant(true));

    QStringList args = getLoaderParameters(copts);

    btnConnected->setChecked(false);
    portListener->close(); // disconnect uart before use

    showBuildStart(aSideLoader,args);

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

    progress->hide();
    return process->exitCode();
}

int  MainWindow::startProgram(QString program, QString workpath, QStringList args)
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

    connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(procReadyRead()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(procFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)));

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(workpath);

    procDone = false;
    process->start(program,args);

    /* process Qt application events until procDone
     */
    while(procDone == false)
        QApplication::processEvents();

    progress->hide();
    return process->exitCode();
}

void MainWindow::procError(QProcess::ProcessError error)
{
    QVariant name = process->property("Name");
    compileStatus->appendPlainText(name.toString() + tr(" error ... (%1)").arg(error));
    compileStatus->appendPlainText(process->readAllStandardOutput());
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
#if 0
    QVariant myterm = process->property("Terminal");
    if(myterm.toBool() && exitCode == 0) {
        btnConnected->setChecked(true);
        term->getEditor()->setPlainText("");
        portListener->open();
        term->show();
    }
#endif
}

void MainWindow::procReadyRead()
{
    QByteArray bytes = process->readAllStandardOutput();
    if(bytes.length() == 0)
        return;

#if defined(Q_WS_WIN32)
    QString eol("\r");
#else
    QString eol("");
#endif

    QStringList lines = QString(bytes).split("\r\n");
    for (int n = 0; n < lines.length(); n++) {
        QString line = lines[n];
        if(line.length() > 0) {
            if(line.contains("Propeller Version",Qt::CaseInsensitive)) {
                compileStatus->insertPlainText(eol+line+eol);
                progress->setValue(0);
            }
            else
            if(line.contains("loading",Qt::CaseInsensitive)) {
                progMax = 0;
                progress->setValue(0);
            }
            else
            if(line.contains("writing",Qt::CaseInsensitive)) {
                progress->setValue(50);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("Download OK",Qt::CaseInsensitive)) {
                progress->setValue(100);
                compileStatus->insertPlainText(line+eol);
            }
            else
            if(line.contains("remaining",Qt::CaseInsensitive)) {
                if(progMax == 0) {
                    QString bs = line.mid(0,line.indexOf(" "));
                    progMax = bs.toInt();
                    progMax /= 1024;
                    progCount = 0;
                    if(progMax == 0) {
                        progress->setValue(100);
                    }
                }
                if(progMax != 0) {
                    progCount++;
                    progress->setValue(100*progCount/progMax);
                }
            }
#if 0
            // can't do this otherwise error info gets lost.
            else
            if(line.contains("error:",Qt::CaseInsensitive)) {
                status->setText(status->text()+line+" ");
            }
#endif
            else {
                compileStatus->insertPlainText(line+eol);
            }
            compileStatus->moveCursor(QTextCursor::StartOfLine);
            compileStatus->moveCursor(QTextCursor::End);
        }
    }
}

int  MainWindow::checkBuildStart(QProcess *proc, QString progName)
{
    QMessageBox mbox;
    mbox.setStandardButtons(QMessageBox::Ok);
    qDebug() << QDir::currentPath();
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
    qDebug() << progName+argstr;
    compileStatus->insertPlainText(shortFileName(progName)+argstr+"\n");
    compileStatus->moveCursor(QTextCursor::End);
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
        mbox.setInformativeText(tr("Save File: ") + tabName.mid(0,tabName.indexOf(" *")) + tr(" ?"));
        int ret = mbox.exec();
        if(ret == QMessageBox::Save)
            saveFileByTabIndex(tab);
    }

    editors->at(tab)->setPlainText("");
    editors->remove(tab);
    if(editorTabs->count() == 1)
        newFile();
    editorTabs->removeTab(tab);

    fileChangeDisable = false;
}

void MainWindow::changeTab(int index)
{
/*
    if(editorTabs->count() == 1)
        setProject();
*/
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
    /* container for project, etc... */
    leftSplit = new QSplitter(this);
    leftSplit->setMinimumHeight(APPWINDOW_MIN_HEIGHT-60);
    leftSplit->setOrientation(Qt::Vertical);
    vsplit->addWidget(leftSplit);

    /* project tree */
    projectTree = new QTreeView(this);
    projectTree->setMinimumWidth(PROJECT_WIDTH);
    projectTree->setMaximumWidth(PROJECT_WIDTH);
    projectTree->setToolTip(tr("Current Project"));
    connect(projectTree,SIGNAL(pressed(QModelIndex)),this,SLOT(projectTreeClicked(QModelIndex)));
    leftSplit->addWidget(projectTree);

    // projectMenu is popup for projectTree
    projectMenu = new QMenu(QString("Project Menu"));
    projectMenu->addAction(tr("Add File"), this,SLOT(addProjectFile()));
    projectMenu->addAction(tr("Delete File"), this,SLOT(deleteProjectFile()));
    projectMenu->addAction(tr("Show File"), this,SLOT(showProjectFile()));

    projectOptions = new ProjectOptions(this);
    projectOptions->setMinimumWidth(PROJECT_WIDTH);
    projectOptions->setMaximumWidth(PROJECT_WIDTH);
    projectOptions->setToolTip(tr("Project Options"));
    leftSplit->addWidget(projectOptions);

    QList<int> lsizes = leftSplit->sizes();
    lsizes[0] = leftSplit->height()*2/10;
    lsizes[1] = leftSplit->height()*8/10;
    leftSplit->setSizes(lsizes);

    rightSplit = new QSplitter(this);
    rightSplit->setMinimumHeight(APPWINDOW_MIN_HEIGHT-50);
    rightSplit->setOrientation(Qt::Vertical);
    vsplit->addWidget(rightSplit);

    /* project editors */
    editors = new QVector<QPlainTextEdit*>();

    /* project editor tabs */
    editorTabs = new QTabWidget(this);
    editorTabs->setTabsClosable(true);
    connect(editorTabs,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));
    connect(editorTabs,SIGNAL(currentChanged(int)),this,SLOT(changeTab(int)));
    rightSplit->addWidget(editorTabs);

    compileStatus = new QPlainTextEdit(this);
    compileStatus->setLineWrapMode(QPlainTextEdit::NoWrap);
    connect(compileStatus,SIGNAL(selectionChanged()),this,SLOT(compileStatusClicked()));
    rightSplit->addWidget(compileStatus);

    QList<int> rsizes = rightSplit->sizes();
    rsizes[0] = rightSplit->height()*3/4;
    rsizes[1] = rightSplit->height()*1/4;
    rightSplit->setSizes(rsizes);

    /* status bar for progressbar */
    QStatusBar *statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);
    progress = new QProgressBar();
    progress->setMaximumSize(90,20);
    progress->hide();

    programSize = new QLabel();
    programSize->setMinimumWidth(PROJECT_WIDTH+2);
    status = new QLabel();

    statusBar->addPermanentWidget(progress);
    statusBar->addWidget(programSize);
    statusBar->addWidget(status);
    statusBar->setMaximumHeight(22);

}

/*
 * Find error or warning in a file
 */
void MainWindow::compileStatusClicked(void)
{
    int n = 0;
    QTextCursor cur = compileStatus->textCursor();
    cur.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
    cur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    compileStatus->setTextCursor(cur);
    QString line = cur.selectedText();
    QStringList list = line.split(":");
    if(list.count() < 2)
        return;

    QString file = list[0];
    /* open file in tab if not there already */
    for(n = 0; n < editorTabs->count();n++) {
        if(editorTabs->tabText(n).contains(file)) {
            editorTabs->setCurrentIndex(n);
            break;
        }
    }
    if(n > editorTabs->count()-1) {
        file = sourcePath(projectFile)+list[0];
        openFileName(file);
    }

    QPlainTextEdit *editor = editors->at(editorTabs->currentIndex());
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

void MainWindow::projectTreeClicked(QModelIndex index)
{
    if(projectModel == NULL)
        return;

    projectIndex = index;
    Qt::MouseButtons buttons = QApplication::mouseButtons();
    if(buttons == Qt::RightButton) {
        projectMenu->popup(QCursor::pos());
    }
    else {
        showProjectFile();
    }
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

        QString ext = fileName.mid(fileName.lastIndexOf("."));
        if(ext.length()) {
            ext = ext.toLower();
            if(ext == ".cog") {
                // don't copy .cog files
            }
            else if(ext == ".dat") {
                // don't copy .dat files
            }
            else if(ext == ".o") {
                // don't copy .o files
            }
            else if(ext == ".out") {
                // don't copy .out files
            }
            else if(ext == ".side") {
                // don't copy .side files
            }
            else {
                QFile copy(sourcePath(projectFile)+this->shortFileName(fileName));
                QString copystr = "";
                QFile reader(fileName);
                if(reader.open(QFile::ReadOnly | QFile::Text)) {
                    copystr = reader.readAll();
                    reader.close();
                }
                if(copy.open(QFile::WriteOnly | QFile::Text)) {
                    copy.write(copystr.toAscii());
                    copy.close();
                }
            }
        }

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
            projstr += this->shortFileName(fileName) + "\n";
            list.clear();
            list = projectOptions->getOptions();

            foreach(QString arg, list) {
                projstr += ">"+arg+"\n";
            }
            if(file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(projstr.toAscii());
                file.close();
            }
        }
        updateProjectTree(sourcePath(projectFile)+mainFile,"");
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
        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }
    }
    updateProjectTree(sourcePath(projectFile)+mainFile,"");
}

void MainWindow::showProjectFile()
{
    QString fileName;

    QVariant vs = projectModel->data(projectIndex, Qt::DisplayRole);
    if(vs.canConvert(QVariant::String))
    {
        fileName = vs.toString();
        /* Temporarily disallow opening .spin files
         * until we know how to handle them.
         */
        if(fileName.indexOf(".spin",Qt::CaseInsensitive) < 0)
            openFileName(sourcePath(projectFile)+fileName);
    }
}

/*
 * save project file with options.
 */
void MainWindow::saveProjectOptions()
{
    QString projstr = "";
    QStringList list;

    if(projectFile.length() > 0)
        setWindowTitle(QString(ASideGuiKey)+" "+projectFile);

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
                projectOptions->setOptions(arg);
            else
                projstr += arg + "\n";
        }
        list.clear();
        list = projectOptions->getOptions();
        foreach(QString arg, list) {
            if(arg.compare(ProjectOptions::board+"::") == 0)
                projstr += ">"+ProjectOptions::board+"::"+cbBoard->currentText()+"\n";
            else
                projstr += ">"+arg+"\n";
        }

        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(projstr.toAscii());
            file.close();
        }
    }
}

/*
 * update project tree and options by reading from project.side file
 */
void MainWindow::updateProjectTree(QString fileName, QString text)
{
    QString projName = this->shortFileName(fileName);
    projName = projName.mid(0,projName.lastIndexOf("."));
    projName += ".side";

    basicPath = sourcePath(fileName);
    projectFile = basicPath+projName;
    setWindowTitle(QString(ASideGuiKey)+" "+projectFile);

    if(projectModel != NULL) delete projectModel;
    projectModel = new CBuildTree(projName, this);

    QFile file(projectFile);
    if(!file.exists()) {
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(this->shortFileName(fileName).toAscii());
            projectModel->addRootItem(this->shortFileName(fileName));
            file.close();
        }
    }
    else {
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
        if(mains.compare(projs) == 0) {
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

void MainWindow::updateReferenceTree(QString fileName, QString text)
{
}

void MainWindow::referenceTreeClicked(QModelIndex index)
{
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
            cbPort->addItem(name);
            friendlyPortName.append(ports.at(i).friendName);
        }
#else
        name = ports.at(i).physName;
        cbPort->addItem(name);
        friendlyPortName.append(ports.at(i).friendName);
#endif
    }
}

void MainWindow::connectButton()
{
    if(btnConnected->isChecked()) {
        term->getEditor()->setPortEnable(true);
        term->activateWindow();
        term->show();
        term->getEditor()->setFocus();
        portListener->open();
    }
    else {
        portListener->close();
        term->hide();
    }
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
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    QPlainTextEdit *editor = new QPlainTextEdit(this);
    editor->setTabStopWidth(40);
    editor->setFont(font);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    connect(editor,SIGNAL(textChanged()),this,SLOT(fileChanged()));
    highlighter = new Highlighter(editor->document());
    editors->append(editor);
}
/*
 * TODO: add ctrl-tab to cycle through tabs
 */
void MainWindow::setEditorTab(int num, QString shortName, QString fileName, QString text)
{
    QPlainTextEdit *editor = editors->at(num);
    fileChangeDisable = true;
    editor->setPlainText(text);

    /* connect later when we can handle multiple popup events
     */
#if ENABLE_CTRL_RIGHTCLICK_FIND
    connect(editor,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(editorMenu(QPoint)));
    editor->setContextMenuPolicy(Qt::CustomContextMenu);
#endif

    fileChangeDisable = false;
    editorTabs->setTabText(num,shortName);
    editorTabs->setTabToolTip(num,fileName);
    editorTabs->setCurrentIndex(num);
}

void MainWindow::editorMenu(QPoint point)
{
#if ENABLE_CTRL_RIGHTCLICK_FIND
    Qt::KeyboardModifiers keybm = QApplication::keyboardModifiers();
    if(keybm & Qt::ControlModifier){
        findDeclaration(point);
    }
    else {
        QPlainTextEdit *ed = editors->at(editorTabs->currentIndex());
        QTextCursor cur = ed->textCursor();
        QList<QAction*> list = edpopup->actions();
        if(cur.selectedText().length() == 0) {
            list.at(2)->setEnabled(false);
            list.at(3)->setEnabled(false);
        }
        else {
            list.at(2)->setEnabled(true);
            list.at(3)->setEnabled(true);
        }
        if(ed->canPaste())
            list.at(4)->setEnabled(true);
        else
            list.at(4)->setEnabled(false);

        edpopup->popup(QCursor::pos());
    }
#endif
}

/*
 * TODO: why don't icons show up in linux?
 * TODO: save as? pencil on a disk?
 */
void MainWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(QIcon(":/images/newfile.png"), tr("&New"), this, SLOT(newFile()), QKeySequence::New);
    fileMenu->addAction(QIcon(":/images/openfile.png"), tr("&Open"), this, SLOT(openFile()), QKeySequence::Open);
    fileMenu->addAction(QIcon(":/images/savefile.png"), tr("&Save"), this, SLOT(saveFile()), QKeySequence::Save);
    fileMenu->addAction(QIcon(":/images/saveasfile.png"), tr("Save &As"), this, SLOT(saveAsFile()),QKeySequence::SaveAs);

    fileMenu->addAction(QIcon(":/images/Delete.png"),tr("Close"), this, SLOT(closeFile()));
    fileMenu->addAction(tr("Close All"), this, SLOT(closeAll()));

    // recent file actions
    separatorFileAct = fileMenu->addSeparator();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);

    updateRecentFileActions();

    fileMenu->addSeparator();

    // fileMenu->addAction(QIcon(":/images/print.png"), tr("Print"), this, SLOT(printFile()), QKeySequence::Print);
    // fileMenu->addAction(QIcon(":/images/zip.png"), tr("Archive"), this, SLOT(zipFile()), 0);

    fileMenu->addAction(QIcon(":/images/exit.png"), tr("E&xit"), qApp, SLOT(quit()), QKeySequence::Quit);

    QMenu *projMenu = new QMenu(tr("&Project"), this);
    menuBar()->addMenu(projMenu);

    projMenu->addAction(QIcon(":/images/newproj.png"), tr("New Project"), this, SLOT(newProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_N);
    projMenu->addAction(QIcon(":/images/openproj.png"), tr("Open Project"), this, SLOT(openProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_O);
    projMenu->addAction(QIcon(":/images/closeproj.png"), tr("Save and Close Project"), this, SLOT(closeProject()), Qt::CTRL+Qt::ShiftModifier+Qt::Key_X);
    projMenu->addAction(QIcon(":/images/treeproject.png"), tr("Set Project"), this, SLOT(setProject()), Qt::Key_F4);
    //projMenu->addAction(QIcon(":/images/hardware.png"), tr("Load Board Types"), this, SLOT(hardware()), Qt::Key_F6);

    // recent project actions
    separatorProjectAct = projMenu->addSeparator();

    for (int i = 0; i < MaxRecentProjects; ++i) {
        recentProjectActs[i] = new QAction(this);
        recentProjectActs[i]->setVisible(false);
        connect(recentProjectActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentProject()));
    }

    for (int i = 0; i < MaxRecentProjects; ++i)
        projMenu->addAction(recentProjectActs[i]);

    updateRecentProjectActions();

    projMenu->addSeparator();
    projMenu->addAction(QIcon(":/images/properties.png"), tr("Properties"), this, SLOT(properties()), Qt::Key_F5);

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
    editMenu->addAction(QIcon(":/images/find.png"), tr("&Find"), this, SLOT(findInFile()), QKeySequence::Find);
    editMenu->addAction(QIcon(":/images/next.png"), tr("Find Next"), this, SLOT(findNextInFile()), QKeySequence::FindNext);
    editMenu->addAction(QIcon(":/images/previous.png"), tr("Find Previous"), this, SLOT(findPrevInFile()), QKeySequence::FindPrevious);

    editMenu->addAction(QIcon(":/images/replace.png"), tr("&Replace"), this, SLOT(replaceInFile()), Qt::CTRL + Qt::Key_R);

    if(ctags->enabled()) {
        editMenu->addSeparator();
        editMenu->addAction(QIcon(":/images/back.png"),tr("Go &Back"), this, SLOT(prevDeclaration()), QKeySequence::Back);
        editMenu->addAction(QIcon(":/images/forward.png"),tr("Find Function"), this, SLOT(findDeclaration()), QKeySequence::Forward);
    }

    editMenu->addSeparator();
    editMenu->addAction(QIcon(":/images/redo.png"), tr("&Redo"), this, SLOT(redoChange()), QKeySequence::Redo);
    editMenu->addAction(QIcon(":/images/undo.png"), tr("&Undo"), this, SLOT(undoChange()), QKeySequence::Undo);

    QMenu *debugMenu = new QMenu(tr("&Debug"), this);
    menuBar()->addMenu(debugMenu);

    debugMenu->addAction(QIcon(":/images/runconsole.png"), tr("Run Console"), this, SLOT(programDebug()), Qt::Key_F8);
    debugMenu->addAction(QIcon(":/images/build.png"), tr("Build"), this, SLOT(programBuild()), Qt::Key_F9);
    debugMenu->addAction(QIcon(":/images/run.png"), tr("Run"), this, SLOT(programRun()), Qt::Key_F10);
    debugMenu->addAction(QIcon(":/images/burnee.png"), tr("Burn"), this, SLOT(programBurnEE()), Qt::Key_F11);


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
    //QToolButton *btnFilePrint = new QToolButton(this);
    //QToolButton *btnFileZip = new QToolButton(this);

    addToolButton(fileToolBar, btnFileNew, QString(":/images/newfile.png"));
    addToolButton(fileToolBar, btnFileOpen, QString(":/images/openfile.png"));
    addToolButton(fileToolBar, btnFileSave, QString(":/images/savefile.png"));
    addToolButton(fileToolBar, btnFileSaveAs, QString(":/images/saveasfile.png"));
    //addToolButton(fileToolBar, btnFilePrint, QString(":/images/print.png"));
    //addToolButton(fileToolBar, btnFileZip, QString(":/images/zip.png"));

    connect(btnFileNew,SIGNAL(clicked()),this,SLOT(newFile()));
    connect(btnFileOpen,SIGNAL(clicked()),this,SLOT(openFile()));
    connect(btnFileSave,SIGNAL(clicked()),this,SLOT(saveFile()));
    connect(btnFileSaveAs,SIGNAL(clicked()),this,SLOT(saveAsFile()));
    //connect(btnFilePrint,SIGNAL(clicked()),this,SLOT(printFile()));
    //connect(btnFileZip,SIGNAL(clicked()),this,SLOT(zipFile()));

    btnFileNew->setToolTip(tr("New File"));
    btnFileOpen->setToolTip(tr("Open File"));
    btnFileSave->setToolTip(tr("Save File"));
    btnFileSaveAs->setToolTip(tr("Save As File"));
    //btnFilePrint->setToolTip(tr("Print"));
    //btnFileZip->setToolTip(tr("Archive"));

    /*
     * Project Toobar
     */
    projToolBar = addToolBar(tr("Project"));
    QToolButton *btnProjectNew = new QToolButton(this);
    QToolButton *btnProjectOpen = new QToolButton(this);
    QToolButton *btnProjectClose = new QToolButton(this);
    QToolButton *btnProjectApp = new QToolButton(this);


    addToolButton(projToolBar, btnProjectNew, QString(":/images/newproj.png"));
    addToolButton(projToolBar, btnProjectOpen, QString(":/images/openproj.png"));
    addToolButton(projToolBar, btnProjectClose, QString(":/images/closeproj.png"));
    addToolButton(projToolBar, btnProjectApp, QString(":/images/treeproject.png"));

    connect(btnProjectNew,SIGNAL(clicked()),this,SLOT(newProject()));
    connect(btnProjectOpen,SIGNAL(clicked()),this,SLOT(openProject()));
    connect(btnProjectClose,SIGNAL(clicked()),this,SLOT(closeProject()));
    connect(btnProjectApp,SIGNAL(clicked()),this,SLOT(setProject()));

    btnProjectNew->setToolTip(tr("New Project"));
    btnProjectOpen->setToolTip(tr("Open Project"));
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
        browseToolBar = addToolBar(tr("Source Browser"));
        QToolButton *btnBrowseBack = new QToolButton(this);
        addToolButton(browseToolBar, btnBrowseBack, QString(":/images/back.png"));
        connect(btnBrowseBack,SIGNAL(clicked()),this,SLOT(prevDeclaration()));
        btnBrowseBack->setToolTip("Back");

        QToolButton *btnFindDef = new QToolButton(this);
        addToolButton(browseToolBar, btnFindDef, QString(":/images/forward.png"));
        connect(btnFindDef,SIGNAL(clicked()),this,SLOT(findDeclaration()));
        btnFindDef->setToolTip("Find Function Declaration");
    }


    debugToolBar = addToolBar(tr("Debug"));
    QToolButton *btnDebugDebugTerm = new QToolButton(this);
    QToolButton *btnDebugRun = new QToolButton(this);
    QToolButton *btnDebugBuild = new QToolButton(this);
    QToolButton *btnDebugBurnEEP = new QToolButton(this);

    addToolButton(debugToolBar, btnDebugBuild, QString(":/images/build.png"));
    addToolButton(debugToolBar, btnDebugBurnEEP, QString(":/images/burnee.png"));
    addToolButton(debugToolBar, btnDebugRun, QString(":/images/run.png"));
    addToolButton(debugToolBar, btnDebugDebugTerm, QString(":/images/runconsole.png"));

    connect(btnDebugBuild,SIGNAL(clicked()),this,SLOT(programBuild()));
    connect(btnDebugBurnEEP,SIGNAL(clicked()),this,SLOT(programBurnEE()));
    connect(btnDebugDebugTerm,SIGNAL(clicked()),this,SLOT(programDebug()));
    connect(btnDebugRun,SIGNAL(clicked()),this,SLOT(programRun()));

    btnDebugBuild->setToolTip(tr("Build"));
    btnDebugBurnEEP->setToolTip(tr("Burn EEPROM"));
    btnDebugRun->setToolTip(tr("Run"));
    btnDebugDebugTerm->setToolTip(tr("Run Console"));

    ctrlToolBar = addToolBar(tr("Control"));
    ctrlToolBar->setLayoutDirection(Qt::RightToLeft);
    cbBoard = new QComboBox(this);
    cbPort = new QComboBox(this);
    cbBoard->setLayoutDirection(Qt::LeftToRight);
    cbPort->setLayoutDirection(Qt::LeftToRight);
    cbBoard->setToolTip(tr("Board Type Select"));
    cbPort->setToolTip(tr("Serial Port Select"));
    cbBoard->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cbPort->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(cbBoard,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentBoard(int)));
    connect(cbPort,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentPort(int)));

    btnConnected = new QToolButton(this);
    btnConnected->setToolTip(tr("Serial Port Console"));
    btnConnected->setCheckable(true);
    connect(btnConnected,SIGNAL(clicked()),this,SLOT(connectButton()));

    QToolButton *btnPortScan = new QToolButton(this);
    btnPortScan->setToolTip(tr("Rescan Serial Ports"));
    connect(btnPortScan,SIGNAL(clicked()),this,SLOT(enumeratePorts()));

    QToolButton *btnLoadBoards = new QToolButton(this);
    btnLoadBoards->setToolTip(tr("Reload Board List"));
    connect(btnLoadBoards,SIGNAL(clicked()),this,SLOT(initBoardTypes()));

    addToolButton(ctrlToolBar, btnConnected, QString(":/images/console.png"));
    addToolButton(ctrlToolBar, btnPortScan, QString(":/images/refresh.png"));
    ctrlToolBar->addWidget(cbPort);
    addToolButton(ctrlToolBar, btnLoadBoards, QString(":/images/hardware.png"));
    ctrlToolBar->addWidget(cbBoard);
    ctrlToolBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}

