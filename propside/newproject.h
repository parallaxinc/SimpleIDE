#ifndef NEWPROJECT_H
#define NEWPROJECT_H

#include <QtGui>

class NewProject : public QDialog
{
    Q_OBJECT
public:
    explicit NewProject(QWidget *parent = 0);
    virtual ~NewProject();

    QString getCurrentPath();

    QString getName();
    QString getPath();

signals:

public slots:
    void nameChanged();
    void browsePath();
    void accept();
    void reject();
    void showDialog();

private:
    QString projName;
    QString projPath;

    QLineEdit   *name;
    QLineEdit   *path;
    QString     mypath;

    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // NEWPROJECT_H
