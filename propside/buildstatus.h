#ifndef BUILDSTATUS_H
#define BUILDSTATUS_H

#include <QtGui>
#include "highlightbuild.h"

class BuildStatus : public QPlainTextEdit
{
public:
    BuildStatus(QWidget *parent);
    virtual ~BuildStatus();

private:
    HighlightBuild *highlighter;
};

#endif // BUILDSTATUS_H
