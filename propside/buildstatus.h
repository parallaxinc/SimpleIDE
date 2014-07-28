#ifndef BUILDSTATUS_H
#define BUILDSTATUS_H

#include "qtversion.h"

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
