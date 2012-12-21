#ifndef PROJECTOPTIONS_H
#define PROJECTOPTIONS_H

#include <QWidget>

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

    explicit ProjectOptions(QWidget *parent = 0);
    ~ProjectOptions();

    void clearOptions();

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

    QStringList getOptions();
    void        setOptions(QString s);

    /* spin stuff */

    QString getSpinCompOptions();
    void setSpinCompOptions(QString s);

    QStringList getSpinOptions();
    void        setSpinOptions(QString s);

public slots:
    void compilerChanged(QString comp);

private:
    QString     boardType;
    QVector<QWidget*> tabs;

signals:
    void compilerChanged();


};

#endif // PROJECTOPTIONS_H
