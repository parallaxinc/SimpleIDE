#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include <QtWidgets>

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
    void showPopup();
};

#endif // PROJECTTREE_H
