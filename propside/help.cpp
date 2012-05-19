#include "help.h"

Help::Help()
{
    lay = new QVBoxLayout(this);
    webview = new QWebView(this);
    butbox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(butbox, SIGNAL(accepted()), this, SLOT(accept()));
    lay->addWidget(webview);
    lay->addWidget(butbox);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setLayout(lay);
}

Help::~Help()
{
    delete webview;
}

void Help::show(QString path, QString text)
{
    QString address = "file:///"+path+"/index.html";
    QString tagtext = "\">"+text+"</a>";
    QDir dir(path);
    QStringList filter(QString("*.html"));
    foreach(QString name, dir.entryList(filter)) {
        QFile file(path+"/"+name);
        QString fileText;
        if(file.open(QFile::ReadOnly)) {
            fileText = file.readAll();
            if (fileText.length() > 0 && fileText.indexOf(tagtext)) {
                QStringList list = fileText.split(tagtext);
                if(list.length() > 1) {
                    QString s = list[0];
                    s = s.mid(s.lastIndexOf("href=\"")+6);
                    s = s.left(s.indexOf(tagtext));
                    address = "file:///"+path+"/"+s;
                    file.close();
                    break;
                }
            }
            file.close();
        }
    }
    webview->load(QUrl(address));
    exec();
}

void Help::accept()
{
    done(QDialog::Accepted);
}
