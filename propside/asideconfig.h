#ifndef ASIDECONFIG_H
#define ASIDECONFIG_H

#include <QtGui>
#include "asideboard.h"

class ASideConfig
{
public:
    static const QString SdRun;
    static const QString SdLoad;
    static const QString Serial;
    static const QString IDE;
    static const QString SubDelimiter;
    static const QString UserDelimiter;

    ASideConfig();
    ~ASideConfig();

    int         loadBoards(QString filePath);
    int         addBoards(QString filePath);
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
