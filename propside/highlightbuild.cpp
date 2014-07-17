#include "highlightbuild.h"

HighlightBuild::HighlightBuild(QTextDocument *parent, Properties *prop)
    : Highlighter(parent, prop)
{
    highlight();
}

void HighlightBuild::highlight()
{
    getProperties();

    HighlightingRule rule;

    // numbers
    numberFormat.setForeground(hlNumColor);
    numberFormat.setFontWeight(hlNumWeight);
    numberFormat.setFontItalic(hlNumStyle);
    rule.format = numberFormat;
    rule.pattern = QRegExp("\\b\\d+");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b0x[0-9,a-f,A-F]*");
    highlightingRules.append(rule);

    /// keyword colors for module names from beginning of line to :
    keywordFormat.setForeground(hlKeyWordColor);
    keywordFormat.setFontWeight(hlKeyWordWeight);
    keywordFormat.setFontItalic(hlKeyWordStyle);
    rule.format = keywordFormat;
    rule.pattern = QRegExp("^Project[^\n]*");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("^collect[^\n]*");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("^Done.");
    highlightingRules.append(rule);


    preprocessorFormat.setFontItalic(hlPreProcStyle);
    preprocessorFormat.setForeground(hlPreProcColor);
    preprocessorFormat.setFontWeight(hlPreProcWeight);
    rule.format = preprocessorFormat;
    rule.pattern = QRegExp("warning:[^\n]*");
    highlightingRules.append(rule);

    // quoted strings Errors that is
    quotationFormat.setFontItalic(hlQuoteStyle);
    quotationFormat.setForeground(hlQuoteColor);
    quotationFormat.setFontWeight(hlQuoteWeight);
    rule.pattern = QRegExp("error:[^\n]*");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("relocation truncated[^\n]*");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("region `.*overflowed[^\n]*");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("Build Failed![^\n]*");
    highlightingRules.append(rule);

    // single line comments ... use for in function markup
    singleLineCommentFormat.setFontItalic(hlLineComStyle);
    singleLineCommentFormat.setForeground(hlLineComColor);
    singleLineCommentFormat.setFontWeight(hlLineComWeight);
    rule.pattern = QRegExp("In function[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // multilineline comments ... gotta have something :(
    multiLineCommentFormat.setFontItalic(hlBlockComStyle);
    multiLineCommentFormat.setForeground(hlBlockComColor);
    multiLineCommentFormat.setFontWeight(hlBlockComWeight);
    commentStartExpression = QRegExp("(");
    commentEndExpression = QRegExp(")");
}
