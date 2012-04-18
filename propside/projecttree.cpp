#include "projecttree.h"

ProjectTree::ProjectTree(QWidget *parent) : QTreeView(parent)
{
    this->mouseRightClick = false;
    this->header()->setStretchLastSection(false);
    this->header()->setResizeMode(QHeaderView::ResizeToContents);
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
