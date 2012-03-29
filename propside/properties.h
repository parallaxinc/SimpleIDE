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
#define loadDelayKey        "LoadDelay_us"

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

    void cleanSettings();

    int getTabSpaces();
    int getLoadDelay();
    int setComboIndexByValue(QComboBox *combo, QString value);

    Qt::GlobalColor getQtColor(int index);

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
    void addHighlights(QComboBox *box, QVector<PColor*> p);
    void setupHighlight();

    QTabWidget  tabWidget;

    QLineEdit   *leditCompiler;
    QLineEdit   *leditIncludes;
    QLineEdit   *leditWorkspace;
    QString     compilerstr;
    QString     includesstr;
    QString     mypath;

    QLineEdit   tabSpaces;
    QLineEdit   loadDelay;

    QToolButton hlNumButton;

    QCheckBox   hlEnable;
    QCheckBox   hlNumStyle;
    QCheckBox   hlNumWeight;
    QComboBox   hlNumColor;
    QCheckBox   hlFuncStyle;
    QCheckBox   hlFuncWeight;
    QComboBox   hlFuncColor;
    QCheckBox   hlKeyWordStyle;
    QCheckBox   hlKeyWordWeight;
    QComboBox   hlKeyWordColor;
    QCheckBox   hlPreProcStyle;
    QCheckBox   hlPreProcWeight;
    QComboBox   hlPreProcColor;
    QCheckBox   hlQuoteStyle;
    QCheckBox   hlQuoteWeight;
    QComboBox   hlQuoteColor;
    QCheckBox   hlLineComStyle;
    QCheckBox   hlLineComWeight;
    QComboBox   hlLineComColor;
    QCheckBox   hlBlockComStyle;
    QCheckBox   hlBlockComWeight;
    QComboBox   hlBlockComColor;

    QVector<PColor*> propertyColors;
};

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

};


#endif // PROPERTIES_H
