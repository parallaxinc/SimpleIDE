#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QtGui>

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QString landing, QWidget *parent = 0);
    void    show();
    void    exec();

private:
    QCheckBox *showSplashStartCheckBox;

signals:

public slots:
    void accept();
    void reject();
};

#endif // ABOUTDIALOG_H
