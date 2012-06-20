#include "buildspin.h"
#include "Sleeper.h"

#include "properties.h"
#include "asideconfig.h"

BuildSpin::BuildSpin(ProjectOptions *projopts, QPlainTextEdit *compstat, QLabel *stat, QLabel *progsize, QProgressBar *progbar, QComboBox *cb)
    : Build(projopts, compstat, stat, progsize, progbar, cb)
{
}

int  BuildSpin::runBuild(QString option, QString projfile, QString compiler)
{
    int rc = -1;

    projectFile = projfile;
    aSideCompiler = compiler;
    aSideCompilerPath = sourcePath(compiler);

    return rc;
#if 0
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
#endif
}

int  BuildSpin::makeDebugFiles(QString fileName, QString projfile, QString compiler)
{
    projectFile = projfile;
    aSideCompiler = compiler;
    aSideCompilerPath = sourcePath(compiler);

    return 0;
}


int  BuildSpin::runBstc(QString spinfile)
{
    int rc = 0;

    //getApplicationSettings();
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

