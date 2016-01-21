#include "imagelink.h"

#include "stdint.h"

Imagelink::Imagelink(QObject *parent) : QObject(parent), consoleText(""), currentImage(QImage(160, 121, QImage::Format_RGB32)){
    bluetoothPort = new QSerialPort(this);
}



void Imagelink::openPort(){
    connect(bluetoothPort, SIGNAL(readyRead()), this, SLOT(readImage()));
    QString activePortName = LOCAL_COMPORT;
    QSerialPortInfo activePortInfo;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        if(info.portName() == activePortName)
            activePortInfo = info;
    }
    if(activePortInfo.isNull()){
        console(QString("ERROR: Port \"%1\" could not be found.").arg(activePortName));
        return;
    }
    console(QString("Active port set to \"%1\"").arg(activePortName));
    bluetoothPort->setPort(activePortInfo);
    bluetoothPort->setPortName(activePortName);
    bluetoothPort->setBaudRate(BAUDRATE);
    bluetoothPort->setDataBits(DATABITS);
    bluetoothPort->setParity(PARITY);
    bluetoothPort->setStopBits(STOPBITS);
    bluetoothPort->setFlowControl(FLOWCONTROL);
    if(bluetoothPort->open(QIODevice::ReadWrite))
        console(QString("Port \"%1\" opened.").arg(activePortInfo.portName()));
    else
        console(QString("ERROR: Port \"%1\" could not be opened.").arg(activePortInfo.portName()));
}

void Imagelink::readImage(){
    QByteArray data = bluetoothPort->readAll();
    console("Bluetooth package received.");

    if(sizeof(data) != sizeof(uint8_t)*121*160*2){
        console("ERROR: Received image package size does not fit required size.");
        return;
    }

    //Extract linear int-array out of data
    uint8_t orig[121*160*2];
    for(int i = 0; i<(121*160*2); i++){
        orig[i] = data[i];
    }

    //Convert linear YCbCr array to RBG image
    QImage rgb(160, 121, QImage::Format_RGB32);
    uint8_t y = 0;
    uint8_t cb = 0;
    uint8_t cr = 0;

    for(int line = 0; line < 121; line++){
        for(int column = 0; column < 80; column++){
            y   = orig[320*line + 4*column + 1];
            cb  = orig[320*line + 4*column + 0];
            cr  = orig[320*line + 4*column + 2];
            rgb.setPixel(2*column, line, getRgbValue(y, cb, cr));
            y   = orig[320*line + 4*column + 3];
            rgb.setPixel(2*column+1, line, getRgbValue(y,cb,cr));
        }
    }
    currentImage = rgb;
    emit updateImage();
}

QRgb Imagelink::getRgbValue(uint8_t y, uint8_t cb, uint8_t cr){
    uint32_t r = (uint32_t) y + 1.402 * ((uint32_t) cr - 128);
    uint32_t g = (uint32_t) y - 0.34414 * ((uint32_t) cb - 128) - 0.71414 * ((uint32_t) cr - 128);
    uint32_t b = (uint32_t) y + 1.772 * ((uint32_t) cb - 128);
    return qRgb(r, g, b);
}

void Imagelink::console(QString msg){
    consoleText = msg;
    emit updateConsole();
}
