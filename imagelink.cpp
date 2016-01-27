#include "imagelink.h"

Imagelink::Imagelink(QObject *parent) : QObject(parent), consoleText(""), currentImage(QImage(160, 121, QImage::Format_RGB32)), imageTransmitActive(false), portOpen(false){
    bluetoothPort = new QSerialPort(this);
}

void Imagelink::initializePort(){
    connect(bluetoothPort, SIGNAL(readyRead()), this, SLOT(readData()));
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        list.append(PortInfo(info.portName(), false));
    }
    emit updateStatus();
}

void Imagelink::openPort(){
    QSerialPortInfo activePortInfo;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        if(activePortName == info.portName()){
            activePortInfo = info;
        }
    }
    if(activePortInfo.isNull()){
        console("ERROR: No port selected or port could not be found.");
        return;
    }
    bluetoothPort->setPort(activePortInfo);
//    bluetoothPort->setPortName(LOCAL_COMPORT);
    bluetoothPort->setBaudRate(BAUDRATE);
    bluetoothPort->setDataBits(DATABITS);
    bluetoothPort->setParity(PARITY);
    bluetoothPort->setStopBits(STOPBITS);
    bluetoothPort->setFlowControl(FLOWCONTROL);
    if(bluetoothPort->open(QIODevice::ReadWrite)){
        console(QString("Port \"%1\" opened.").arg(activePortName));
        portOpen = true;
        PortInfo activeInfo = PortInfo(activePortName, false);
        for(int i = 0; i < list.length(); i++){
            PortInfo listInfo = list.at(i);
            if(listInfo == activeInfo)
                list.replace(i, PortInfo(activePortName, true));
        }
        emit updateStatus();
    }
    else
        console(QString("ERROR: Port \"%1\" could not be opened.").arg(activePortName));
}

void Imagelink::closePort(){
    QSerialPortInfo activePortInfo;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        if(activePortName == info.portName()){
            activePortInfo = info;
        }
    }
    if(activePortInfo.isNull()){
        console("ERROR: No port selected or port could not be found.");
        return;
    }
    bluetoothPort->setPort(activePortInfo);
    bluetoothPort->close();
    console(QString("Port \"%1\" closed.").arg(activePortInfo.portName()));
    portOpen = false;
    PortInfo activeInfo = PortInfo(activePortName, true);
    for(int i = 0; i < list.length(); i++){
        PortInfo listInfo = list.at(i);
        if(listInfo == activeInfo)
            list.replace(i, PortInfo(activePortName, false));
    }
    emit updateStatus();
}

void Imagelink::sendData(const QByteArray &command){
    bluetoothPort->write(command);
}

void Imagelink::sendCommand(const Command &tc){
    QByteArray buffer(sizeof(Command), 0x00);
    memcpy(buffer.data(), (char*)&tc, sizeof(Command));
    console("Command information:");
    console(QString("ID: %1").arg(tc.id));
    console(QString("Identifier: %1").arg(tc.identifier));
    console(QString("Value: %1").arg(tc.value));
    sendData(buffer);
}

void Imagelink::readData(){
    //console("Bluetooth package received.");
    QByteArray data = bluetoothPort->readAll();
    console(data);
//    imageBuffer.append(data);
//    if(data.endsWith("\n")){
//        console("End of transmission.");
//        readImage();
//    }
}

void Imagelink::readImage(){

    //FRAME START
    //[i] and [i+1] and [i+2] -> uint8_t
    //FRAME STOP\n

    //Readings from maybe fucked up sample image file
    QFile file("D:\\picture.txt");
    file.open(QIODevice::ReadOnly);
    imageBuffer = file.readAll();
    file.close();

    //Save image in file for later tests
//    QFile file2("D:\\rawData3.txt");
//    file2.open(QIODevice::WriteOnly);
//    file2.write(imageBuffer);
//    file2.close();

    if((!imageBuffer.startsWith("FRAME START")) || (!imageBuffer.endsWith("\n"))){
        console("Data not fitting for image.");
        return;
    }

    //Remove flags
    imageBuffer.remove(0, 11);
    imageBuffer.remove(imageBuffer.length()-12, 12);

    //Check length
    if(imageBuffer.length() != IMAGE_PIXELS*2*3){
        console("ERROR: Received image package size does not fit required size.");
        return;
    }

    //Extract linear uint8-array out of data
    QByteArray buffer(3,0x00);
    uint8_t orig[IMAGE_PIXELS*2];
    for(int i = 0; i<(IMAGE_PIXELS*2); i++){
        buffer[0] = (imageBuffer.at(3*i));
        buffer[1] = (imageBuffer.at(3*i+1));
        buffer[2] = (imageBuffer.at(3*i+2));
        orig[i] = (uint8_t) buffer.toInt();
    }

    /*Should be alright till here!!!!*/

    //Convert linear YCbCr array to RBG image
    QImage rgb(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB32);
    uint8_t y = 0;
    uint8_t cb = 0;
    uint8_t cr = 0;

    //RGB (doesn't work?)
    for(int line = 0; line < IMAGE_HEIGHT; line++){
        for(int column = 0; column < IMAGE_WIDTH; column +=2){
            y   = orig[IMAGE_WIDTH*2*line + 2*column + 1];
            cb  = orig[IMAGE_WIDTH*2*line + 2*column + 0];
            cr  = orig[IMAGE_WIDTH*2*line + 2*column + 2];
            rgb.setPixel(column, line, getRgbValue(y, cb, cr));
            y   = orig[IMAGE_WIDTH*2*line + 2*column + 3];
            rgb.setPixel(column+1, line, getRgbValue(y, cb, cr));
        }
    }

    //Grayscale (not very different of course)
//    for(int line = 0; line < IMAGE_HEIGHT; line++){
//        for(int column = 0; column < IMAGE_WIDTH; column +=2){
//            y   = orig[IMAGE_WIDTH*2*line + 2*column + 1];
//            cb  = orig[IMAGE_WIDTH*2*line + 2*column + 0];
//            cr  = orig[IMAGE_WIDTH*2*line + 2*column + 2];
//            rgb.setPixel(column, line, qRgb(y, y, y));
//            y   = orig[IMAGE_WIDTH*2*line + 2*column + 3];
//            rgb.setPixel(column+1, line, qRgb(y, y, y));
//        }
//    }

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

bool Imagelink::isOpen(){
    PortInfo activeInfo = PortInfo(activePortName, false);
    for(int i = 0; i < list.length(); i++){
        PortInfo listInfo = list.at(i);
        if(listInfo == activeInfo)
            return listInfo.isOpen;
    }
    console("ERROR: Port problem.");
    return 0;
}

PortInfo::PortInfo(QString name, bool open){
    portName = name;
    isOpen = open;
}

bool PortInfo::operator==(PortInfo& test){
    if(this->portName == test.portName)
        return true;
    return false;
}
