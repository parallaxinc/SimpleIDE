#ifndef CBUILDTREE_H
#define CBUILDTREE_H

#include "treeitem.h"
#include "treemodel.h"

class CBuildTree : public TreeModel
{
public:

    CBuildTree(const QString &shortFileName, QObject *parent);

    void addRootItem(QString text);
    void aSideIncludes(QString &text);
    void aSideIncludes(QString &filePath, QString &incPath, QString &separator, QString &text, bool root = false);
    void addFileReferences(QString &filePath, QString &incPath, QString &separator, QString &text, bool root = false);

};

#endif // CBUILDTREE_H
