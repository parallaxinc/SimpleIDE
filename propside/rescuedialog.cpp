#include "rescuedialog.h"

RescueDialog::RescueDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Rescue Me!"));
    setWindowFlags(Qt::Tool);
    layout = new QVBoxLayout(this);;
    label = new QLabel(tr("Copy/Paste this information so someone can help."));
    edit  = new QPlainTextEdit();
    layout->addWidget(label);
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
