#ifndef HINTDIALOG_H
#define HINTDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class HintDialog;
}

class HintDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit HintDialog(QString tag, QString hint, QWidget *parent = 0);
    ~HintDialog();
    int exec();

    static void hint(QString tag, QString hint, QWidget *parent = 0);

private slots:
    void on_okButton_clicked();
    
private:
    Ui::HintDialog *ui;
    QSettings settings;
    QString key;
};

#endif // HINTDIALOG_H
