#ifndef ASIDECONFIG_H
#define ASIDECONFIG_H

#include <QtGui>
#include "asideboard.h"

class ASideConfig
{
public:
    ASideConfig();
    ~ASideConfig();

    int         loadBoards(QString filePath);
    ASideBoard *newBoard(QString name);
    void        deleteBoardByName(QString name);

    QStringList getBoardNames();
    ASideBoard *getBoardByName(QString name);
    ASideBoard *getBoardData(QString name);


private:
    QString             filePath;
    QStringList         boardNames;
    QList<ASideBoard*> *boards;
};

#endif // ASIDECONFIG_H
