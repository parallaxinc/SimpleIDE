#include "projecttree.h"

ProjectTree::ProjectTree(QWidget *parent) : QTreeView(parent)
{
    this->mouseRightClick = false;
    this->header()->setStretchLastSection(false);
    this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

ProjectTree::~ProjectTree()
{
}

void ProjectTree::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        this->mouseRightClick = true;
        emit showPopup();
    }
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
    if(keyPressed == Qt::Key_Delete)
        emit deleteProjectItem();
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
