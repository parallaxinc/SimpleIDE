#ifndef GDB_H
#define GDB_H

#include <QtCore>
#include "editor.h"
#include "PortListener.h"

class GDB : public QObject
{
    Q_OBJECT
public:
    explicit GDB(QVector<Editor*> *editors, PortListener *port, QObject *parent = 0);

signals:

public slots:
    void next();
    void step();
    void finish();
    void backtrace();
    void until();

private:
    QVector<Editor*>    *editors;
    PortListener        *port;
};

#endif // GDB_H
