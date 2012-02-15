#ifndef ASIDEBOARD_H
#define ASIDEBOARD_H

#include <QtGui>
#include <QHash>

class ASideBoard
{
public:
    ASideBoard();
    ~ASideBoard();

    QString   get(QString property);
    void        set(QString property, QString value);
    QString   getBoardName();
    void        setBoardName(QString name);
    QString   getFormattedConfig();
    QStringList *getAll();
    int         parseConfig(QString file);

    static const QString clkmode;
    static const QString pllmode;
    static const QString clkfreq;
    static const QString baudrate;
    static const QString rxpin;
    static const QString txpin;
    static const QString tvpin;
    static const QString textseg;
    static const QString dataseg;
    static const QString flashsize;
    static const QString ramsize;
    static const QString cachedriver;
    static const QString cachesize;
    static const QString cacheparam1;
    static const QString cacheparam2;

private:

    QString   boardName;
    QStringList *propNames;
    QStringList *clkModes;
    QStringList *baudRates;
    QStringList *dataRam;
    QStringList *textRam;
    QList<int>  *pinList;

    QHash<QString, QString> propHash;
};

#endif // ASIDEBOARD_H
