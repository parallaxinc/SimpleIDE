#include "help.h"
#include "properties.h"

Help::Help()
{
}

Help::~Help()
{
}

void Help::show(QString path, QString text)
{
    QString address = "file:///"+path+"/index.html";
    if(text.isEmpty()) {
        QDesktopServices::openUrl(QUrl(address));
        return;
    }
    QString tagtext = "\">"+text+"</a>";
    QDir dir(path);
    QStringList filter(QString("*.html"));
    QStringList entryList = dir.entryList(filter);
    foreach(QString name, entryList) {
        QFile file(path+"/"+name);
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
                    address = "file:///"+path+"/"+s;
                    QUrl url(address);
                    if(url.isValid() != true)
                        qDebug() << url.errorString();
                    QDesktopServices::openUrl(url);
                    return;
                }
            }
        }
    }
    QDesktopServices::openUrl(QUrl(address));
}
