#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include <QtGui>

class ProjectTree : public QTreeView
{
Q_OBJECT
public:
    ProjectTree(QWidget *parent = 0);
    ~ProjectTree();

    bool rightClick(bool value);

protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    bool mouseRightClick;
    int  keyPressed;

signals:
    void deleteProjectItem();
};

#endif // PROJECTTREE_H
