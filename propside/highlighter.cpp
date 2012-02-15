/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include "highlighter.h"

//! [0]
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    highlightC();
}

void Highlighter::highlightC()
{
    HighlightingRule rule;

    // do "functions" first so we can override if names are keywords
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // handle C keywords
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns
            << "\\bauto\\b"
            << "\\bbreak\\b"
            << "\\bcase\\b"
            << "\\bchar\\b"
            << "\\bconst\\b"
            << "\\bcontinue\\b"
            << "\\bdefault\\b"
            << "\\bdo\\b"
            << "\\bdouble\\b"
            << "\\belse\\b"
            << "\\benum\\b"
            << "\\bextern\\b"
            << "\\bfloat\\b"
            << "\\bfor\\b"
            << "\\bgoto\\b"
            << "\\bif\\b"
            << "\\bint\\b"
            << "\\blong\\b"
            << "\\bstruct\\b"
            << "\\bswitch\\b"
            << "\\bregister\\b"
            << "\\breturn\\b"
            << "\\bshort\\b"
            << "\\bsigned\\b"
            << "\\bsizeof\\b"
            << "\\bstatic\\b"
            << "\\btypedef\\b"
            << "\\bunion\\b"
            << "\\bunsigned\\b"
            << "\\bvoid\\b"
            << "\\bvolatile\\b"
            << "\\bwhile\\b"
            << "={2,}" << "+{2,}" << "-{2,}" << "_{2,}" << "\\{2,}"
            << "\\belif\\b"
            << "\\bifdef\\b"
            << "\\bendif\\b"
            ;
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    preprocessorFormat.setForeground(Qt::darkYellow);
    preprocessorFormat.setFontWeight(QFont::Bold);
    //preprocessorFormat.setFontItalic(true);
    QStringList preprocessorPatterns;
    preprocessorPatterns
            << "\\bassert\\b"
            << "\\bclass\\b"
            << "\\bdefine\\b"
            << "\\bdefined\\b"
            << "\\berror\\b"
            << "\\bident\\b"
            << "\\bimport\\b"
            << "\\binclude\\b"
            << "\\binclude_next\\b"
            << "\\bline\\b"
            << "\\bpragma\\b"
            << "\\bpublic\\b"
            << "\\bprivate\\b"
            << "\\bunassert\\b"
            << "\\bundef\\b"
            << "\\bwarning\\b"
            ;
    foreach (const QString &pattern, preprocessorPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = preprocessorFormat;
        highlightingRules.append(rule);
    }

    // single line comments
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // quoted strings
    quotationFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("[<\"].*[\">]");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // multilineline comments
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

}

/*
 * this is mostly experimental. not sure if it can be used yet.
 */
void Highlighter::highlightSpin()
{
    this->parent();

    HighlightingRule rule;

    // do "functions" first so we can override if names are keywords
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // handle Spin keywords
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    /*
     * add spin patterns later
     */
    keywordPatterns
            << "\\babort\\b"
            << "={2,}" << "+{2,}" << "-{2,}" << "_{2,}" << "\\{2,}"
            << "\\belif\\b"
            << "\\bifdef\\b"
            << "\\bendif\\b"
            ;
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    preprocessorFormat.setForeground(Qt::darkYellow);
    preprocessorFormat.setFontWeight(QFont::Bold);
    //preprocessorFormat.setFontItalic(true);
    QStringList preprocessorPatterns;
    preprocessorPatterns
            << "\\bcon\\b"
            << "\\bdat\\b"
            << "\\bobj\\b"
            << "\\bpub\\b"
            << "\\bpri\\b"
            << "\\bvar\\b"
            << "\\bdefine\\b"
            << "\\bdefined\\b"
            << "\\berror\\b"
            << "\\binclude\\b"
            << "\\bundef\\b"
            << "\\bwarning\\b"
            ;
    foreach (const QString &pattern, preprocessorPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = preprocessorFormat;
        highlightingRules.append(rule);
    }

    // single line comments
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // quoted strings
    quotationFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("[<\"].*[\">]");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // multilineline comments
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

}

//! [7]
void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
//! [7] //! [8]
    setCurrentBlockState(0);
//! [8]

//! [9]
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

//! [9] //! [10]
    while (startIndex >= 0) {
//! [10] //! [11]
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
//! [11]
