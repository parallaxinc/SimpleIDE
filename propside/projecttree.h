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
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    bool mouseRightClick;
    int  keyPressed;

};

#endif // PROJECTTREE_H
