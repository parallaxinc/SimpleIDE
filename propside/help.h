#ifndef HELP_H
#define HELP_H
#include <QtGui>

class Help : public QDialog
{
    Q_OBJECT;
public:
    Help();
    ~Help();
    void show(QString path, QString text);
};

#endif // HELP_H
