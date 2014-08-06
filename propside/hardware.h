#ifndef HARDWARE_H
#define HARDWARE_H

#include <QDialog>
#include "ui_hardware.h"
#include "asideconfig.h"

namespace Ui {
    class Hardware;
}

class Hardware : public QDialog
{
    Q_OBJECT

public:
    explicit Hardware(QWidget *parent = 0);
    ~Hardware();

    void loadBoards();
    void saveBoards();

public slots:
    void accept();
    void reject();
    void boardChanged(int index);
    void deleteBoard();

private:
    void setBoardInfo(ASideBoard *board);
    void setDialogBoardInfo(ASideBoard *board);
    void setComboCurrent(QComboBox *cb, QString value);

    Ui::Hardware *ui;

    ASideConfig    *aSideConfig;
    QString         aSideCfgFile;
    QString         aSideSeparator;
};

#endif // HARDWARE_H
