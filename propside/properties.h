#ifndef PROPERTIES_H
#define PROPERTIES_H

#define publisherKey        "MicroCSource"
#define publisherComKey     "MicroCSource.com"
#define ASideGuiKey         "SimpleIDE"
#define compilerKey         "Compiler"
#define includesKey         "Includes"
#define separatorKey        "PathSeparator"
#define configFileKey       "ConfigFile"
#define workspaceKey        "Workspace"
#define editorFontKey       "EditorFont"
#define fontSizeKey         "FontSize"
#define lastFileNameKey     "LastFileName"
#define lastBoardNameKey    "LastBoardName"
#define lastPortNameKey     "LastPortName"
#define lastTermXposKey     "LastTermXposition"
#define lastTermYposKey     "LastTermYposition"

#define tabSpacesKey        "TabSpacesCount"
#define hlEnableKey         "HighlightEnable"
#define hlNumStyleKey       "HighlightNumberStyle"
#define hlNumWeightKey      "HighlightNumberWeight"
#define hlNumColorKey       "HighlightNumberColor"
#define hlFuncStyleKey      "HighlightFunctionStyle"
#define hlFuncWeightKey     "HighlightFunctionWeight"
#define hlFuncColorKey      "HighlightFunctionColor"
#define hlKeyWordStyleKey   "HighlightKeyWordTypeStyle"
#define hlKeyWordWeightKey  "HighlightKeyWordTypeWeight"
#define hlKeyWordColorKey   "HighlightKeyWordTypeColor"
#define hlPreProcStyleKey   "HighlightPreProcTypeStyle"
#define hlPreProcWeightKey  "HighlightPreProcTypeWeight"
#define hlPreProcColorKey   "HighlightPreProcTypeColor"
#define hlQuoteStyleKey     "HighlightQuoteTypeStyle"
#define hlQuoteWeightKey    "HighlightQuoteTypeWeight"
#define hlQuoteColorKey     "HighlightQuoteTypeColor"
#define hlLineComStyleKey   "HighlightLineCommmentTypeStyle"
#define hlLineComWeightKey  "HighlightLineCommmentTypeWeight"
#define hlLineComColorKey   "HighlightLineCommmentTypeColor"
#define hlBlockComStyleKey  "HighlightBlockCommentTypeStyle"
#define hlBlockComWeightKey "HighlightBlockCommentTypeWeight"
#define hlBlockComColorKey  "HighlightBlockCommentTypeColor"

#include <QtGui>

class PColor;

class Properties : public QDialog
{
    Q_OBJECT
public:
    explicit Properties(QWidget *parent = 0);

    enum Colors {
        Black, DarkGray, Gray, LightGray,
        Blue, DarkBlue,
        Cyan, DarkCyan,
        Green, DarkGreen,
        Magenta, DarkMagenta,
        Red, DarkRed,
        Yellow, DarkYellow,
        LastColor };

    int getTabSpaces();

signals:

public slots:
    void browseCompiler();
    void browseIncludes();
    void browseWorkspace();
    void accept();
    void reject();
    void showProperties();

private:

    void setupFolders();
    void setupGeneral();
    void setupHighlight();

    QTabWidget  tabWidget;

    QLineEdit   *leditCompiler;
    QLineEdit   *leditIncludes;
    QLineEdit   *leditWorkspace;
    QString     compilerstr;
    QString     includesstr;
    QString     mypath;

    QLineEdit   tabSpaces;

    QCheckBox   hlEnable;
    QComboBox   hlNumStyle;
    QCheckBox   hlNumWeight;
    QComboBox   hlNumColor;
    QToolButton hlNumButton;
    QComboBox   hlFuncStyle;
    QCheckBox   hlFuncWeight;
    QComboBox   hlFuncColor;
    QComboBox   hlKeyWordStyle;
    QCheckBox   hlKeyWordWeight;
    QComboBox   hlKeyWordColor;
    QComboBox   hlPreProcStyle;
    QCheckBox   hlPreProcWeight;
    QComboBox   hlPreProcColor;
    QComboBox   hlQuoteStyle;
    QCheckBox   hlQuoteWeight;
    QComboBox   hlQuoteColor;
    QComboBox   hlLineComStyle;
    QCheckBox   hlLineComWeight;
    QComboBox   hlLineComColor;
    QComboBox   hlBlockComStyle;
    QCheckBox   hlBlockComWeight;
    QComboBox   hlBlockComColor;

    QVector<PColor*> propertyColors;
};

class PColor : public QColor
{
private:
    QString mName;

public:
    PColor (QString name) { mName = name; }
    QString getName() { return mName; }

};


#endif // PROPERTIES_H
