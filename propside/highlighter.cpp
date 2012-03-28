/****************************************************************************
  The basis of this code is the Qt Highlighter example having the copyright
  below. This code is substanially different for choosing the rules, and it
  adds registry selection for highlight attributes, but the basic highlighter
  mechanism is the same as the original.
*****************************************************************************/

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
Highlighter::Highlighter(QTextDocument *parent, Properties *prop)
    : QSyntaxHighlighter(parent)
{
    properties = prop;
    highlightC();
}

bool Highlighter::getStyle(QString key, bool *italic)
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant var = settings.value(key, false);

    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        *italic = var.toBool();
        return true;
    }
    return false;
}

bool Highlighter::getWeight(QString key, QFont::Weight *weight)
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant var = settings.value(key, false);

    if(var.canConvert(QVariant::Bool)) {
        QString s = var.toString();
        *weight = var.toBool() ? QFont::Bold : QFont::Normal;
        return true;
    }
    return false;
}

bool Highlighter::getColor(QString key, Qt::GlobalColor *color)
{
    QSettings settings(publisherKey, ASideGuiKey, this);
    QVariant var = settings.value(key, false);

    if(var.canConvert(QVariant::Int)) {
        QString s = var.toString();
        int n = var.toInt();
        *color = (Qt::GlobalColor) properties->getQtColor(n);
        return true;
    }
    return false;
}

void Highlighter::getProperties()
{
    bool   style;
    QFont::Weight   weight;
    Qt::GlobalColor color;

    if(getStyle(hlNumStyleKey,&style))
        hlNumStyle = style;
    if(getWeight(hlNumWeightKey, &weight))
        hlNumWeight = weight;
    if(getColor(hlNumColorKey, &color))
        hlNumColor = color;

    if(getStyle(hlFuncStyleKey,&style))
        hlFuncStyle = style;
    if(getWeight(hlFuncWeightKey, &weight))
        hlFuncWeight = weight;
    if(getColor(hlFuncColorKey, &color))
        hlFuncColor = color;

    if(getStyle(hlKeyWordStyleKey,&style))
        hlKeyWordStyle = style;
    if(getWeight(hlKeyWordWeightKey, &weight))
        hlKeyWordWeight = weight;
    if(getColor(hlKeyWordColorKey, &color))
        hlKeyWordColor = color;

    if(getStyle(hlPreProcStyleKey,&style))
        hlPreProcStyle = style;
    if(getWeight(hlPreProcWeightKey, &weight))
        hlPreProcWeight = weight;
    if(getColor(hlPreProcColorKey, &color))
        hlPreProcColor = color;

    if(getStyle(hlQuoteStyleKey,&style))
        hlQuoteStyle = style;
    if(getWeight(hlQuoteWeightKey, &weight))
        hlQuoteWeight = weight;
    if(getColor(hlQuoteColorKey, &color))
        hlQuoteColor = color;

    if(getStyle(hlLineComStyleKey,&style))
        hlLineComStyle = style;
    if(getWeight(hlLineComWeightKey, &weight))
        hlLineComWeight = weight;
    if(getColor(hlLineComColorKey, &color))
        hlLineComColor = color;

    if(getStyle(hlBlockComStyleKey,&style))
        hlBlockComStyle = style;
    if(getWeight(hlBlockComWeightKey, &weight))
        hlBlockComWeight = weight;
    if(getColor(hlBlockComColorKey, &color))
        hlBlockComColor = color;
}

void Highlighter::highlightC()
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

    // do "functions" first so we can override if names are keywords
    functionFormat.setFontItalic(hlFuncStyle);
    functionFormat.setForeground(hlFuncColor);
    functionFormat.setFontWeight(hlFuncWeight);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // handle C keywords
    keywordFormat.setForeground(hlKeyWordColor);
    keywordFormat.setFontWeight(hlKeyWordWeight);
    keywordFormat.setFontItalic(hlKeyWordStyle);
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

    preprocessorFormat.setFontItalic(hlPreProcStyle);
    preprocessorFormat.setForeground(hlPreProcColor);
    preprocessorFormat.setFontWeight(hlPreProcWeight);
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
            << "\\bint\\d+_t"
            << "\\buint\\d+_t"
            ;
    foreach (const QString &pattern, preprocessorPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = preprocessorFormat;
        highlightingRules.append(rule);
    }

    // quoted strings
    quotationFormat.setFontItalic(hlQuoteStyle);
    quotationFormat.setForeground(hlQuoteColor);
    quotationFormat.setFontWeight(hlQuoteWeight);

    rule.pattern = QRegExp("[\"].*[\"]");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("[<][a-z,A-Z].*[>]");
    highlightingRules.append(rule);

    // single line comments
    singleLineCommentFormat.setFontItalic(hlLineComStyle);
    singleLineCommentFormat.setForeground(hlLineComColor);
    singleLineCommentFormat.setFontWeight(hlLineComWeight);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // multilineline comments
    multiLineCommentFormat.setFontItalic(hlBlockComStyle);
    multiLineCommentFormat.setForeground(hlBlockComColor);
    multiLineCommentFormat.setFontWeight(hlBlockComWeight);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

}

/*
 * this is mostly experimental. not sure if it can be used yet.
 */
void Highlighter::highlightSpin()
{
    this->parent();

    getProperties();

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

    // quoted strings
    quotationFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("[\"].*[\"]");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // single line comments
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
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
