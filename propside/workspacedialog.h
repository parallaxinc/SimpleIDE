#ifndef WORKSPACEDIALOG_H
#define WORKSPACEDIALOG_H

#include "qtversion.h"

#include <QDialog>
#include <QObject>
#include <QString>

class WorkspaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkspaceDialog(QWidget *parent = 0);

    bool replaceDialog(QString folder, QString &selected);
    QString getWorkspace();

private:
    QLineEdit   leditFolder;
    QPushButton btnOk;
    QPushButton btnShowChange;
    QPushButton btnChange;

    QLabel      dialogLabel;
    QLabel      blankLabel;
    QLabel      changeLabel;

    QGridLayout grid;
    QVBoxLayout chgLayout;

    QString     workspace;

signals:

public slots:
    void accept();
    void changeFolder();
    void showChangeFolder();
};

#endif // WORKSPACEDIALOG_H
