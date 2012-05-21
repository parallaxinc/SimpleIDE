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
    QDesktopServices::openUrl(QUrl(address));
}
