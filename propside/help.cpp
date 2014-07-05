#include "help.h"
#include "properties.h"

Help::Help() : hisIndex(-1)
{
    blay = new QVBoxLayout();
    bedit = new QTextBrowser();
    //bedit->setOpenLinks(false);

    blay->addWidget(bedit);
    blay->setContentsMargins(1,1,1,1);

#ifdef ADD_HELP_INDEX_WIP
    btool = new QToolBar();
    blay->addWidget(btool);

    cbHelpMode = new QComboBox(btool);
    cbHelpMode->setEditable(false);
    cbHelpMode->addItem(tr("Contents"));
    cbHelpMode->addItem(tr("Index"));
    cbHelpMode->addItem(tr("Search"));
    cbHelpMode->setCurrentIndex(0);

    connect(cbHelpMode,SIGNAL(currentIndexChanged(int)),this,SLOT(helpModeChanged(int)));

    leHelpSearch = new QLineEdit(btool);
    leHelpSearch->setPlaceholderText("Search Item");
    leHelpSearch->setMaximumWidth(120);
    leHelpSearch->setEnabled(false);
    connect(leHelpSearch,SIGNAL(textChanged(QString)),this,SLOT(helpSearchChanged(QString)));
    btool->addWidget(cbHelpMode);
    btool->addWidget(leHelpSearch);

    btnBack = new QToolButton(btool);
    btnFwd  = new QToolButton(btool);
    addToolButton(btool, btnBack, ":/images/back.png");
    connect(btnBack, SIGNAL(clicked()), this, SLOT(back()));
    addToolButton(btool, btnFwd, ":/images/forward.png");
    connect(btnFwd,  SIGNAL(clicked()), this, SLOT(forward()));

    btnBack->setEnabled(false);
    btnFwd->setEnabled(false);

    //connect(bedit,SIGNAL(sourceChanged(QUrl)),this,SLOT(editChanged(QUrl)));
#endif
    connect(bedit,SIGNAL(anchorClicked(QUrl)),this,SLOT(editChanged(QUrl)));

    setLayout(blay);
    setWindowFlags(Qt::Window);
    history = new QStringList();

    resize(800,400);
}

Help::~Help()
{
}

void Help::addToolButton(QToolBar *bar, QToolButton *btn, QString imgfile, const char *slot, const char *signal)
{
    const QSize buttonSize(24, 24);
    btn->setIcon(QIcon(QPixmap(imgfile.toLatin1())));
    btn->setMinimumSize(buttonSize);
    btn->setMaximumSize(buttonSize);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bar->addWidget(btn);
    if(signal != 0 && slot != 0) {
        connect(btn, signal, this, slot);
    }
    if(signal == 0 && slot != 0) {
        connect(btn, SIGNAL(clicked()), this, slot);
    }
}

void Help::accept()
{
    hide();
}

void Help::home()
{
}

void Help::back()
{
#ifdef ADD_HELP_INDEX_WIP
    if(hisIndex < 1) {
        btnBack->setEnabled(false);
        return;
    }
    QString s = history->at(hisIndex-1);
    hisIndex--;
    if(hisIndex < 1) {
        btnBack->setEnabled(false);
    }
    if(s.length() > 0) {
        openAddress(s);
        btnFwd->setEnabled(true);
    }
#endif
}

void Help::forward()
{
#ifdef ADD_HELP_INDEX_WIP
    if(hisIndex >= history->count()-1) {
        btnFwd->setEnabled(false);
        return;
    }
    QString s = history->at(hisIndex+1);
    hisIndex++;
    btnBack->setEnabled(true);
    if(s.length() > 0) {
        openAddress(s);
    }
    if(hisIndex == history->count()-1) {
        btnFwd->setEnabled(false);
    }
#endif
}

void Help::index()
{
}

void Help::search()
{
}

void Help::helpModeChanged(int n)
{
#ifdef ADD_HELP_INDEX_WIP
    if(n > 0) {
        leHelpSearch->setEnabled(true);
    } else {
        leHelpSearch->setEnabled(false);
    }
#endif
}

void Help::helpSearchChanged(QString text)
{
    if(text.length() > 0) {
    }
}

void Help::editChanged(QUrl url)
{
#ifdef ADD_HELP_INDEX_WIP
    QString title = windowTitle();
    if(title.compare(url.toLocalFile()) == 0)
        return;
    setWindowTitle(url.toLocalFile());
    if(hisIndex < 0) {
        history->append(url.toString());
        hisIndex = history->count()-1;
        btnBack->setEnabled(false);
        btnFwd->setEnabled(false);
    }
    else if(hisIndex == history->count()-1) {
        if(history->at(hisIndex).compare(url.toString()) != 0) {
            history->append(url.toString());
            hisIndex = history->count()-1;
            btnFwd->setEnabled(false);
        }
    }
    else {
        if(history->at(hisIndex).compare(url.toString()) != 0) {
            history->insert(hisIndex,url.toString());
        }
        btnFwd->setEnabled(true);
    }
    if(hisIndex > 0) {
        btnBack->setEnabled(true);
    }
    if(hisIndex < history->count()-1) {
        btnFwd->setEnabled(true);
    }
    openAddress(url.toString());
    qDebug() << "editChanged" << history->count() << hisIndex;
#else
    hide(); // The light-weight QTextBrowser doesn't handle some html.
    openAddress(url.toString());
#endif
}

void Help::openAddress(QString address)
{
    qDebug() << "openAddress()" << address;
    QUrl url(address);
    QString fileName = url.toLocalFile();
    setWindowTitle(fileName);

    if(QFile::exists(fileName)) {

        QString frag = url.fragment();
        QString css;

        if(frag.length() > 0) {
            //if(frag[0]=='#') frag = frag.mid(1);
            QString home = address.left(address.lastIndexOf("#"));
            QString libname = home.mid(home.lastIndexOf("/")+1);
            libname = libname.left(libname.indexOf("_"));
            home = "<h3><a href=\""+homeAddress+"\">Propeller C Simple Library</a> \""+libname+".h\" Symbol:</h3>";
            QString text;
            /* replace memdoc class with something else*/
            QRegExp rex("<a class=\"anchor\" id=\""+frag+"\"></a>\n.*<a class=\"anchor");
            rex.setCaseSensitivity(Qt::CaseInsensitive);
            rex.setMinimal(true);

            QRegExp rex2("<a class=\"anchor\" id=\""+frag+"\"></a>\n.*</div><!-- contents -->");
            rex2.setCaseSensitivity(Qt::CaseInsensitive);
            rex2.setMinimal(true);

            QFile file(fileName);
            if(file.open(QFile::ReadOnly)) {
                text = file.readAll();
                file.close();
                int pos = 0;
                bool found = false;
                while((pos = rex.indexIn(text,pos)) > -1) {
                    QString s = rex.cap();
                    s = s.left(s.lastIndexOf("<a class=\"anchor"));
                    if(s.contains(frag)) {
                        qDebug() << "cap" << rex.captureCount() << s;
                        QFile cssFile(":/images/helpfunction.css");
                        if(cssFile.open(QFile::ReadOnly)) {
                            css = cssFile.readAll();
                            cssFile.close();
                            css = "<head><style>"+css+"></style></head>";
                        }
                        text = "<html>"+css+"<body>"+home+""+s+"<p class=\"memdoc\">Parallax Propeller C Learning System</p></body></html>";
                        found = true;
                        break;
                    }
                    pos += rex.matchedLength();
                }
                if(!found) {
                    int pos = 0;
                    while((pos = rex2.indexIn(text,pos)) > -1) {
                        QString s = rex2.cap();
                        s = s.left(s.lastIndexOf("</div><!-- contents -->"));
                        if(s.contains(frag)) {
                            qDebug() << "cap" << rex2.captureCount() << s;
                            QFile cssFile(":/images/helpfunction.css");
                            if(cssFile.open(QFile::ReadOnly)) {
                                css = cssFile.readAll();
                                cssFile.close();
                                css = "<head><style>"+css+"></style></head>";
                            }
                            text = "<html>"+css+"<body>"+home+""+s+"<p class=\"memdoc\">Parallax Propeller C Learning System</p></body></html>";
                            break;
                        }
                        pos += rex2.matchedLength();
                    }
                }
            }

            bedit->setLineWrapMode(QTextBrowser::NoWrap);
            if(text.length() > 0) {
                qDebug() << text;
                bedit->setStyleSheet(css);
                bedit->setHtml(text);
                bedit->find(url.fragment());
                setModal(false);
                open();
            }
            else {
                QDesktopServices::openUrl(url);
            }
        }
        else {
            QDesktopServices::openUrl(url);
        }
    }
}

void Help::show(QString path, QString text)
{
    QString address = path+"Simple Libraries Index.html";
    homeAddress = "file:///"+address;
    if(text.isEmpty()) {
        //QDesktopServices::openUrl(QUrl(address));
        openAddress("file:///"+address);
        return;
    }
    QString tagtext = "\">"+text+"</a>";
    QString filetag = "<a href=";

    QFile mfile(address);
    QString fileText;
    QStringList fileList;

    QRegExp rcap("\".*\"");
    rcap.setMinimal(true);

    if(mfile.open(QFile::ReadOnly)) {
        fileText = mfile.readAll();
        mfile.close();
        /*
         * file made of *<a href=* and odd numbered splits have links.
         */
        if (fileText.length() > 0 && fileText.indexOf(filetag,Qt::CaseInsensitive)) {
            QStringList list = fileText.split(filetag);
            for(int n = 1; n < list.length(); n++) {
                QString s = list[n];
                QString ref;
                if(rcap.indexIn(s) > -1) {
                    ref = rcap.cap();
                    ref = ref.remove("\"");
                }
                if(ref.length() < 1) {
                    continue;
                }
                QString fs = QUrl("file:///"+path+ref).toLocalFile();
                if(QFile::exists(fs)) {
                    fileList.append(fs);
                }
            }
        }
    }

    foreach(QString name, fileList) {
        QFile file(name);

        if(file.open(QFile::ReadOnly)) {
            fileText = file.readAll();
            file.close();
            if (fileText.length() > 0 && fileText.indexOf(tagtext,Qt::CaseInsensitive) > 0) {
                QStringList list = fileText.split(tagtext);
                if(list.length() > 1) {
                    QString s = list[0];
                    s = s.mid(s.lastIndexOf("href=\"")+6);
                    s = s.trimmed();
                    QString mpath = name.left(name.lastIndexOf("/"));
                    address = mpath+"/"+s;
                    QUrl url(address, QUrl::TolerantMode);
                    if(url.isValid() != true)
                        qDebug() << url.errorString();
                    //QDesktopServices::openUrl(url);
                    openAddress("file:///"+address);
                    return;
                }
            }
        }
    }

    QString target = "target=\"_blank\">";
    QString libtag  = target+text;
    QRegExp rex(libtag+".*\.h<");
    rex.setCaseSensitivity(Qt::CaseInsensitive);
    rex.setMinimal(true);
    QFile lfile(address);
    if(lfile.open(QFile::ReadOnly)) {
        QString fileText = lfile.readAll();
        lfile.close();
        if (fileText.length() > 0 && fileText.indexOf(libtag,Qt::CaseInsensitive) > 0) {
            int pos = 0;
            while((pos = rex.indexIn(fileText,pos)) > -1) {
                QString s = rex.cap();
                s = s.left(s.lastIndexOf("<"));
                s = s.mid(target.length());
                qDebug() << "Search for" << text << "found" << s;
                if(s.compare(text+".h") == 0) {
                    s = fileText.mid(pos);
                    for(; pos > -1; pos--) {
                        QString href("href=\"");
                        s = fileText.mid(pos);
                        if(s.indexOf(href) == 0) {
                            s = s.mid(href.length());
                            s = s.mid(0, s.indexOf("\""));
                            s = s.trimmed();
                            if(s.endsWith("\"")) s = s.left(s.length()-1);
                            address = path+s;
                            openAddress("file:///"+address);
                            return;
                        }
                    }
                }
                pos += rex.matchedLength();
            }
#if 0
            QStringList list = fileText.split(libtag);
            if(list.length() > 1) {
                QString s = list[0];
                s = s.mid(s.lastIndexOf("href=\"")+6);
                s = s.trimmed();
                if(s.endsWith("\"")) s = s.left(s.length()-1);
                address = path+s;
                openAddress("file:///"+address);
                return;
            }
#endif
        }
    }

    // if nothing is found above, open an address with this
    //QDesktopServices::openUrl(QUrl("file:///"+address));
    openAddress("file:///"+address);
}

