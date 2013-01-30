#include "asideconfig.h"

const QString ASideConfig::SdRun  = "SdXMMC";
const QString ASideConfig::SdLoad = "SdLoad";
const QString ASideConfig::Serial = "Serial";
const QString ASideConfig::IDE = "IDE:";
const QString ASideConfig::SubDelimiter = ":";
const QString ASideConfig::UserDelimiter = "-";

ASideConfig::ASideConfig()
{
    boards = new QList<ASideBoard*>();
}

ASideConfig::~ASideConfig()
{
    boards->clear();
    delete boards;
}

void ASideConfig::deleteBoardByName(QString name)
{
    for(int n = 0; n < boardNames.count(); n++)
    {
        if(boardNames.at(n) == name)
        {
            boardNames.removeAt(n);
            boards->removeAt(n);
            break;
        }
    }
}

/*
 * This doesn't scale, but the number of boards is small for now.
 * Can easily make boards a QHash later.
 */
ASideBoard *ASideConfig::getBoardByName(QString name)
{
    for(int n = 0; n < boardNames.count(); n++)
    {
        if(boardNames.at(n) == name)
            return boards->at(n);
    }
    return NULL;
}

ASideBoard *ASideConfig::newBoard(QString name)
{
    boardNames.append(name);
    ASideBoard *board = new ASideBoard();
    board->setBoardName(name);
    return board;
}

int ASideConfig::loadBoards(QString filePath)
{
    QDir dir(filePath);
    if(!dir.exists())
        return 0;

    /* clear old data */
    for(int n = boards->count()-1; n > -1; n--)
    {
        ASideBoard *brd = boards->at(n);
        delete brd;
    }
    boards->clear();
    boardNames.clear();
    boardNames.append("NONE");

    addBoards(filePath);
}


int ASideConfig::addBoards(QString filePath)
{
    QDir dir(filePath);
    if(!dir.exists())
        return 0;

    QStringList filter;
    filter << "*.cfg";
    dir.setNameFilters(filter);
    dir.setSorting(QDir::Name);
    QStringList names = dir.entryList(filter);

    QString boardfile(filePath+"boards.txt");
    QString boardtxt("");
    if(QFile::exists(boardfile)) {
        QFile bd(boardfile);
        if(bd.open(QFile::ReadOnly | QFile::Text)) {
            boardtxt = bd.readAll();
            bd.close();
        }
    }

    foreach(QString name, names) {
        if(boardtxt.isEmpty() == false) {
            if(boardtxt.contains(name, Qt::CaseInsensitive) == false)
                continue;
        }
        QFile fileReader(filePath+name);
        if (!fileReader.open(QIODevice::ReadOnly))
            return 0;
        QString file = fileReader.readAll();
        name = name.mid(0,name.lastIndexOf("."));

        /* add default board */
        ASideBoard *board = newBoard(name.toUpper());
        if (board->parseConfig(file))
            boards->append(board);

        QStringList list = file.split("\n",QString::SkipEmptyParts);
        QStringList sublist;

        /* find board subtypes */
        foreach(QString s, list) {
            if(s.contains("[",Qt::CaseInsensitive)) {
                s = s.mid(s.indexOf("[")+1);
                if(s.contains("]") == false) {
                    /* badly formed board type. Ignore it */
                    continue;
                }
                s = s.mid(0,s.indexOf("]"));
                s = s.trimmed();
                if(s.compare(name,Qt::CaseInsensitive) != 0) {
                    sublist.append(name.toUpper());
                    ASideBoard *board = newBoard(name.toUpper()+ASideConfig::SubDelimiter+s.toUpper());
                    if (board->parseConfig(file))
                        boards->append(board);
                }
            }
        }

        /* add IDE control variants */
        if(file.contains(ASideConfig::IDE,Qt::CaseInsensitive)) {
            foreach(QString s, list) {
                if(s.contains(ASideConfig::IDE,Qt::CaseInsensitive)) {
                    s = s.mid(s.indexOf(ASideConfig::IDE)+ASideConfig::IDE.length());
                    s = s.trimmed();
                    ASideBoard *board = newBoard(name.toUpper()+ASideConfig::UserDelimiter+s.toUpper());
                    if (board->parseConfig(file))
                        boards->append(board);
                }
            }
        }

        fileReader.close();
    }
    return boards->count();
}

#if REMOVE_CRUFT
/*
 * old way
 */
int ASideConfig::loadBoards_v0_2_0(QString filePath)
{
    QFile fileTest;
    if(!fileTest.exists(filePath))
        return 0;

    QFile fileReader(filePath);
    if (!fileReader.open(QIODevice::ReadOnly))
        return 0;

    /* clear old data */
    for(int n = boards->count()-1; n > -1; n--)
    {
        ASideBoard *brd = boards->at(n);
        delete brd;
    }
    boards->clear();
    boardNames.clear();

    QString file = fileReader.readAll();

    QString bstr;
    QStringList barr = file.split("[");

    for (int n = 0; n < barr.count(); n++)
    {
        bstr = barr.at(n);
        if (bstr.length() == 0)
            continue;

        // get board name
        bstr = barr.at(n).mid(0, barr.at(n).indexOf(']'));
        bstr = bstr.trimmed().toUpper();
        ASideBoard *board = newBoard(bstr);

        // the rest of the board definition parsed here
        bstr = barr.at(n);
        bstr = barr.at(n).mid(barr.at(n).indexOf(']')+1);
        bstr = bstr.trimmed();

        // parse the rest in Board class
        if (board->parseConfig(bstr))
            boards->append(board);
    }
    fileReader.close();
    return boards->count();
}
#endif

QStringList ASideConfig::getBoardNames()
{
    return boardNames;
}

ASideBoard* ASideConfig::getBoardData(QString name)
{
    int length = boards->count();
    if(length == 0)
        return NULL;
    for(int n = 0; n < length; n++)
    {
        if(boards->at(n)->getBoardName() == name)
            return boards->at(n);
    }
    return NULL;
}
