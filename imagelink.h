#ifndef IMAGELINK_H
#define IMAGELINK_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QtEndian>
#include <QImage>
#include <QVector>
#include <QFile>

#include "stdint.h"

#define LOCAL_COMPORT "COM3"
#define BAUDRATE QSerialPort::Baud115200
#define DATABITS QSerialPort::Data8
#define PARITY QSerialPort::NoParity
#define STOPBITS QSerialPort::OneStop
#define FLOWCONTROL QSerialPort::NoFlowControl

struct PortInfo{
    QString portName;
    bool isOpen;
    PortInfo(QString name, bool open);
    bool operator==(PortInfo& test);
};

class Imagelink : public QObject
{
    Q_OBJECT

public:
    QString activePortName;
    QString consoleText;
    QImage currentImage;

    explicit Imagelink(QObject *parent = 0);
    void initializePort();
    void openPort();
    void closePort();
    void sendCommand(QString command);
    bool isOpen();

signals:
    void updateConsole();
    void updateImage();
    void updateStatus();

private:
    QSerialPort *bluetoothPort;
    QByteArray imageBuffer;
    bool imageTransmitActive;
    bool portOpen;
    QList<PortInfo> list;

    void console(QString msg);
    QRgb getRgbValue(uint8_t y, uint8_t cb, uint8_t cr);
    void readImage();

private slots:
    void readData();
};

#endif // SERIALPORT_H
