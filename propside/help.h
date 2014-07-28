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
