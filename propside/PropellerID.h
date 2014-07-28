#ifndef PROPELLERID_H
#define PROPELLERID_H

#include <QThread>
#include "qextserialport.h"

class PropellerID : public QThread
{
Q_OBJECT
public:
    PropellerID(QObject *parent = 0);
    virtual ~PropellerID();

    int  isDevice(QString port);
    void run();


    enum { RESET_BY_DTR = 1 };
    enum { RESET_BY_RTS = 2 };

    void setDtrReset() {
        resetType = RESET_BY_DTR;
    }
    void setRtsReset() {
        resetType = RESET_BY_RTS;
    }

private:

    int resetType;
    QextSerialPort *port;

    enum { RXSIZE = (1<<10)-1 };

    int rxhead;
    int rxtail;
    char rxqueue[RXSIZE+1];

    /**
     * receive a buffer
     * @param buff - char pointer to buffer
     * @param n - number of bytes in buffer to read
     * @returns number of bytes read
     */
    int rx(char* buff, int n);

    /**
     * transmit a buffer
     * @param buff - char pointer to buffer
     * @param n - number of bytes in buffer to send
     * @returns zero on failure
     */
    int tx(char* buff, int n);

    /**
     * receive a buffer with a timeout
     * @param buff - char pointer to buffer
     * @param n - number of bytes in buffer to read
     * @param timeout - timeout in milliseconds
     * @returns number of bytes read or SERIAL_TIMEOUT
     */
    int rx_timeout(char* buff, int n, int timeout);

    /**
     * hwreset ... resets Propeller hardware using DTR or RTS
     * @param sparm - pointer to DCB serial control struct
     * @returns void
     */
    void hwreset(void);

private:

    char LFSR; // 'P'

    int pload_verbose;
    int pload_delay;

    enum { SHUTDOWN_CMD             =  0 };
    enum { DOWNLOAD_RUN_BINARY      =  1 };
    enum { DOWNLOAD_EEPROM          =  2 };
    enum { DOWNLOAD_RUN_EEPROM      =  3 };
    enum { DOWNLOAD_SHUTDOWN        =  4 };
    enum { SERIAL_TIMEOUT           = -1 };
    enum { PLOAD_RESET_DEVICE       =  0 };
    enum { PLOAD_NORESET            =  1 };
    enum { PLOAD_STATUS_OK          =  0 };
    enum { PLOAD_STATUS_OPEN_FAILED = -1 };
    enum { PLOAD_STATUS_NO_PROPELLER= -2 };


    /**
     * update the LFSR byte
     */
    int iterate(void);

    /**
     * getBit ... get bit from serial stream
     * @param hSerial - file handle to serial port
     * @param status - pointer to transaction status 0 on error
     * @returns bit state 1 or 0
     */
    int getBit(int* status, int timeout);

    /**
     * getAck ... get ack from serial stream
     * @param hSerial - file handle to serial port
     * @param status - pointer to transaction status 0 on error
     * @returns bit state 1 or 0
     */
    int getAck(int* status, int timeout);

    /**
     * makelong ... make an encoded long word to string
     * @param data - value to send
     * @param buff - uint8_t buffer
     * @returns nada
     */
    void makelong(int data, char* buff);

    /**
     * sendlong ... transmit an encoded long word to propeller
     * @param hSerial - file handle to serial port
     * @param data - value to send
     * @returns number of bytes sent
     */
    int sendlong(int data);

    /**
     * hwfind ... find propeller using sync-up sequence.
     * @param hSerial - file handle to serial port
     * @returns zero on failure
     */
    int hwfind(int retry);

    /**
     * read all data until empty.
     */
    void flushPort();

    /**
     * find a propeller on port
     * @param hSerial - file handle to serial port
     * @param sparm - pointer to DCB serial control struct
     * @param port - pointer to com port name
     * @returns 1 if found, 0 if not found, or -1 if port not opened.
     */
    int findprop(const char* port);

private slots:
    void portHandler();

signals:
    void portEvent();

};

#endif // PROPELLERID_H
