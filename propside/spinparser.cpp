#include "spinparser.h"

#define KEY_ELEMENT_SEP ":"

SpinParser::SpinParser()
{
    setKind(&SpinKinds[SpinParser::K_NONE],     FALSE,'n', "none", "none"); // place-holder only
    setKind(&SpinKinds[SpinParser::K_CONST],    TRUE, 'c', "constant", "constants");
    setKind(&SpinKinds[SpinParser::K_PUB],      TRUE, 'p', "public", "methods");
    setKind(&SpinKinds[SpinParser::K_PRI],      TRUE, 'f', "private", "functions");
    setKind(&SpinKinds[SpinParser::K_OBJECT],   TRUE, 'o', "obj", "objects");
    setKind(&SpinKinds[SpinParser::K_TYPE],     TRUE, 't', "type", "types");
    setKind(&SpinKinds[SpinParser::K_VAR],      TRUE, 'v', "var", "variables");
    setKind(&SpinKinds[SpinParser::K_DAT],      TRUE, 'd', "dat", "dat");
    setKind(&SpinKinds[SpinParser::K_ENUM],     TRUE, 'g', "enum", "enumerations");

    KeyWord keyCon = {"con", K_CONST, 0};
    KeyWord keyObj = {"obj", K_OBJECT, 0};
    KeyWord keyPub = {"pub", K_PUB, 0};
    KeyWord keyPri = {"pri", K_PRI, 0};
    KeyWord keyVar = {"var", K_VAR, 0};
    KeyWord keyDat = {"dat", K_DAT, 0};
    KeyWord keyNull= {NULL,  K_NONE, 0};

    spin_keywords.append(keyCon);
    spin_keywords.append(keyObj);
    spin_keywords.append(keyPub);
    spin_keywords.append(keyPri);
    spin_keywords.append(keyVar);
    spin_keywords.append(keyDat);
    spin_keywords.append(keyNull);
}

SpinParser::~SpinParser()
{
    clearDB();
}

void SpinParser::clearDB()
{
    db.clear();
    spinFiles.clear();
}

void SpinParser::setKind(kindOption *kind, bool en, const char letter, const char *name, const char *desc)
{
    kind->enabled = en;
    kind->letter = letter;
    kind->name = name;
    kind->description = desc;
}

/*
 * get a tree list.
 * all object instances will be listed with sub-objects
 * the list will be indented according to relative position
 */
QStringList SpinParser::spinFileTree(QString file, QString libpath)
{
    QString key;
    QString value;
    QString subnode;
    QString subfile;

    libraryPath = libpath;
    clearDB();

    this->findSpinTags(file, "root");

    spinFiles.append(file.mid(file.lastIndexOf("/")+1));
    QStringList keys = db.keys();

    makeTags(file);

    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        QStringList levels = key.split("/");
        int lcount = levels.count()-1;
        QStringList keyel = levels[lcount].split(KEY_ELEMENT_SEP);
        if(QString(keyel[0]).compare(keyel[1]) == 0) {
            objectInfo(value, subnode, subfile);
            //for(int n = 0; n < lcount; n++) subfile = " " + subfile;
            spinFiles.append(subfile);
        }
    }

    return spinFiles;
}

void SpinParser::makeTags(QString file)
{
    QStringList keys = db.keys();

    QString tagheader = \
"!_TAG_FILE_FORMAT	1	/original ctags format/\n"
"!_TAG_FILE_SORTED	1	/0=unsorted, 1=sorted, 2=foldcase/\n"
"!_TAG_PROGRAM_AUTHOR	Darren Hiebert	/dhiebert@users.sourceforge.net/\n"
"!_TAG_PROGRAM_NAME	Exuberant Ctags	//\n"
"!_TAG_PROGRAM_URL	http://ctags.sourceforge.net	/official site/\n"
"!_TAG_PROGRAM_VERSION	5.8	//\n";

    QString path = file.mid(0,file.lastIndexOf("/")+1);
    QFile tags(path+"tags");
    if(tags.open(QFile::WriteOnly | QFile::Text)) {
        tags.write(tagheader.toAscii());
        foreach(QString key, keys) {
            QStringList tl = db[key].split("\t");
            QString ts = tl[0]+"\t"+tl[1]+"\t/^"+tl[2]+"$/";
            tags.write(ts.toAscii()+"\n");
        }
        tags.close();
    }

}

/*
 * all symbols are accessible by key.
 * a key is a name list such as "/root/obj/subobj/subsubobj"
 * if the key is empty, return the root or top file list
 */
QStringList SpinParser::spinSymbols(QString file, QString objname)
{
    QStringList list;
    QStringList keys = db.keys();
    QString key;
    QString value;
    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        if(objname.length() > 0) {
            if(key.contains(objname+":",Qt::CaseInsensitive)) {
                QStringList tabs = value.split("\t");
                if(tabs.count() > 2)
                    list.append(tabs.at(2));
            }
        }
        else {
            if(value.contains(file,Qt::CaseInsensitive)) {
                QStringList tabs = value.split("\t");
                if(tabs.count() > 2)
                    list.append(tabs.at(2));
            }
        }
    }
    return list;
}

/*
 * these are convenience wrappers for typefilter
 */
QStringList SpinParser::spinConstants(QString file, QString objname)
{
    QStringList list;
    QStringList keys = db.keys();
    QString key;
    QString value;
    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        if(key.contains(objname+":",Qt::CaseInsensitive)) {
            QStringList tabs = value.split("\t");
            if(tabs.count() > 2)
                if(QString(tabs.at(3)).contains("c",Qt::CaseInsensitive))
                    list.append(tabs.at(2));
        }
    }
    return list;
}

QStringList SpinParser::spinMethods(QString file, QString objname)
{
    QStringList list;
    QStringList keys = db.keys();
    QString key;
    QString value;
    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        if(key.contains(objname+":",Qt::CaseInsensitive)) {
            QStringList tabs = value.split("\t");
            if(tabs.count() > 2) {
                if(QString(tabs.at(3)).contains("p",Qt::CaseInsensitive))
                    list.append(tabs.at(2));
                if(QString(tabs.at(3)).contains("f",Qt::CaseInsensitive))
                    list.append(tabs.at(2));
            }
        }
    }
    return list;
}

QStringList SpinParser::spinDat(QString file, QString objname)
{
    QStringList list;
    QStringList keys = db.keys();
    QString key;
    QString value;
    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        if(key.contains(objname+":",Qt::CaseInsensitive)) {
            QStringList tabs = value.split("\t");
            if(tabs.count() > 2) {
                if(QString(tabs.at(3)).contains("d",Qt::CaseInsensitive))
                    list.append(tabs.at(2));
            }
        }
    }
    return list;
}

QStringList SpinParser::spinVars(QString file, QString objname)
{
    QStringList list;
    QStringList keys = db.keys();
    QString key;
    QString value;
    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        if(key.contains(objname+":",Qt::CaseInsensitive)) {
            QStringList tabs = value.split("\t");
            if(tabs.count() > 2) {
                if(QString(tabs.at(3)).contains("v",Qt::CaseInsensitive))
                    list.append(tabs.at(2));
            }
        }
    }
    return list;
}

QStringList SpinParser::spinObjects(QString file, QString objname)
{
    QStringList list;
    QStringList keys = db.keys();
    QString key;
    QString value;
    for(int n = 0; n < keys.length(); n++) {
        key = keys[n];
        value = db[key];
        if(key.contains(objname+":",Qt::CaseInsensitive)) {
            QStringList tabs = value.split("\t");
            if(tabs.count() > 2) {
                if(QString(tabs.at(3)).contains("o",Qt::CaseInsensitive))
                    list.append(tabs.at(2));
            }
        }
    }
    return list;
}

/*
 *   FUNCTION DEFINITIONS
 */

/* Match the name of a tag (function, variable, type, ...) starting at pos. */
char const *SpinParser::extract_name (char const *pos, QString &name)
{
    while (isspace (*pos))
        pos++;
    name.clear();
    for (; *pos && !isspace (*pos) && *pos != '(' && *pos != ','; pos++)
        name.append(*pos);
    return pos;
}

int SpinParser::tokentype(QString tmp)
{
    int rc = K_NONE;
    int i;
    /* find out token type for parser states */
    if(tmp.indexOf(' ') > 0) {
        tmp = tmp.mid(0,tmp.indexOf(' '));
    }
    for (i = 0; spin_keywords[i].token != NULL; i++)
    {
        if (tmp.compare(spin_keywords[i].token,Qt::CaseInsensitive) == 0) {
            rc = spin_keywords[i].kind;
            break;
        }
    }
    return rc;
}

/* Match a keyword starting at p (case insensitive). */
int SpinParser::match_keyword (const char *p, KeyWord const *kw, QString &tag)
{
    QString name;
    int i;
    int j;
    int rc = K_NONE;
    char *tmp = (char*)p;

    /* printf ("match_keyword\n"); */

    for (i = 0; i < kw->token.length(); i++)
    {
        if (tolower (p[i]) != kw->token[i])
            return rc;
    }

    p += i;
    if(*p && !isspace(*p))
        return K_NONE;

    /* if p stands alone on the line, don't add it */
    if(strlen(p) == 0)
         return tokentype(tmp);

    for (j = 0; j < 1 + kw->skip; j++)
    {
        p = extract_name (p, name);
    }
    /* printf ("match_keyword name %d %s\n", name->length, name->buffer); */

    if(name.length() > 0) {
        rc = tokentype(kw->token);
        tag = name+"\t"+currentFile+"\t"+tmp+"\t"+SpinKinds[rc].letter;
    }
    return rc;
}

void SpinParser::match_constant (QString p)
{
    QString tag("");
    int len = p.indexOf("=");
    if(p.contains(QRegExp("\\b?=\\b")))
        len = 0;
    if(len > 0) {
        // add to database
        QString s = p.mid(0,len).trimmed();
        if(s.indexOf("con",0,Qt::CaseInsensitive) == 0)
            s = s.mid(4);
        s = s.trimmed();
        tag = s+"\t"+currentFile+"\t"+p+"\t"+SpinKinds[K_CONST].letter;
        db.insert(objectNode+KEY_ELEMENT_SEP+s,tag);
        qDebug() << objectNode << " ## " << tag;
    }
}

void SpinParser::match_dat (QString p)
{
    int j;
    QString tag = "";
    char *types[] = { (char*)"byte", (char*)"word", (char*)"long", 0 };
    char *type;
    for(j = 0; types[j] != 0; j++) {
        type = types[j];
        QString s = p.trimmed();
        if(s.indexOf("dat",0,Qt::CaseInsensitive) == 0)
            s = s.mid(4);
        s = s.trimmed();
        if(s.contains(type,Qt::CaseInsensitive)) {
            s = s.mid(0,s.indexOf(type,0,Qt::CaseInsensitive));
            s = s.trimmed();
            QStringList list = s.split(",",QString::SkipEmptyParts);
            if(list.length() > 0) {
                foreach(s,list) {
                    if(s.contains("["))
                        s = s.mid(0,s.indexOf("["));
                    s = s.trimmed();
                    tag = s+"\t"+currentFile+"\t"+p+"\t"+SpinKinds[K_VAR].letter;
                    db.insert(objectNode+KEY_ELEMENT_SEP+s,tag);
                    qDebug() << objectNode << " DAT " << tag;
                }
            }
            break;
        }
    }
}

void SpinParser::match_object (QString p)
{
    QString tag = "";
    QString subfile = "";
    QString subnode = "";
    int len = p.indexOf(":");
    if(len > 0) {
        // add to database
        QString s = p.mid(0,len);
        if(s.contains("OBJ",Qt::CaseInsensitive))
            s = s.mid(s.indexOf("OBJ",0,Qt::CaseInsensitive)+3);
        s = s.trimmed();
        tag = s+"\t"+currentFile+"\t"+p+"\t"+SpinKinds[K_OBJECT].letter;
        objectInfo(tag, subnode, subfile);
        db.insert(objectNode+"/"+subnode+KEY_ELEMENT_SEP+s,tag);
        qDebug() << objectNode+"/"+subnode << " :: " << tag;
        findSpinTags(subfile,objectNode+"/"+subnode);
    }
}

void SpinParser::match_pri (QString p)
{
    QString tag("");
    int len = p.indexOf("pri",0,Qt::CaseInsensitive);
    if(len == 0) {
        // add to database
        QString s = p.mid(4);
        if(s.indexOf("|") >= 0)
            s = s.mid(0,s.indexOf("|"));
        if(s.indexOf(":") >= 0)
            s = s.mid(0,s.indexOf(":"));
        if(s.indexOf("(") >= 0)
            s = s.mid(0,s.indexOf("("));
        s = s.trimmed();
        tag = s+"\t"+currentFile+"\t"+p+"\t"+SpinKinds[K_PRI].letter;
        db.insert(objectNode+KEY_ELEMENT_SEP+s,tag);
        qDebug() << objectNode << " pri " << tag;
    }
}

void SpinParser::match_pub (QString p)
{
    QString tag("");
    int len = p.indexOf("pub",0,Qt::CaseInsensitive);
    if(len == 0) {
        // add to database
        QString s = p.mid(4);
        if(s.indexOf("|") >= 0)
            s = s.mid(0,s.indexOf("|"));
        if(s.indexOf(":") >= 0)
            s = s.mid(0,s.indexOf(":"));
        if(s.indexOf("(") >= 0)
            s = s.mid(0,s.indexOf("("));
        s = s.trimmed();
        tag = s+"\t"+currentFile+"\t"+p+"\t"+SpinKinds[K_PUB].letter;
        db.insert(objectNode+KEY_ELEMENT_SEP+s,tag);
        qDebug() << objectNode << " pub " << tag;
    }
}

void SpinParser::match_var (QString p)
{
    int j;
    QString tag = "";
    char *types[] = { (char*)"byte", (char*)"word", (char*)"long", 0 };
    char *type;
    for(j = 0; types[j] != 0; j++) {
        type = types[j];
        QString s = p.trimmed();
        if(s.indexOf("var",0,Qt::CaseInsensitive) == 0)
            s = s.mid(4);
        s = s.trimmed();
        if(s.indexOf(type,0,Qt::CaseInsensitive) == 0) {
            s = s.mid(s.indexOf(type,0,Qt::CaseInsensitive)+4);
            QStringList list = s.split(",",QString::SkipEmptyParts);
            if(list.length() > 0) {
                foreach(s,list) {
                    if(s.contains("["))
                        s = s.mid(0,s.indexOf("["));
                    s = s.trimmed();
                    tag = s+"\t"+currentFile+"\t"+p+"\t"+SpinKinds[K_VAR].letter;
                    db.insert(objectNode+KEY_ELEMENT_SEP+s,tag);
                    qDebug() << objectNode << " VAR " << tag;
                }
            }
            break;
        }
    }
}

int SpinParser::objectInfo(QString tag, QString &name, QString &file)
{
    QStringList list = tag.split("\t",QString::KeepEmptyParts);
    if(list.length() == 0)
        return 0;
    name = QString(list[0]).trimmed();
    file = QString(list[2]).trimmed();
    file = file.mid(file.indexOf("\"")+1);
    file = file.mid(0,file.indexOf("\""));
    file = file.trimmed();

    if(file.contains(".spin",Qt::CaseInsensitive) == false)
        file += ".spin";

    return list.length();
}


QString SpinParser::checkFile(QString fileName)
{
    QString retfile = fileName;

    if(QFile::exists(fileName) == true) {
        return retfile;
    }
    else if(QFile::exists(libraryPath+fileName)) {
        return libraryPath+fileName;
    }
    else {
        QDir dir;
        QStringList list;
        QString fs = fileName;
        QString shortfile = fs.mid(fs.lastIndexOf("/")+1);
        QString path = fs.mid(0,fs.lastIndexOf("/")+1);
        dir.setPath(path);
        list = dir.entryList();
        foreach(QString s, list) {
            if(s.contains(shortfile,Qt::CaseInsensitive)) {
                return path+s;
            }
        }
        dir.setPath(libraryPath);
        list = dir.entryList();
        foreach(QString s, list) {
            if(s.contains(shortfile,Qt::CaseInsensitive)) {
                return libraryPath+"/"+s;
            }
        }
    }
    return retfile;
}

void SpinParser::findSpinTags (QString fileName, QString objnode)
{
    fileName = checkFile(fileName);
    if(QFile::exists(fileName) == false)
        return;

    SpinKind   state;
    QString line;
    QString tag;
    QString filestr;

    state = K_CONST; // spin starts with CONST

    QFile file(fileName);
    QTextStream in(&file);
    //in.setCodec("UTF-8");
    in.setAutoDetectUnicode(true);
    if(file.open(QFile::ReadOnly) != true)
        return;
    filestr = in.readAll();
    file.close();
    QStringList list = filestr.split("\n",QString::SkipEmptyParts,Qt::CaseInsensitive);

    bool blockComment = false;

    for(int n = 0; n < list.length(); n++)
    {
        // do here in case OBJ detect causes context change
        objectNode  = objnode;
        currentFile = fileName;

        QString line = QString(list[n]).trimmed();

        if(blockComment && line.contains("}")) {
            blockComment = false;
            line = line.mid(line.lastIndexOf("}")+1);
        }
        if(blockComment)
            continue;

        if(line.contains("\'")) {
            line = line.mid(0,line.indexOf("\'"));
        }
        if(line.contains("{")) {
            blockComment = true;
            line = line.mid(0,line.indexOf("{"));
        }

        /* Empty line? */
        if (line.length() < 1)
            continue;

        /* In Spin, keywords always are at the start of the line. */
        SpinKind type = K_NONE;
        foreach(KeyWord kw, spin_keywords) {
            if(kw.kind == K_NONE)
                continue;
            type = K_NONE;
            if(line.contains(kw.token,Qt::CaseInsensitive)) {
                // if line has a token, get the type and store it
                type = (SpinKind) match_keyword (line.toAscii(), &kw, tag);

                // keep state until it changes from K_NONE
                if(type != K_NONE) {
                    state = type;
                    break;
                }
            }
        }

        //printf ("state %d\n", state);
        switch(state) {
            case K_CONST:
                match_constant(line);
            break;
            case K_DAT:
                match_dat(line);
            break;
            case K_OBJECT:
                match_object(line);
            break;
            case K_PRI:
                match_pri(line);
            break;
            case K_PUB:
                match_pub(line);
            break;
            case K_VAR:
                match_var(line);
            break;
            default:
                tag = "";
            break;
        }
    }
}
