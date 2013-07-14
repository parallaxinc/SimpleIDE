#ifndef DEVICEID_H
#define DEVICEID_H

#include <QObject>

class DeviceID : public QObject
{
public:
    DeviceID(int reset) {
        resetType = reset;
    }

    virtual bool isDevice(QString port) {
        if(port.isEmpty())
            return false;
        return false;
    }

    enum { RESET_BY_DTR = 1 };
    enum { RESET_BY_RTS = 2 };

    int resetType;

    void setDtrReset() {
        resetType = RESET_BY_DTR;
    }
    void setRtsReset() {
        resetType = RESET_BY_RTS;
    }
};

#endif // DEVICEID_H
