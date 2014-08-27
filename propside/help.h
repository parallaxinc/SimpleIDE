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

#ifndef HELP_H
#define HELP_H

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDialog>
#include <QObject>
#include <QToolButton>
#include <QLineEdit>
#include <QComboBox>

class Help : public QDialog
{
    Q_OBJECT

public:
    Help();
    ~Help();
    void show(QString path, QString text);

private:
    void addToolButton(QToolBar *bar, QToolButton *btn, QString imgfile, const char *signal = 0, const char *slot = 0);
    void openAddress(QString address);

    QVBoxLayout *blay;
    QToolBar    *btool;
    QTextBrowser *bedit;

    QStringList *history;
    int          hisIndex;
    QToolButton *btnBack;
    QToolButton *btnFwd;
    QComboBox   *cbHelpMode;
    QLineEdit   *leHelpSearch;

    QString     homeAddress;

public slots:
    void accept();
    void home();
    void back();
    void forward();
    void index();
    void search();
    void helpModeChanged(int n);
    void helpSearchChanged(QString text);
    void editChanged(QUrl url);
};

#endif // HELP_H
