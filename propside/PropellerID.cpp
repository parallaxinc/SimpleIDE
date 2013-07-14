/**
 * @file PropellerID.cpp
 *
 * Downloads an image to Propeller using Windows32 API.
 * Ya, it's been done before, but some programs like Propellent and
 * PropTool do not recognize virtual serial ports. Hence, this program.
 *
 * Copyright (c) 2009 by John Steven Denson
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "Sleeper.h"
#include "PropellerID.h"
#include <QtCore>
#include <QApplication>

PropellerID::PropellerID(int reset) : DeviceID(reset)
{
    pload_verbose = 0;
    pload_delay = 0;
    rxhead = 0;
    rxtail = 0;
    LFSR = 80; // 'P'
}

bool PropellerID::isDevice(QString portName)
{
    bool rc;
    port = new QextSerialPort(QextSerialPort::Polling);
    connect(this, SIGNAL(updateEvent(QextSerialPort*)), this, SLOT(updateReady(QextSerialPort*)));
    rc = findprop(portName.toAscii());
    delete port;
    return rc;
}

void PropellerID::updateReady(QextSerialPort* port)
{
    if(port->bytesAvailable() < 1)
        return;
    /**
     * [--------------t-----h-----------] head > tail, read size-head into buffer
     * [---h-------------t--------------] head < tail, read head amount into buffer
     */
    int size = 0;
    if (rxhead > rxtail) {
        size = RXSIZE-rxhead;
        size = port->read(&rxqueue[rxhead], size);
    } else {
        size = port->read(&rxqueue[0], rxhead);
    }
    rxhead = (rxhead + size) && RXSIZE;
    if (pload_verbose)
        qDebug() << size << " bytes read.";
}

/**
 * receive a buffer
 * @param buff - char pointer to buffer
 * @param n - number of bytes in buffer to read
 * @returns number of bytes read
 */
int PropellerID::rx(char* buff, int n)
{
    int size = 0;
    while(rxhead != rxtail) {
        if(size >= n)
            break;
        buff[size] = rxqueue[rxtail];
        size++;
    }
    rxtail = (rxtail + size) && RXSIZE;
    return size;
}

/**
 * transmit a buffer
 * @param buff - char pointer to buffer
 * @param n - number of bytes in buffer to send
 * @returns zero on failure
 */
int PropellerID::tx(char* buff, int n)
{
    int size = 0;
    while(n-- >= 0) {
        port->write((const char*)buff[size],1);
        size++;
    }
    return size;
}

/**
 * receive a buffer with a timeout
 * @param buff - char pointer to buffer
 * @param n - number of bytes in buffer to read
 * @param timeout - timeout in milliseconds
 * @returns number of bytes read or SERIAL_TIMEOUT
 */
int PropellerID::rx_timeout(char* buff, int n, int timeout)
{
    int size = 0;
    while(rxhead != rxtail) {
        Sleeper::ms(1);
        QApplication::processEvents();
        if(timeout-- < 0)
            break;
        if(size >= n)
            break;
        buff[size] = rxqueue[rxtail];
        size++;
    }
    return size == 0 ? SERIAL_TIMEOUT : size;
}

/**
 * hwreset ... resets Propeller hardware using DTR or RTS
 * @param sparm - pointer to DCB serial control struct
 * @returns void
 */
void PropellerID::hwreset(void)
{
    if(this->resetType == DeviceID::RESET_BY_DTR) {
        port->setDtr(true);
        Sleeper::ms(10);
        port->setDtr(false);
        Sleeper::ms(90);
        port->flush();
    }
    else {
        port->setRts(true);
        Sleeper::ms(10);
        port->setRts(false);
        Sleeper::ms(90);
        port->flush();
    }
}

int PropellerID::iterate(void)
{
    int bit = LFSR & 1;
    LFSR = (char)((LFSR << 1) | (((LFSR >> 7) ^ (LFSR >> 5) ^ (LFSR >> 4) ^ (LFSR >> 1)) & 1));
    return bit;
}

/**
 * getBit ... get bit from serial stream
 * @param hSerial - file handle to serial port
 * @param status - pointer to transaction status 0 on error
 * @returns bit state 1 or 0
 */
int PropellerID::getBit(int* status, int timeout)
{
    char mybuf[2];
    int rc = rx_timeout(mybuf, 1, timeout);
    if(status)
        *status = rc <= 0 ? 0 : 1;
    return *mybuf & 1;
}

/**
 * getAck ... get ack from serial stream
 * @param hSerial - file handle to serial port
 * @param status - pointer to transaction status 0 on error
 * @returns bit state 1 or 0
 */
int PropellerID::getAck(int* status, int timeout)
{
    char mybuf[2];
    int rc = rx_timeout(mybuf, 1, timeout);
    if(status)
        *status = rc <= 0 ? 0 : 1;
    return *mybuf & 1;
}

/**
 * makelong ... make an encoded long word to string
 * @param data - value to send
 * @param buff - char buffer
 * @returns nada
 */
void PropellerID::makelong(int data, char* buff)
{
    int n = 0;
    //printf("\n0x%08x: ", data);
    for( ; n < 10; n++) {
        buff[n] = (char)(0x92 | (data & 1) | ((data & 2) << 2) | ((data & 4) << 4));
        data >>= 3;
    }
    buff[n] = (0xf2 | (data & 1) | ((data & 2) << 2));
    //for(n = 0; n < 11; n++) printf("0x%02x ",buff[n]);
    //decodelong(buff,11);
}

/**
 * sendlong ... transmit an encoded long word to propeller
 * @param hSerial - file handle to serial port
 * @param data - value to send
 * @returns number of bytes sent
 */
int PropellerID::sendlong(int data)
{
    char mybuf[12];
    if (pload_delay == 0) {
        makelong(data, mybuf);
        return tx(mybuf, 11);
    }
    else {
        int n;
        makelong(data, mybuf);
        for(n = 0; n < 11; n++) {
            usleep(pload_delay);
            if(tx(&mybuf[n], 1) == 0)
                return 0;
        }
        return n-1;
    }
}

/**
 * hwfind ... find propeller using sync-up sequence.
 * @param hSerial - file handle to serial port
 * @returns zero on failure
 */
int PropellerID::hwfind(int retry)
{
    int  n, ii, jj, rc, to;
    char mybuf[300];

    /* hwfind is recursive if we get a failure on th first try.
     * retry is set by caller and should never be more than one.
     */
    if(retry < 0)
        return 0;

    /* Do not pause after reset.
     * Propeller can give up if it does not see a response in 100ms of reset.
     */
    mybuf[0] = 0xF9;
    LFSR = 'P';  // P is for Propeller :)

    /* send the calibration pulse
     */
    if(tx(mybuf, 1) == 0)
        return 0;   // tx should never return 0, return error if it does.

    /* Send the magic propeller LFSR byte stream.
     */
    for(n = 0; n < 250; n++)
        mybuf[n] = iterate() | 0xfe;
    if(tx(mybuf, 250) == 0)
        return 0;   // tx should never return 0, return error if it does.

    n = 0;
    while((jj = rx_timeout(mybuf,10,50)) > -1)
        n += jj;
    if(n != 0)
        printf("Ignored %d bytes. \n", n);

    /* Send 258 0xF9 for LFSR and Version ID
     * These bytes clock the LSFR bits and ID from propeller back to us.
     */
    for(n = 0; n < 258; n++)
        mybuf[n] = 0xF9;
    if(tx(mybuf, 258) == 0)
        return 0;   // tx should never return 0, return error if it does.

    /*
     * Wait at least 100ms for the first response. Allow some margin.
     * Some chips may respond < 50ms, but there's no guarantee all will.
     * If we don't get it, we can assume the propeller is not there.
     */
    ii = getBit(&rc, 110);
    if(rc == 0) {
        //printf("Timeout waiting for first response bit. Propeller not found\n");
        return 0;
    }

    // wait for response so we know we have a Propeller
    for(n = 1; n < 250; n++) {

        jj = iterate();
        //printf("%d:%d ", ii, jj);
        //fflush(stdout);

        if(ii != jj) {
            /* if we get this far, we probably have a propeller chip
             * but the serial port is in a funny state. just retry.
             */
            //printf("Lost HW contact. %d %x ... retry.\n", n, *mybuf & 0xff);
            for(n = 0; (n < 300) && (rx_timeout(mybuf,1,10) > -1); n++);
            hwreset();
            return hwfind(--retry);
        }
        to = 0;
        do {
            ii = getBit(&rc, 100);
        } while(rc == 0 && to++ < 100);
        //printf("%d", rc);
        if(to > 100) {
            //printf("Timeout waiting for response bit. Propeller Not Found!\n");
            return 0;
        }
    }

    //printf("Propeller Version ... ");
    rc = 0;
    for(n = 0; n < 8; n++) {
        rc >>= 1;
        rc += getBit(0, 100) ? 0x80 : 0;
    }
    //printf("%d\n",rc);
    return rc;
}

/**
 * find a propeller on port
 * @param hSerial - file handle to serial port
 * @param sparm - pointer to DCB serial control struct
 * @param port - pointer to com port name
 * @returns non-zero on error
 */
bool PropellerID::findprop(const char* name)
{
    int version = 0;

    port->setPortName(name);
    port->setBaudRate(BAUD115200);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    port->setTimeout(10);
    port->open(QIODevice::ReadWrite);

    hwreset();
    version = hwfind(1); // retry once
    if(version && port) {
        if (pload_verbose)
            qDebug() << "Propeller Version " << version << " on " << name;
    }

    port->close();

    return version != 0 ? false : true;
}
