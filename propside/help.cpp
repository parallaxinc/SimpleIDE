#include "help.h"
#include "properties.h"

Help::Help()
{
    lay = new QVBoxLayout(this);
    webview = new QWebView(this);

    QHBoxLayout *blay = new QHBoxLayout(this);

#if 0
    // Thinking about using combo-box for different library sets.
    // Not fully committed to the idea just yet.
    cbAddress = new QComboBox(this);
    cbAddress->addItem(tr("Propeller-GCC Libraries"));
    cbAddress->addItem(tr("Propeller-GCC UserGuide"));
    cbAddress->addItem(tr("Propeller-GCC Examples"));
    blay->addWidget(cbAddress);
#endif

#if 0
    // TODO: don't let this be enabled for release.
    // refresh button should't be necessary for help
    QToolButton *reload = new QToolButton(this);
    reload->setIcon(QIcon(":/images/refresh.png"));
    reload->setToolTip(tr("Reload"));
    blay->addWidget(reload);
    connect(reload, SIGNAL(clicked()), webview, SLOT(reload()));
#endif
    QToolButton *back = new QToolButton(this);
    back->setIcon(QIcon(":/images/back.png"));
    back->setToolTip(tr("back"));
    blay->addWidget(back);
    connect(back, SIGNAL(clicked()), webview, SLOT(back()));

    QToolButton *forward = new QToolButton(this);
    forward->setIcon(QIcon(":/images/forward.png"));
    forward->setToolTip(tr("Forward"));
    blay->addWidget(forward);
    connect(forward, SIGNAL(clicked()), webview, SLOT(forward()));

    butbox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(butbox, SIGNAL(accepted()), this, SLOT(accept()));
    blay->addWidget(butbox);
    lay->addLayout(blay);
    lay->addWidget(webview);

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(QString(ASideGuiKey)+" "+tr("Library")+" "+tr("Help"));
    setLayout(lay);
}

Help::~Help()
{
    delete webview;
}

void Help::show(QString path, QString text)
{
    QString address = "file:///"+path+"/index.html";
    if(text.isEmpty()) {
        webview->load(QUrl(address));
        exec();
        return;
    }
    QString tagtext = "\">"+text+"</a>";
    QDir dir(path);
    QStringList filter(QString("*.html"));
    foreach(QString name, dir.entryList(filter)) {
        QFile file(path+"/"+name);
        QString fileText;
        if(file.open(QFile::ReadOnly)) {
            fileText = file.readAll();
            if (fileText.length() > 0 && fileText.indexOf(tagtext)) {
                QStringList list = fileText.split(tagtext);
                if(list.length() > 1) {
                    QString s = list[0];
                    s = s.mid(s.lastIndexOf("href=\"")+6);
                    s = s.left(s.indexOf(tagtext));
                    address = "file:///"+path+"/"+s;
                    file.close();
                    break;
                }
            }
            file.close();
        }
    }
    webview->load(QUrl(address));
    exec();
}

void Help::accept()
{
    done(QDialog::Accepted);
}
