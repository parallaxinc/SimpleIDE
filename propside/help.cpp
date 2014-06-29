#include "help.h"
#include "properties.h"

Help::Help()
{
    blay = new QVBoxLayout();
    bedit = new QTextBrowser();
    blay->addWidget(bedit);
    blay->setContentsMargins(1,1,1,1);
    setLayout(blay);
    setWindowFlags(Qt::Window);
#if 0
    // no buttons
    btnBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(accept()));
    blay->addWidget(btnBox);
#endif
    resize(700,600);
}

Help::~Help()
{
}

void Help::accept()
{
    hide();
}

void Help::show(QString path, QString text)
{
    QString address = path+"Simple Libraries Index.html";
    if(text.isEmpty()) {
        //QDesktopServices::openUrl(QUrl(address));
        openAddress(path, address);
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
        QString fileText;
        if(file.open(QFile::ReadOnly)) {
            fileText = file.readAll();
            file.close();
            if (fileText.length() > 0 && fileText.indexOf(tagtext,Qt::CaseInsensitive)) {
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
                    openAddress(path, address);
                    return;
                }
            }
        }
    }

    QString libtag  = "target=\"_blank\">"+text;
    QFile lfile(address);
    if(lfile.open(QFile::ReadOnly)) {
        QString fileText = lfile.readAll();
        lfile.close();
        if (fileText.length() > 0 && fileText.indexOf(libtag,Qt::CaseInsensitive)) {
            QStringList list = fileText.split(libtag);
            if(list.length() > 1) {
                QString s = list[0];
                s = s.mid(s.lastIndexOf("href=\"")+6);
                s = s.trimmed();
                if(s.endsWith("\"")) s = s.left(s.length()-1);
                address = path+s;
                openAddress(path, address);
                return;
            }
        }
    }

    // if nothing is found above, open an address with this
    //QDesktopServices::openUrl(QUrl("file:///"+address));
    openAddress(path, address);
}

void Help::openAddress(QString path, QString address)
{
    qDebug() << "openAddress()" << path << address;
    QUrl url("file:///"+address);
    QString fileName = url.toLocalFile();
    setWindowTitle(fileName);
    bedit->setSource(url);
    if(QFile::exists(fileName)) {
        setModal(false);
        open();
    }
}
