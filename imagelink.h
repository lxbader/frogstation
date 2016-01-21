#ifndef IMAGELINK_H
#define IMAGELINK_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QtEndian>
#include <QImage>

#include "stdint.h"

#define LOCAL_COMPORT "COM3"
#define BAUDRATE QSerialPort::Baud115200
#define DATABITS QSerialPort::Data8
#define PARITY QSerialPort::NoParity
#define STOPBITS QSerialPort::OneStop
#define FLOWCONTROL QSerialPort::NoFlowControl

class Imagelink : public QObject
{
    Q_OBJECT

public:
    QString consoleText;
    QImage currentImage;

    explicit Imagelink(QObject *parent = 0);
    void openPort();

signals:
    void updateConsole();
    void updateImage();

private:
    QSerialPort *bluetoothPort;
    void console(QString msg);
    QRgb getRgbValue(uint8_t y, uint8_t cb, uint8_t cr);

private slots:
    void readImage();
};

#endif // SERIALPORT_H
