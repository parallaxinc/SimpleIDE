#include "asideboard.h"

#include <string>

const QString ASideBoard::clkmode = "clkmode";
const QString ASideBoard::pllmode = "pllmode";
const QString ASideBoard::clkfreq = "clkfreq";
const QString ASideBoard::baudrate = "baudrate";
const QString ASideBoard::reset = "reset";
const QString ASideBoard::rxpin = "rxpin";
const QString ASideBoard::txpin = "txpin";
const QString ASideBoard::tvpin = "tvpin";
const QString ASideBoard::textseg = "text";
const QString ASideBoard::dataseg = "data";
const QString ASideBoard::flashsize = "flash-size";
const QString ASideBoard::ramsize = "ram-size";
const QString ASideBoard::cachedriver = "cache-driver";
const QString ASideBoard::cachesize = "cache-size";
const QString ASideBoard::cacheparam1 = "cache-param1";
const QString ASideBoard::cacheparam2 = "cache-param2";

ASideBoard::ASideBoard()
{
    propNames = new QStringList();

    propNames->append("clkmode");
    propNames->append("clkfreq");
    propNames->append("baudrate");
    propNames->append("reset");
    propNames->append("rxpin");
    propNames->append("txpin");
    propNames->append("tvpin");
    propNames->append("text");
    propNames->append("data");
    propNames->append("flash-size");
    propNames->append("ram-size");
    propNames->append("cache-driver");
    propNames->append("cache-size");
    propNames->append("cache-param1");
    propNames->append("cache-param2");
}

ASideBoard::~ASideBoard()
{
    propNames->clear();
    delete propNames;
}

QString ASideBoard::get(QString property)
{
    if(propHash.contains(property))
        return propHash[property];
    return "";
}

void ASideBoard::set(QString prop, QString value)
{
    value = value.toUpper();

    // use combobox at GUI level for validation.
    // clkmode = value;
    if (prop.indexOf("clkfreq") > -1)
    {
        int intval = value.toLong();
        if (intval > 0)   // must be a positive number
            propHash[prop] = value;
    }
    // use combobox at GUI level for validation.
    // baudrate = value;
    else if (prop.indexOf("rxpin") > -1)
    {
        int intval = value.toInt();
        if ((intval > -1 && intval < 32) && (intval != 28) && (intval != 29))
        {
            propHash[prop] = value;
        }
    }
    else if (prop.indexOf("txpin") > -1)
    {
        int intval = value.toInt();
        if ((intval > -1 && intval < 32) && (intval != 28) && (intval != 29))
        {
            propHash[prop] = value;
        }
    }
    else if (prop.indexOf("tvpin") > -1)
    {
        int intval = value.toInt();
        if ((intval > -1 && intval < 32) && (intval != 28) && (intval != 29))
        {
            propHash[prop] = value;
        }
    }
    // use combobox at GUI level for validation.
    // flash or hub
    // text = value;
    // use combobox at GUI level for validation.
    // ram or hub
    // data = value;
    else
    {
        propHash[prop] = value;
    }
}

QString ASideBoard::getBoardName()
{
    return boardName;
}

void ASideBoard::setBoardName(QString name)
{
    boardName = name;
}

QString ASideBoard::getFormattedConfig()
{
    QString str = "[";
    str.append(boardName);
    str.append("]\n"); // endline translated by device IO
    QStringList *al = getAll();
    for(int n = 0; n < al->count(); n++)
    {
        QString s = (QString) al->at(n);
        if(s.length() > s.indexOf(": ")+2)
        {
            str.append("    ");
            str.append(s);
            str.append("\n");
        }
    }
    delete al;

    str.append("\n");
    return str;
}

QStringList *ASideBoard::getAll()
{
    if(propHash.isEmpty())
        return (QStringList*) NULL;

    QStringList *al = new QStringList();
    al->append("clkfreq: " + get("clkfreq"));
    al->append("clkmode: " + get("clkmode"));
    al->append("baudrate: " + get("baudrate"));
    al->append("reset: " + get("reset"));
    al->append("rxpin: " + get("rxpin"));
    al->append("txpin: " + get("txpin"));
    al->append("tvpin: " + get("tvpin"));
    al->append("text: " + get("text"));
    al->append("data: "+ get("data"));
    al->append("flash-size: "+ get("flash-size"));
    al->append("ram-size: " + get("ram-size"));
    al->append("cache-driver: " + get("cache-driver"));
    al->append("cache-size: " + get("cache-size"));
    al->append("cache-param1: " + get("cache-param1"));
    al->append("cache-param2: " + get("cache-param2"));
    return al;
}

int ASideBoard::parseConfig(QString file)
{
    int propCount = -1;

    QStringList lines = file.split("\n");
    int propNames_length = propNames->size();
    propHash.clear();
    for (int n = 0; n < propNames_length; n++)
        propHash.insert(propNames->at(n), "");

    try
    {
        int lines_length =  lines.count();
        for (int n = 0; n < lines_length; n++)
        {
            QString s = lines.at(n);
            if (s.indexOf('#') > -1)
                s = s.mid(0, s.indexOf('#'));
            s = s.trimmed();
            QString p = s;
            int colon = p.indexOf(":");
            if (colon > -1)
            {
                p = p.mid(0, colon);
                s = s.mid(colon + 1);
                s = s.trimmed();
                set(p, s);
                propCount++;
            }
        }
    }
    catch (int exception)
    {
        return 0;
    }
    return propCount+1;
}

