#ifndef PROPERTYCOLORS_H
#define PROPERTYCOLORS_H

#include <QtGui>

class PColor : public QColor
{
private:
    QString mName;
    Qt::GlobalColor mValue;

public:
    PColor (QString name, Qt::GlobalColor value) {
        mName = name;
        mValue = value;
    }
    QString getName() {
        return mName;
    }
    Qt::GlobalColor getValue() {
        return mValue;
    }

    enum Colors {
        Black, DarkGray,
        Gray, LightGray,
        Blue, DarkBlue,
        Cyan, DarkCyan,
        Green, DarkGreen,
        Magenta, DarkMagenta,
        Red, DarkRed,
        Yellow, DarkYellow,
        White, LastColor };

};

#endif // PROPERTYCOLORS_H
