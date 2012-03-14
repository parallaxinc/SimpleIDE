#include "gdb.h"

GDB::GDB(QVector<Editor*> *editors, PortListener *port, QObject *parent) :
    QObject(parent)
{
    this->editors = editors;
    this->port = port;
}

void GDB::next()
{

}

void GDB::step()
{

}

void GDB::finish()
{

}

void GDB::backtrace()
{

}

void GDB::until()
{

}


