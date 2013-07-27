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

#include "PropellerID.h"

PropellerID::PropellerID(QObject *parent) : QThread(parent)
{
    port = new QextSerialPort(QextSerialPort::Polling);
    pload_verbose = 0;
    rxhead = 0;
    rxtail = 0;
    resetType = RESET_BY_DTR;
    LFSR = 80; // 'P'
}

PropellerID::~PropellerID()
{
    delete port;
}

int PropellerID::isDevice(QString portName)
{
    return findprop(portName.toAscii());
}

void PropellerID::run()
{
    while(port->isOpen()) {
        msleep(25);
        QApplication::processEvents();
        emit portEvent();
    }
}

void PropellerID::portHandler()
{
    /*
     * TODO Queue is broken. Fix it later.
     * For the time being the current queue buffer is big enough to hold
     * all expected receive data + garbage from quickstart boards.
     */

    /*
     * [--t--h-] head >= tail, read size-(head-tail) into buffer
     * [-h-t---] head < tail, read head amount into buffer
     * [b------] head == tail
     * [t-----h] full
     * [-----ht]
     * [------b]
     */
    int size = port->bytesAvailable();
    if(size < 1) {
        return;
    }
#if 1
    if (rxhead >= rxtail) {
        size = RXSIZE-(rxhead-rxtail);
        size = port->read(&rxqueue[rxhead], size);
    } else {
        size = port->read(&rxqueue[rxhead], rxtail-rxhead-1);
    }
#else
    if (rxhead > rxtail) {
        size = RXSIZE-(rxhead-rxtail);
        size = port->read(&rxqueue[rxhead], size);
    } else if (rxhead < rxtail) {
        size = port->read(&rxqueue[0], rxtail - rxhead);
    } else {
        size = port->read(&rxqueue[0], RXSIZE);
    }
#endif
    rxhead = (rxhead + size) & RXSIZE;
    // if (pload_verbose) qDebug() << size << " bytes read H" << rxhead << "T" << rxtail;
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
    while(rxhead == rxtail)
        ;
    while(rxhead != rxtail) {
        if(size >= n)
            break;
        buff[size] = rxqueue[rxtail];
        size++;
        rxtail = (rxtail + size) & RXSIZE;
    }
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
    int size = port->write((const char*)buff, n);
    QApplication::processEvents();
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
    while(rxhead == rxtail) {
        msleep(1);
        QApplication::processEvents();
        if(timeout-- < 0)
            break;
    }
    while(timeout > 0 && rxhead != rxtail) {
        if(size >= n)
            break;
        buff[size] = rxqueue[rxtail];
        size++;
        rxtail = (rxtail + size) & RXSIZE;
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
    if(this->resetType == RESET_BY_DTR) {
        port->setDtr(true);
        msleep(10);
        port->setDtr(false);
        msleep(90);
        port->flush();
    }
    else {
        port->setRts(true);
        msleep(10);
        port->setRts(false);
        msleep(90);
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
    return 0;
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

    /* hwfind is recursive if we get a failure on the first try.
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

    //if(rx_timeout(mybuf,10,10) > -1) { qDebug("Junk chars ..."); }

    /* Send the magic propeller LFSR byte stream.
     */
    for(n = 0; n < 250; n++)
        mybuf[n] = iterate() | 0xfe;
    if(tx(mybuf, 250) == 0)
        return 0;   // tx should never return 0, return error if it does.

    n = 0;
    while((jj = rx_timeout(mybuf,10,10)) > -1) {
        n += jj;
    }
    if(n != 0) qDebug("Ignored %d bytes. \n", n);

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
        if (pload_verbose)
            qDebug("Timeout waiting for first response bit. Propeller not found.");
        return 0;
    }

    // wait for response so we know we have a Propeller
    for(n = 1; n < 250; n++) {

        jj = iterate();
        //if (pload_verbose) { printf("%d:%d %3d ", ii, jj, n); fflush(stdout); }

        if(ii != jj) {
            /* if we get this far, we probably have a propeller chip
             * but the serial port is in a funny state. just retry.
             */
            qDebug("Lost HW contact. %d %x ... retry.", n, *mybuf & 0xff);
            for(n = 0; (n < 300) && (rx_timeout(mybuf,1,10) > -1); n++);
            hwreset();
            return hwfind(--retry);
        }
        to = 0;
        do {
            ii = getBit(&rc, 100);
        } while(rc == 0 && to++ < 100);
        //printf(" %d\n", rc);
        if(to > 100) {
            qDebug("Timeout waiting for bit-stream response.");
            return 0;
        }
    }

    rc = 0;
    for(n = 0; n < 8; n++) {
        rc >>= 1;
        rc += getBit(0, 100) ? 0x80 : 0;
    }
    if (pload_verbose) qDebug("Propeller Version ... %d", rc);
    return rc;
}

void PropellerID::flushPort()
{
    int size = 0;
    do {
        msleep(5);
        QApplication::processEvents();
        size = port->bytesAvailable();
        //if (pload_verbose) qDebug("Flushing port %d", size);
        port->readAll();
    } while(size > 0);
}

/**
 * find a propeller on port
 * @param hSerial - file handle to serial port
 * @param sparm - pointer to DCB serial control struct
 * @param port - pointer to com port name
 * @returns non-zero on error
 */
int PropellerID::findprop(const char* name)
{
    int version = 0;

    if (pload_verbose)
        qDebug("\nChecking for Propeller on port %s", name);

    port->setPortName(name);
    port->setBaudRate(BAUD115200);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    port->setTimeout(10);
    if(port->open(QIODevice::ReadWrite) == false)
        return -1;

    flushPort();

    /*
     * TODO Queue is broken. Fix it later.
     * For the time being the current queue buffer is big enough to hold
     * all expected receive data + garbage from quickstart boards.
     */
    rxhead = 0;
    rxtail = 0;

    connect(this, SIGNAL(portEvent()), this, SLOT(portHandler()));
    start();
    hwreset();
    version = hwfind(1); // retry once
    disconnect(this, SIGNAL(portEvent()), this, SLOT(portHandler()));

    if (pload_verbose) {
        if(version) {
            qDebug() << "Propeller Version" << version << "on" << name;
        } else {
            qDebug() << "Propeller not found on" << name;
        }
    }
    port->close();

    return version != 0 ? 1 : 0;
}
