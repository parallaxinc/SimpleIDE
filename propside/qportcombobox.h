#ifndef QPORTCOMBOBOX_H
#define QPORTCOMBOBOX_H

/*
 * This class is used to overload QComboBox so we can click on the combo-box for enumerating ports.
 */
#include <QComboBox>

class QPortComboBox : public QComboBox
{
Q_OBJECT
public:

    QPortComboBox(QWidget *w)
        : QComboBox(w)
    {
    }
    virtual ~QPortComboBox()
    {
    }

    void mousePressEvent(QMouseEvent * event) {
        // emit before handling rest of event otherwise bad things can happen!
        emit clicked();
        QComboBox::mousePressEvent(event);
    }

signals:
    void clicked();

};

#endif // QPORTCOMBOBOX_H
