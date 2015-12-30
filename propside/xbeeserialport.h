#ifndef XBEESERIALPORT_H
#define XBEESERIALPORT_H

#include <QObject.h>
#include <QHostAddress.h>
#include <QUdpSocket.h>

#define XBEE_APP_PORT 0xBEE
#define XBEE_SER_PORT 0x2616

// Define XBee WiFi's AT commands.
static const char *atCmd[] = {
    // NOTES: [R] - read only, [R/W] = read/write, [s] - string, [b] - binary number, [sb] - string or binary number
    /* xbData */              "\0\0",/* [Wb] write data stream */
    /* xbMacHigh */           "SH",  /* [Rb] XBee's Mac Address (highest 16-bits) */
    /* xbMacLow */            "SL",  /* [Rb] XBee's Mac Address (lowest 32-bits) */
    /* xbSSID */              "ID",  /* [Rs/Ws] SSID (0 to 31 printable ASCII characters) */
    /* xbIPAddr */            "MY",  /* [Rb/Wsb*] XBee's IP Address (32-bits; IPv4); *Read-only in DHCP mode */
    /* xbIPMask */            "MK",  /* [Rb/Wsb*] XBee's IP Mask (32-bits); *Read-only in DHCP mode */
    /* xbIPGateway */         "GW",  /* [Rb/Wsb*] XBee's IP Gateway (32-bits); *Read-only in DHCP mode */
    /* xbIPPort */            "C0",  /* [Rb/Wb] Xbee's UDP/IP Port (16-bits) */
    /* xbIPDestination */     "DL",  /* [Rb/Wsb] Xbee's serial-to-IP destination address (32-bits; IPv4) */
    /* xbNodeID */            "NI",  /* [Rs/Ws] Friendly node identifier string (20 printable ASCII characters) */
    /* xbMaxRFPayload */      "NP",  /* [Rb] Maximum RF Payload (16-bits; in bytes) */
    /* xbPacketingTimeout */  "RO",  /* [Rb/Wb] Inter-character silence time that triggers packetization (8-bits; in character times) */
    /* xbIO2Mode */           "D2",  /* [Rb/Wb] Designated reset pin (3-bits; 0=Disabled, 1=SPI_CLK, 2=Analog input, 3=Digital input, 4=Digital output low, 5=Digital output high) */
    /* xbIO4Mode */           "D4",  /* [Rb/Wb] Designated serial hold pin (3-bits; 0=Disabled, 1=SPI_MOSI, 2=<undefined>, 3=Digital input, 4=Digital output low, 5=Digital output high) */
    /* xbOutputMask */        "OM",  /* [Rb/Wb] Output mask for all I/O pins (each 1=output pin, each 0=input pin) (15-bits on TH, 20-bits on SMT) */
    /* xbOutputState */       "IO",  /* [Rb/Wb] Output state for all I/O pins (each 1=high, each 0=low) (15-bits on TH, 20-bits on SMT).  Period affected by updIO2Timer */
    /* xbIO2Timer */          "T2",  /* [Rb/Wb] I/O 2 state timer (100 ms units; $0..$1770) */
    /* xbIO4Timer */          "T4",  /* [Rb/Wb] I/O 4 state timer (100 ms units; $0..$1770) */
    /* xbSerialMode */        "AP",  /* [Rb/Wb] Serial mode (0=Transparent, 1=API wo/Escapes, 2=API w/Escapes) */
    /* xbSerialBaud */        "BD",  /* [Rb/Wb] serial baud rate ($1=2400, $2=4800, $3=9600, $4=19200, $5=38400, $6=57600, $7=INITIAL_BAUD_RATE, $8=230400, $9=460800, $A=921600) */
    /* xbSerialParity */      "NB",  /* [Rb/Wb] serial parity ($0=none, $1=even, $2=odd) */
    /* xbSerialStopBits */    "SB",  /* [Rb/Wb] serial stop bits ($0=one stop bit, $1=two stop bits) */
    /* xbRTSFlow */           "D6",  /* [Rb/Wb] RTS flow control pin (3-bits; 0=Disabled, 1=RTS Flow Control, 2=<undefined>, 3=Digital input, 4=Digital output low, 5=Digital output high) */
    /* xbSerialIP */          "IP",  /* [Rb/Wb] Protocol for serial service (0=UDP, 1=TCP) */
    /* xbFirmwareVer */       "VR",  /* [Rb] Firmware version.  Nibbles ABCD; ABC = major release, D = minor release.  B = 0 (standard release), B > 0 (variant release) (16-bits) */
    /* xbHardwareVer */       "HV",  /* [Rb] Hardware version.  Nibbles ABCD; AB = module type, CD = revision (16-bits) */
    /* xbHardwareSeries */    "HS",  /* [Rb] Hardware series. (16-bits?) */
    /* xbChecksum */          "CK",  /* [Rb] current configuration checksum (16-bits) */
    /* xbWrite */             "WR",  /* [W] write parameter values to non-volatile memory (use sparingly to preserve flash) */
    /* xbDestPort */          "DE"   /* [Rb/Wb] Xbee's serial UDP/IP Port (16-bits) */
};

typedef enum {
    xbData,
    xbMacHigh,
    xbMacLow,
    xbSSID,
    xbIPAddr,
    xbIPMask,
    xbIPGateway,
    xbIPPort,
    xbIPDestination,
    xbNodeID,
    xbMaxRFPayload,
    xbPacketingTimeout,
    xbIO2Mode,
    xbIO4Mode,
    xbOutputMask,
    xbOutputState,
    xbIO2Timer,
    xbIO4Timer,
    xbSerialMode,
    xbSerialBaud,
    xbSerialParity,
    xbSerialStopBits,
    xbRTSFlow,
    xbSerialIP,
    xbFirmwareVer,
    xbHardwareVer,
    xbHardwareSeries,
    xbChecksum,
    xbWrite,
    xbDestPort
} xbCommand;

struct appHeader {
    qint16 number1;		    // Can be any random number.
    qint16 number2;			// Must be number1 ^ 0x4242
    qint8 packetID;			// Reserved (use 0)
    qint8 encryptionPad;		// Reserved (use 0)
    qint8 commandID;			// $00 = Data, $02 = Remote Command, $03 = General Purpose Memory Command, $04 = I/O Sample
    qint8 commandOptions;     // Bit 0 : Encrypt (Reserved), Bit 1 : Request Packet ACK, Bits 2..7 : (Reserved)
};

struct txPacket {
    appHeader hdr;              // application packet header
    qint8 frameID;			// 1
    qint8 configOptions;		// 0 = Queue command only; must follow with AC command to apply changes, 2 = Apply Changes immediately
    char atCommand[2];		    // Command Name - Two ASCII characters that identify the AT command
};

struct rxPacket {
    appHeader hdr;              // application packet header
    qint8 frameID;			// 1
    char atCommand[2];		    // Command Name - Two ASCII characters that identify the AT command
    qint8 status;             // Command status (0 = OK, 1 = ERROR, 2 = Invalid Command, 3 = Invalid Parameter)
};

struct XbeeInfo {
    qint32 ipAddr;
    qint32 macAddrHigh;
    qint32 macAddrLow;
    qint32 firmwareVersion;
    qint32 cfgChecksum;
    QString nodeID;
};

typedef enum {
    serialUDP = 0,
    serialTCP
} ipModes;

typedef enum {
    pinDisabled = 0,
    pinEnabled,
    pinAnalog,
    pinInput,
    pinOutLow,
    pinOutHigh,
} ioKinds;

typedef enum {
    transparentMode = 0,
    apiWoEscapeMode,
    apiWEscapeMode
} serialModes;

typedef enum {
    parityNone = 0,
    parityEven,
    parityOdd
} serialParity;

typedef enum {
    stopBits1 = 0,
    stopBits2
} stopBits;

class XBeeSerialPort : public QObject
{
    Q_OBJECT
public:
    XBeeSerialPort(QObject *parent = 0);

    bool open(QHostAddress addr, qint64 baudrate);
    bool isOpen();
    bool isSequential() const;
    void close();
    void flush();

    qint64 bytesAvailable() const;
    QByteArray readAll();
    qint64 write(QByteArray barry, int len);

    void setBaudRate(qint64 baudrate);
    qint64 baudRate() const;
    void setPortName(QString name);
    QString portName() const;
private:
    int setItem(xbCommand cmd, const QString value);
    int setItem(xbCommand cmd, int value);
    int sendRemoteCommand(xbCommand cmd, txPacket *tx, int txCnt, rxPacket *rx, int rxSize);

signals:

public slots:
    void readyRead();

private:
    QString port;
    int  baud;
    bool isopen;

    qint32 remoteaddr;
    qint32 myhostaddr;
    QUdpSocket socket;
    QByteArray rxdata;
};

#endif // XBEESERIALPORT_H
