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

void Imagelink::sendCommand(QString command){
    bluetoothPort->write(command.toLocal8Bit());
}

void Imagelink::readData(){
    console("Bluetooth package received.");
    QByteArray data = bluetoothPort->readAll();
    imageBuffer.append(data);
//    if(data.startsWith("FRAME START") && data.endsWith("FRAME STOP\n")){
//        data.remove(0, 11);
//        data.remove(data.length()-10, 10);
//        imageBuffer = data;
//        console(data);
//    }else if(data.startsWith("FRAME START")){
//        data.remove(0, 11);
//        imageBuffer = data;
//        imageTransmitActive = true;
//        console(data);
//    }else if(imageTransmitActive && data.endsWith("FRAME STOP\n")){
//        data.remove(data.length()-10, 10);
//        imageBuffer.append(data);
//        console(data);
//        imageTransmitActive = false;
//        console("Image transfer complete.");
//        readImage();
//    }else if(imageTransmitActive){
//        imageBuffer.append(data);
//        console(data);
//    }else{
//        console("Invalid data format.");
//        console(data);
//    }
    if(data.endsWith("\n")){
        console("End of transmission.");
        readImage();
    }
}

void Imagelink::readImage(){
    console("readImage() started");
    console(imageBuffer);

    QFile file("D:\\rawData.txt");
    file.open(QIODevice::WriteOnly);
    file.write(imageBuffer);
    file.close();

    console("\n\n");
//    if(( !imageBuffer.startsWith("FRAME START")) || (!imageBuffer.endsWith("FRAME STOP\n"))){
//        console("Data not fitting for image.");
//        return;
//    }
    imageBuffer.remove(0, 11);
    imageBuffer.remove(imageBuffer.length()-12, 12);

    QFile file2("D:\\rawData2.txt");
    file2.open(QIODevice::WriteOnly);
    file2.write(imageBuffer);
    file2.close();


//FRAME START
//[i] und [i+1] und [i+2] -> uint8_t
//FRAME STOP\n

    console(imageBuffer);

    if(imageBuffer.length() != 121*160*2*3){
        console("ERROR: Received image package size does not fit required size.");
    }

    //Extract linear int-array out of data
    QByteArray buffer;
    uint8_t orig[121*160*2];
    for(int i = 0; i<(121*160*2); i++){
        buffer.append(imageBuffer.at(3*i));
        buffer.append(imageBuffer.at(3*i+1));
        buffer.append(imageBuffer.at(3*i+2));
        orig[i] = (uint8_t) buffer.toInt();
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
