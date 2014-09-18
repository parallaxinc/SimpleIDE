/*
 * This file is part of the Parallax Propeller SimpleIDE development environment.
 *
 * Copyright (C) 2014 Parallax Incorporated
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qtversion.h"

#include "projecttree.h"

ProjectTree::ProjectTree(QWidget *parent) : QTreeView(parent)
{
    this->mouseRightClick = false;
#ifdef QT5
    this->header()->setStretchLastSection(false);
    this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    this->header()->setStretchLastSection(true);
#endif
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
