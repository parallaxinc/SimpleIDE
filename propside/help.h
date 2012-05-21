#ifndef HELP_H
#define HELP_H
#include <QtGui>
#include <QtWebKit>

class Help : public QDialog
{
    Q_OBJECT;
public:
    Help();
    ~Help();
    void show(QString path, QString text);

public slots:
    void accept();

private:
    QVBoxLayout         *lay;
    QComboBox           *cbAddress;
    QDialogButtonBox    *butbox;
    QWebView            *webview;
};

#endif // HELP_H
