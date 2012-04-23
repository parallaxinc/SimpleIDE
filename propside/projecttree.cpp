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

void ProjectTree::keyPressEvent(QKeyEvent *event)
{
    keyPressed = event->key();
    QTreeView::keyPressEvent(event);
}

void ProjectTree::keyReleaseEvent(QKeyEvent *event)
{
    if(keyPressed == Qt::Key_Delete) {
        // add emit signal here later to let keyboard delete entries
        // emit deleteItem();
    }
    QTreeView::keyReleaseEvent(event);
}
