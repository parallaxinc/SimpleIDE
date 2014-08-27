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

#ifndef PROJECTOPTIONS_H
#define PROJECTOPTIONS_H

#include <QWidget>
#include <QComboBox>

#include "asideconfig.h"

namespace Ui {
    class Project;
}

class ProjectOptions : public QWidget
{
    Q_OBJECT

private:
    Ui::Project *ui;

public:
    static const QString compiler;
    static const QString memtype;
    static const QString memTypeCOG;
    static const QString memTypeCMM;
    static const QString memTypeLMM;
    static const QString memTypeXMM;
    static const QString memTypeXMMC;
    static const QString memTypeXMMSINGLE;
    static const QString memTypeXMMSPLIT;
    static const QString optimization;
    static const QString cflags;
    static const QString lflags;
    static const QString board;

    static const QString C_COMPILER;
    static const QString CPP_COMPILER;
    static const QString SPIN_COMPILER;


    enum { TAB_OPT=0, TAB_C_COMP, TAB_C_LIB, TAB_SPIN_COMP };

    explicit ProjectOptions(QWidget *parent, QComboBox *boardType);
    ~ProjectOptions();

    void clearOptions();

    QStringList getMemModelList();
    QString getCompiler();
    QString getMemModel();
    QString getOptimization();

    QToolButton *getHardwareButton();
    QComboBox   *getHardwareComboBox();

    QString get32bitDoubles();
    QString getWarnAll();
    QString getNoFcache();
    QString getExceptions();
    QString getTinyLib();
    QString getMathLib();
    QString getPthreadLib();
    QString getSimplePrintf();
    QString getStripElf();
    QString getCompOptions();
    QString getLinkOptions();
    QString getBoardType();
    QString getMakeLibrary();
    QString getEnableGcSections();

    void setCompiler(QString s);
    void setMemModel(QString s);
    void setOptimization(QString s);

    void set32bitDoubles(bool s);
    void setWarnAll(bool s);
    void setNoFcache(bool s);
    void setExceptions(bool s);
    void setTinyLib(bool s);
    void setMathLib(bool s);
    void setPthreadLib(bool s);
    void setSimplePrintf(bool s);
    void setStripElf(bool s);
    void setCompOptions(QString s);
    void setLinkOptions(QString s);
    void setBoardType(QString s);
    void setMakeLibrary(bool s);
    void setEnableGcSections(bool s);

    QStringList getOptions();
    void        setOptions(QString s);

    /* spin stuff */

    QString getSpinCompOptions();
    void setSpinCompOptions(QString s);

    QStringList getSpinOptions();
    void        setSpinOptions(QString s);

    void        setMaximumWidth(int width);
    void        setMinimumWidth(int width);

public slots:
    void compilerChanged(QString comp);

private:
    QString     boardType;
    QVector<QWidget*> tabs;
    QComboBox   *cbBoard;

signals:
    void compilerChanged();


};

#endif // PROJECTOPTIONS_H
