#ifndef IMAGELINK_H
#define IMAGELINK_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QtEndian>
#include <QImage>
#include <QVector>
#include <QFile>
#include<QColor>

#include "stdint.h"

#include "payload.h"

#define LOCAL_COMPORT "COM3"
#define BAUDRATE 921600
#define DATABITS QSerialPort::Data8
#define PARITY QSerialPort::NoParity
#define STOPBITS QSerialPort::OneStop
#define FLOWCONTROL QSerialPort::NoFlowControl

#define IMAGE_WIDTH 160
#define IMAGE_HEIGHT 121
#define IMAGE_PIXELS IMAGE_WIDTH*IMAGE_HEIGHT

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
    void sendData(const QByteArray &command);
    void sendCommand(const Command &tc);
    bool isOpen();
    void readImage();

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
//    void readImage();

private slots:
    void readData();
};

#endif // SERIALPORT_H
