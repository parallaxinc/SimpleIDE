#include "projecttree.h"

ProjectTree::ProjectTree(QWidget *parent) : QTreeView(parent)
{
    this->mouseRightClick = false;
}

void ProjectTree::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
        this->mouseRightClick = true;
    QTreeView::mousePressEvent(event);
}

bool ProjectTree::rightClick(bool value)
{
    bool retval = this->mouseRightClick;
    this->mouseRightClick = value;
    return retval;
}
