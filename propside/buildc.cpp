#include "buildc.h"
#include "Sleeper.h"
#include "properties.h"
#include "asideconfig.h"

#include <QtGui>

BuildC::BuildC(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb, Properties *p)
    : Build(projopts, compstat, stat, progsize, progbar, cb, p)
{
}

int  BuildC::runBuild(QString option, QString projfile, QString compiler)
{
    int rc = 0;

    projectFile = projfile;
    aSideCompiler = compiler;
    aSideCompilerPath = sourcePath(compiler);

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

    //checkAndSaveFiles();

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
            rc = QMessageBox::question(0,
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
            int rc = QMessageBox::question(0,
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
            if(compileStatus->toPlainText().indexOf("error:",0, Qt::CaseInsensitive) > 0) {
                compileStatus->appendPlainText("Click error or warning messages above to debug.\n");
            }
            if(compileStatus->toPlainText().indexOf("undefined reference",0, Qt::CaseInsensitive) > 0) {
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
            if(compileStatus->toPlainText().indexOf("overflowed by", 0, Qt::CaseInsensitive) > 0) {
                compileStatus->appendPlainText("Your program is too big for the memory model selected in the project.");
                cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
                compileStatus->setTextCursor(cur);
                return rc;
            }
            if(compileStatus->toPlainText().indexOf("Error: Relocation overflows", 0, Qt::CaseInsensitive) > 0) {
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

int  BuildC::runCOGC(QString name, QString outext)
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
    rc = startProgram(compstr, sourcePath(projectFile), args);
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

int  BuildC::runBstc(QString spinfile)
{
    int rc = 0;

    //getApplicationSettings();
    if(checkCompilerInfo()) {
        return -1;
    }

    QStringList args;
    args.append("-c");

    /* run the bstc program */
    QString spin = properties->getSpinCompilerStr();
    QString comp = spin.mid(spin.lastIndexOf("/")+1);

    QDir libdir;

    if((comp.compare("spin",Qt::CaseInsensitive) == 0) ||
       (comp.compare("spin.exe",Qt::CaseInsensitive) == 0)) {
        // Roy's compiler always makes a .binary
        if(libdir.exists(properties->getSpinLibraryStr())) {
            args.append("-I");
            args.append(properties->getSpinLibraryStr());
        }
    }
    else {
        /* other compiler options */
        if(projectOptions->getSpinCompOptions().length()) {
            QStringList complist = projectOptions->getSpinCompOptions().split(" ",QString::SkipEmptyParts);
            foreach(QString compopt, complist) {
                args.append(compopt);
            }
        }

        // BSTC needs to be told to make a .binary
        if(libdir.exists(properties->getSpinLibraryStr())) {
            args.append("-L");
            args.append(properties->getSpinLibraryStr());
        }
    }

    args.append(spinfile); // using shortname limits us to files in the project directory.
    rc = startProgram(comp, sourcePath(projectFile), args);

    return rc;
}

int  BuildC::runCogObjCopy(QString datfile, QString tarfile)
{
    int rc = 0;

    //getApplicationSettings();
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

int  BuildC::runObjCopyRedefineSym(QString oldsym, QString newsym, QString file)
{
    int rc = 0;

    //getApplicationSettings();
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

int  BuildC::runObjCopy(QString datfile)
{
    int rc = 0;

    //getApplicationSettings();
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

int  BuildC::runGAS(QString gasfile)
{
    int rc = 0;

    //getApplicationSettings();
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

int  BuildC::runPexMake(QString fileName)
{
    int rc = 0;

    //getApplicationSettings();
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

int  BuildC::runAR(QStringList copts, QString libname)
{
    int rc;
    QString compstr;
    QStringList args;
    args.append("rs");
    args.append(libname);

    foreach(QString s, copts) {
        if(s.contains(".out",Qt::CaseInsensitive))
            continue;
        if(s.contains(".o",Qt::CaseInsensitive))
            args.append(s.mid(s.lastIndexOf("/")+1));
        if(s.contains(".cog",Qt::CaseInsensitive))
            args.append(s.mid(s.lastIndexOf("/")+1));
        if(s.contains(".ecog",Qt::CaseInsensitive))
            args.append(s.mid(s.lastIndexOf("/")+1));
    }

#if defined(Q_WS_WIN32)
    compstr = shortFileName(aSideCompiler);
#else
    compstr = aSideCompiler;
#endif
    compstr = compstr.replace("gcc","ar");

    /* remove old archive */
    if(QFile::exists(sourcePath(projectFile)+libname))
        QFile::remove(sourcePath(projectFile)+libname);

    /* this runs the archiver */
    rc = startProgram(compstr,sourcePath(projectFile),args);
    return rc;
}

int  BuildC::runCompiler(QStringList copts)
{
    int rc = 0;

    if(projectFile.isNull()) {
        QMessageBox mbox(QMessageBox::Critical, "Error No Project",
            "Please select a tab and press F4 to set main project file.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    //getApplicationSettings();
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

    /* this is intermediate compile */
    QStringList tlist;
    int inc = 0;
    int lib = 0;
    QStringList libs;

    // remove and save libs
    foreach (QString s, args) {
        if(s.contains("-l")) {
            args.removeOne(s);
            libs.append(s);
            continue;
        }
    }

    QString libname = QString(args.last());
    QString projname = libname;

    // remove main file. put back after library build
    args.removeLast();

    foreach (QString s, args) {

        if( s.contains(".spin",Qt::CaseInsensitive) ||
            s.contains(".a",Qt::CaseInsensitive) ||
            s.contains(".o",Qt::CaseInsensitive) ||
            s.contains(".cog",Qt::CaseInsensitive) ||
            s.contains(".ecog",Qt::CaseInsensitive) ||
            s.contains(".out",Qt::CaseInsensitive) ||
            s.contains(".elf",Qt::CaseInsensitive) ||
            s.contains("-o")
            )
            continue;

        // *.spin should never happen because of C build rules
        // remove .h files
        if(s.contains(".h",Qt::CaseInsensitive)) {
            args.removeOne(s);
            continue;
        }

        if(inc) {
            inc = 0;
            tlist.append(s);
        }
        else if(lib) {
            lib = 0;
            tlist.append(s);
        }
        else if(s.indexOf("-I") == 0) {
            inc++;
            tlist.append(s);
        }
        else if(s.indexOf("-L") == 0) {
            lib++;
            tlist.append(s);
        }
        else if(s.indexOf("-") == 0) {
            tlist.append(s);
        }
        else if(s.compare(".") != 0) {
            if(!tlist.contains("-c"))
                tlist.append("-c");
            tlist.append(s);
            args.removeOne(s);
            s = s.mid(s.lastIndexOf("/")+1);
            args.append(s.mid(0,s.indexOf("."))+".o");
            rc = startProgram(compstr,sourcePath(projectFile),tlist);
            tlist.removeLast();
            if(rc != 0)
                return rc;
        }
    }

    /* let's make a library after compiling the program so we can use .o from save-temps
     */
    libname = libname.mid(0,libname.lastIndexOf(".")) + ".a";
    if(projectOptions->getMakeLibrary().isEmpty() != true)
    {

        QString projobj = projname;
        projobj.replace(".c",".o");
        if(args.contains(projobj)) {
            args.removeOne(projobj);
        }
        this->runAR(args, sourcePath(projectFile)+libname); //QString("lib"+libname));

        foreach(QString s, args) {
            if(s.contains(".out",Qt::CaseInsensitive))
                continue;
            if(s.contains(".o",Qt::CaseInsensitive))
                args.removeOne(s);
            if(s.contains(".cog",Qt::CaseInsensitive))
                args.removeOne(s);
            if(s.contains(".ecog",Qt::CaseInsensitive))
                args.removeOne(s);
        }
    }

    // add main file back
    args.append(projname);

    // add libs back
    foreach(QString s, libs) {
        if(libname.contains(s.mid(2))) {
            if(projectOptions->getMakeLibrary().isEmpty() != true)
                args.append(s);
            continue;
        }
        args.append(s);
    }

    // this is the final compile/link
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
    QString ssize = QString(tr("Code Size")+" %L1 "+tr("bytes")+" (%L2 "+tr("total")+")").arg(codeSize).arg(memorySize);
    programSize->setText(ssize);

    return rc;
}

/*
 * make debug info for a .c file
 */
int BuildC::makeDebugFiles(QString fileName, QString projfile, QString compiler)
{
    projectFile = projfile;
    aSideCompiler = compiler;
    aSideCompilerPath = sourcePath(compiler);

    if(fileName.length() == 0)
        return -1;

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

    if(projectFile.isNull()) {
        QMessageBox mbox(QMessageBox::Critical, "Error. No Project",
            "Please select a tab and press F4 to set main project file.", QMessageBox::Ok);
        mbox.exec();
        return -1;
    }

    if(fileName.contains(FILELINK))
        fileName = fileName.mid(fileName.indexOf(FILELINK)+QString(FILELINK).length());

    //getApplicationSettings();
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

QStringList BuildC::getCompilerParameters(QStringList copts)
{
    QStringList args;
    getCompilerParameters(copts, &args);
    return args;
}

int BuildC::getCompilerParameters(QStringList copts, QStringList *args)
{
    // use the projectFile instead of the current tab file
    //QString srcpath = sourcePath(projectFile);

    //portName = cbPort->itemText(cbPort->currentIndex());
    //boardName = cbBoard->itemText(cbBoard->currentIndex());

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
    args->append(".");
    args->append("-L");
    args->append(".");

    if(projectOptions->getWarnAll().length())
        args->append(projectOptions->getWarnAll());
    if(projectOptions->get32bitDoubles().length())
        args->append(projectOptions->get32bitDoubles());
    if(projectOptions->getExceptions().length())
        args->append(projectOptions->getExceptions());
    if(projectOptions->getNoFcache().length())
        args->append(projectOptions->getNoFcache());

    if(projectOptions->getSimplePrintf().length() > 0) {
        /* don't use simple printf flag for COG model programs. */
        if(model.contains("cog",Qt::CaseInsensitive) == true) {
            this->compileStatus->insertPlainText(tr("Ignoring")+" \"Simple printf\""+tr(" flag in COG mode program.")+"\n");
            this->compileStatus->moveCursor(QTextCursor::End);
        }
        else if(projectOptions->getTinyLib().length() > 0) {
            this->compileStatus->insertPlainText(tr("Ignoring")+" \"Simple printf\""+tr(" flag in a program using -ltiny.")+"\n");
            this->compileStatus->moveCursor(QTextCursor::End);
        }
        else {
            args->append(projectOptions->getSimplePrintf());
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
        else if (parm.indexOf(".cfg",0, Qt::CaseInsensitive) > -1){
            // don't append .cfg parameter
        }
        else {
            args->append(parm);
        }
    }

    /* libraries */
    if(projectOptions->getTinyLib().length()) {
        if(model.contains("cog",Qt::CaseInsensitive) == true) {
            this->compileStatus->insertPlainText(tr("Ignoring")+" \"-ltiny\""+tr(" flag in COG mode programs.")+"\n");
            this->compileStatus->moveCursor(QTextCursor::End);
        }
        else  if(projectOptions->getMathLib().length()) {
            this->compileStatus->insertPlainText(tr("Ignoring")+" \"-ltiny\""+tr(" flag in -lm floating point programs.")+"\n");
            this->compileStatus->moveCursor(QTextCursor::End);
        }
        else {
            args->append(projectOptions->getTinyLib());
        }
    }
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

void BuildC::appendLoaderParameters(QString copts, QString projfile, QStringList *args)
{
    /* if propeller-load parameters -l or -z in copts, don't append a.out */
    if((copts.indexOf("-l") > 0 || copts.indexOf("-z") > 0) == false)
        args->append("a.out");

    QStringList olist = copts.split(" ",QString::SkipEmptyParts);
    for (int n = 0; n < olist.length(); n++)
        args->append(olist[n]);

    //qDebug() << args;
}
