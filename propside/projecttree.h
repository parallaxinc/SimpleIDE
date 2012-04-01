#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include <QtGui>

class ProjectTree : public QTreeView
{
public:
    ProjectTree(QWidget *parent = 0);
    bool rightClick(bool value);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    bool mouseRightClick;
};

#endif // PROJECTTREE_H
