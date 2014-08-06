#include "rescuedialog.h"
#include <QMessageBox>

RescueDialog::RescueDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Rescue Me!"));
    setWindowFlags(Qt::Tool);
    layout = new QVBoxLayout(this);
    hlayout = new QHBoxLayout(this);
    copyBtn = new QPushButton(tr("Copy"),this);
    connect(copyBtn, SIGNAL(clicked()), this, SLOT(copyText()));
    label = new QLabel(tr("Copy and paste this information to a place where someone can help."));
    edit  = new QPlainTextEdit();
    hlayout->addWidget(copyBtn);
    hlayout->addWidget(label,100);
    layout->addLayout(hlayout);
    layout->addWidget(edit);
    setLayout(layout);
    resize(600,400);
}

void RescueDialog::setEditText(QString text)
{
    edit->setPlainText(text);
    edit->selectAll();
    edit->setReadOnly(true);
}

void RescueDialog::copyText()
{
    edit->selectAll();
    edit->copy();
    QMessageBox::information(this,tr("Copy Done"),tr("Text copied. Now paste."));
}
