/*
 * This file is part of the Parallax Propeller SimpleIDE development environment.
 *
 * Copyright (C) 2014 Parallax Incorporated
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    rule.pattern = QRegExp("\\b:[0-9,a-f,A-F][0-9,a-f,A-F]");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[0-9,a-f,A-F][0-9,a-f,A-F]:");
    highlightingRules.append(rule);
    //rule.pattern = QRegExp("\\b[0-9,a-f,A-F][0-9,a-f,A-F][0-9,a-f,A-F][0-9,a-f,A-F][0-9,a-f,A-F][0-9,a-f,A-F]");
    //highlightingRules.append(rule);

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
