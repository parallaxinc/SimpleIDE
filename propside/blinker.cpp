#include "blinker.h"

Blinker::Blinker(QLabel *status)
{
    this->status = status;
}

/*
 * use for blinking status color
 */
void Blinker::run()
{
    int count = 71;
    while(count-- > 0) {
        msleep(100);
        QApplication::processEvents();
        switch(count) {
            case 60:
            case 40:
            case 20:
                emit statusNone();
                break;
            case 70:
            case 50:
            case 30:
            case 10:
                emit statusFailed();
                break;
        }
    }
}


