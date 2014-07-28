/*
 * Replace is similar to Find.
 */

#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include "qtversion.h"

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    ReplaceDialog(QWidget *parent = 0);
    QTextDocument::FindFlag getFlags(int prev = 0);

    QString getFindText();
    void    clearFindText();
    QString getReplaceText();
    void    clearReplaceText();
    void    setFindText(QString text);

    bool showBeginMessage(QString type);
    bool showEndMessage(QString type);

    void setEditor(QPlainTextEdit *ed);

public slots:
    void findChanged(QString text);
    void findClicked();
    void findDirection();
    void findNextClicked();
    void findPrevClicked();
    void replaceClicked();
    void replaceNextClicked();
    void replacePrevClicked();
    void replaceAllClicked();

private:
    QPlainTextEdit *editor;

    QToolButton *findNextButton;
    QToolButton *findPrevButton;
    QToolButton *replaceNextButton;
    QToolButton *replacePrevButton;
    QPushButton *replaceAllButton;
    QLineEdit   *findEdit;
    QString     findText;
    QLineEdit   *replaceEdit;
    QString     replaceText;

    bool        findForward;
    QPushButton *okButton;

    QToolButton *caseSensitiveButton;
    QToolButton *wholeWordButton;
    QToolButton *regexButton;

    QGridLayout *layout;

    /* last find position */
    int         findPosition;
    int         wasClicked;
};

#endif
// REPLACEDIALOG_H
