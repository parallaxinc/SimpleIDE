#ifndef RESCUEDIALOG_H
#define RESCUEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QString>

class RescueDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RescueDialog(QWidget *parent = 0);
    void setEditText(QString text);

signals:

public slots:

private:
    QVBoxLayout *layout;
    QLabel *label;
    QPlainTextEdit *edit;
};

#endif // RESCUEDIALOG_H
