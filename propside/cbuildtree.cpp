#include "cbuildtree.h"
#include <QtWidgets>

CBuildTree::CBuildTree(const QString &shortFileName, QObject *parent)
    : TreeModel(shortFileName, parent)
{

}

/*
 * Get rows as a QStringList
 */
QStringList CBuildTree::getRowList()
{
    QStringList list;
    int rows = this->rowCount();
    QModelIndex none;
    for(int n = 0; n < rows; n++) {
        QVariant qv = this->data(this->index(n,0,none),Qt::DisplayRole);
        QString s;
        if(qv.canConvert(QVariant::String)) {
            s = qv.toString();
            list.append(s);
        }
    }
    return list;
}

/*
 * this should be part of a child class, but I'm lazy right now
 */
void CBuildTree::addRootItem(QString text)
{
    QList<QVariant> clist;
    clist << text.toLatin1();
    if(!isDuplicate(rootItem, text))
        rootItem->appendChild(new TreeItem(clist, rootItem));
}


/*
 * this should be part of a child class, but I'm lazy right now
 */
void CBuildTree::aSideIncludes(QString &filePath, QString &incPath, QString &separator,QString &text, bool root)
{
    QString inc, cap, s;
    QStringList st = text.split('\n');
    QRegExp rx("(include) ([^\n]*)");

    rx.setCaseSensitivity(Qt::CaseInsensitive);

    int len = st.length();
    for(int n = 0; n < len; n++)
    {
        s = st.at(n);
        int gotit = rx.indexIn(s);
        if(gotit > -1) {
            QList<QVariant> clist;
            inc = rx.cap(1);
            cap = rx.cap(2);
            if(cap != "" && cap.indexOf("\"") > -1) {
                QStringList caps = cap.split("\"");
                cap = caps.at(1);
                cap = cap.trimmed();
                clist << cap;
                if(!isDuplicate(rootItem, cap))
                    rootItem->appendChild(new TreeItem(clist, rootItem));
            }

            QString newPath = filePath.mid(0,(filePath.lastIndexOf(separator)+1))+cap;
            QString newInc = incPath+cap;
            if(QFile::exists(newPath) == true)
            {
                QString filename = newPath;
                QFile myfile(filename);
                if (myfile.open(QFile::ReadOnly | QFile::Text))
                {
                    text = myfile.readAll();
                    myfile.close();
                    aSideIncludes(filename, incPath, separator, text);
                }
            }
            else if(QFile::exists(newInc) == true)
            {
                QString filename = newInc;
                QFile myfile(filename);
                if (myfile.open(QFile::ReadOnly | QFile::Text))
                {
                    text = myfile.readAll();
                    myfile.close();
                    aSideIncludes(filename, incPath, separator, text);
                }
            }
        }
    }
}

/*
 * this should be part of a child class, but I'm lazy right now
 */
void CBuildTree::aSideIncludes(QString &text)
{
    QString inc, cap, s;
    QStringList st = text.split('\n');
    QRegExp rx("(include) ([^\n]*)");
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    int len = st.length();
    for(int n = 0; n < len; n++)
    {
        s = st.at(n);
        int gotit = rx.indexIn(s);
        if(gotit > -1) {
            QList<QVariant> clist;
            inc = rx.cap(1);
            cap = rx.cap(2);
            if(cap != "" && cap.indexOf("\"") > -1) {
                QStringList caps = cap.split("\"");
                cap = caps.at(1);
                cap = cap.trimmed();
                clist << cap;
                if(!isDuplicate(rootItem, cap))
                    rootItem->appendChild(new TreeItem(clist, rootItem));
            }
        }
    }
}

/*
 * this should be part of a child class, but I'm lazy right now
 */
void CBuildTree::addFileReferences(QString &filePath, QString &incPath, QString &separator, QString &text, bool root)
{
    QString inc, cap, s;
    QStringList st = text.split('\n');
    QRegExp rx("(include) ([^\n]*)");
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    QRegExp dx("def ([^\n]*)");
    dx.setCaseSensitivity(Qt::CaseInsensitive);

    int len = st.length();
    for(int n = 0; n < len; n++)
    {
        s = st.at(n);
        int gotdef = dx.indexIn(s);

        if(gotdef > -1) {
            QList<QVariant> clist;
            inc = dx.cap(0);
            if(!root)
                inc = "  " + inc;
            clist << inc;
            if(!isDuplicate(rootItem, inc))
                rootItem->appendChild(new TreeItem(clist, rootItem, filePath));
        }

        int gotit = rx.indexIn(s);
        if(gotit > -1) {
            QList<QVariant> clist;
            inc = rx.cap(1);
            cap = rx.cap(2);
            if(cap != "" && cap.indexOf("\"") > -1) {
                QStringList caps = cap.split("\"");
                cap = caps.at(1);
                cap = cap.trimmed();
                clist << cap;
                if(!isDuplicate(rootItem, cap))
                    rootItem->appendChild(new TreeItem(clist, rootItem));
            }

            QString newPath = filePath.mid(0,(filePath.lastIndexOf(separator)+1));
            QFile file;
            if(file.exists(newPath+cap))
            {
                QString filename = newPath+cap;
                QFile myfile(filename);
                if (myfile.open(QFile::ReadOnly | QFile::Text))
                {
                    text = myfile.readAll();
                    myfile.close();
                    addFileReferences(filename, incPath, separator, text);
                }
            }
            else if(file.exists(incPath+cap))
            {
                QString filename = incPath+cap;
                QFile myfile(filename);
                if (myfile.open(QFile::ReadOnly | QFile::Text))
                {
                    text = myfile.readAll();
                    myfile.close();
                    addFileReferences(filename, incPath, separator, text);
                }
            }
        }
    }
}

