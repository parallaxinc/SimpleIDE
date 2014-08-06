#include "workspacedialog.h"

WorkspaceDialog::WorkspaceDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("SimpleIDE Create Workspace");

    dialogLabel.setText(tr("SimpleIDE will create the required workspace here:"));

    QFontMetrics fm = dialogLabel.fontMetrics();
    leditFolder.setMinimumWidth(dialogLabel.text().length()*fm.width(QChar('9').toLatin1()));
    leditFolder.setReadOnly(true);

    btnOk.setText(tr("&Ok"));
    btnChange.setToolTip("Click to create folder.");
    connect(&btnOk, SIGNAL(clicked()), this, SLOT(accept()));

    btnShowChange.setText("More ...");
    connect(&btnShowChange, SIGNAL(clicked()), this, SLOT(showChangeFolder()));

    btnChange.setText("Change");
    btnChange.setToolTip("Click to change folder. Not recommended.");
#if 0
    QFont chFont = btnChange.font();
    chFont.setPointSize(7);
    QFontMetrics cfm = QFontMetrics(chFont);
    int wd = btnChange.text().length();
    wd = cfm.width(QChar('c').toLatin1())*(wd+2);
    int ht = cfm.height()+7;
    btnChange.setFont(chFont);
    btnChange.setMaximumHeight(ht);
    //btnChange.setMaximumWidth(wd);
#endif
    connect(&btnChange, SIGNAL(clicked()), this, SLOT(changeFolder()));

    chgLayout.addWidget(&btnChange);
    //chgLayout.addItem(&changeLabel);

    grid.addWidget(&dialogLabel, 0, 0, 2, 4);
    grid.addWidget(&blankLabel, 1, 0);
    grid.addWidget(&blankLabel, 2, 0);
    grid.addWidget(&leditFolder, 3, 0);
    grid.addWidget(&btnChange, 3, 1);
    grid.addWidget(&blankLabel, 4, 0);
    grid.addWidget(&btnOk, 5, 1);
    grid.addWidget(&blankLabel, 6, 0);
    //grid.addLayout(chgLayout, 7, 0, 1, 4);

    btnOk.setDefault(true);

    this->setLayout(&grid);
    this->setWindowFlags(Qt::Tool);
}

bool WorkspaceDialog::replaceDialog(QString folder, QString &selected)
{
    if(!folder.endsWith("/")) folder += "/";
    leditFolder.setText(folder);
    workspace = "";
    this->exec();

    if(workspace.length() > 0) {
        selected = workspace;
        QDir ws(selected);
        ws.mkpath(selected);
        return ws.exists();
    }
    else {
        selected = workspace;
        return false;
    }
}

QString WorkspaceDialog::getWorkspace()
{
    return workspace;
}

void WorkspaceDialog::showChangeFolder()
{
    //changeLayout.setVisible(true);
}

void WorkspaceDialog::changeFolder()
{
    QString folder = leditFolder.text();
    if(folder.endsWith("/")) folder = folder.left(folder.length()-1);
    qDebug() << "changeFolder" << folder;

    QString wrk = QFileDialog::getExistingDirectory(this,
        tr("SimpleIDE Workspace Folder Location"),folder);

    if(!wrk.length()) {
        return;
    }

    QDir wrkd(wrk);
    if(wrkd.exists(wrk) == false) {
        QMessageBox::critical(this,
            tr("Folder Error"), tr("Can't find specified folder."));
        workspace = "";
    }
    else {
        if(!wrk.endsWith("/")) wrk+="/";
        workspace = wrk+"SimpleIDE/";
    }
    leditFolder.setText(workspace);
}

void WorkspaceDialog::accept()
{
    workspace = leditFolder.text();
    this->close();
}


