#ifndef HIGHLIGHTBUILD_H
#define HIGHLIGHTBUILD_H

#include <QObject>
#include "properties.h"
#include "highlighter.h"

class HighlightBuild : public Highlighter
{
public:
    HighlightBuild(QTextDocument *parent, Properties *prop);
    void highlight();
};

#endif
// HIGHLIGHTBUILD_H
