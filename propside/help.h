#ifndef HELP_H
#define HELP_H

#include <QtWidgets>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QDialog>

class Help : public QDialog
{
    Q_OBJECT
public:
    Help();
    ~Help();
    void show(QString path, QString text);

private:
    void openAddress(QString path, QString address);

    QVBoxLayout *blay;
    QTextBrowser *bedit;
    QDialogButtonBox *btnBox;

public slots:
    void accept();
};

#endif // HELP_H
