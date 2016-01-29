#include "imagelink.h"

Imagelink::Imagelink(QObject *parent) : QObject(parent), consoleText(""), currentImage(QImage(160, 121, QImage::Format_RGB32)), imageTransmitActive(false), portOpen(false){
    bluetoothPort = new QSerialPort(this);
}


void Imagelink::initializePort(){
    connect(bluetoothPort, SIGNAL(readyRead()), this, SLOT(readData()));

    /*Make internal list of available ports, all inactive*/
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
    bluetoothPort->setBaudRate(BAUDRATE);
    bluetoothPort->setDataBits(DATABITS);
    bluetoothPort->setParity(PARITY);
    bluetoothPort->setStopBits(STOPBITS);
    bluetoothPort->setFlowControl(FLOWCONTROL);
    if(bluetoothPort->open(QIODevice::ReadWrite)){
        console(QString("Port \"%1\" opened.").arg(activePortName));
        portOpen = true;
        PortInfo activeInfo = PortInfo(activePortName, false);

        /*Search selected and opened port in list of available ports and set opened port to active*/
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
    /*Set port inactive in list of available ports*/
    for(int i = 0; i < list.length(); i++){
        PortInfo listInfo = list.at(i);
        if(listInfo == activeInfo)
            list.replace(i, PortInfo(activePortName, false));
    }
    emit updateStatus();
}


/*Sending data (not used)*/
void Imagelink::sendData(const QByteArray &command){
    bluetoothPort->write(command);
}

/*Sending commands (not used)*/
void Imagelink::sendCommand(const Command &tc){
    QByteArray buffer(sizeof(Command), 0x00);
    memcpy(buffer.data(), (char*)&tc, sizeof(Command));
//    console("Command information:");
//    console(QString("ID: %1").arg(tc.id));
//    console(QString("Identifier: %1").arg(tc.identifier));
//    console(QString("Value: %1").arg(tc.value));
    sendData(buffer);
}


/*Reading out port*/
void Imagelink::readData(){
    QByteArray data = bluetoothPort->readAll();
    imageBuffer.append(data);
    /*all used flags are designed as "&<start flag>DATA<end flag>&"*/
    if(data.endsWith("&")){
        evaluateBuffer();
    }
}


/*Check flags*/
void Imagelink::evaluateBuffer(){
    /*Remove everything before flag and the & at flag beginning to get rid of some error messages*/
    int x = imageBuffer.lastIndexOf("&", imageBuffer.length()-2);
    imageBuffer.remove(0, x+1);

    /*Remove last & char*/
    imageBuffer.remove(imageBuffer.length()-1, 1);

    /*Check whether it's an image*/
    /*Start flag "&FRAME START", end flag "FRAME STOP&"*/
    /*& chars already removed*/
    if((imageBuffer.startsWith("FRAME START")) && (imageBuffer.endsWith("FRAME STOP"))){
        console("Image received.");
        /*Remove rest of flags*/
        imageBuffer.remove(0, 11);
        imageBuffer.remove(imageBuffer.length()-10, 10);
        readImage();
        return;
    }

    /*Check whether it's a console text*/
    /*Start flag "&CONSOLE START", end flag "CONSOLE STOP&"*/
    /*& chars already removed*/
    if((imageBuffer.startsWith("CONSOLE START")) && (imageBuffer.endsWith("CONSOLE STOP"))){
        /*Remove rest of flags*/
        imageBuffer.remove(0, 13);
        imageBuffer.remove(imageBuffer.length()-12, 12);
        console(imageBuffer);
        imageBuffer.clear();
        return;
    }
    console("Bluetooth message dropped due to incomplete flags.");
    imageBuffer.clear();
}


void Imagelink::readImage(){
    /*Readings from saved imageBuffer file*/
//    QFile file("D:\\SPACEMASTER\\SFPICS\\new_picture.txt");
//    file.open(QIODevice::ReadOnly);
//    imageBuffer = file.readAll();
//    file.close();

    /*Save imageBuffer in file for later tests*/
//    int i = QDateTime::currentDateTime().toMSecsSinceEpoch();
//    QFile file2(QString("D:\\SPACEMASTER\\SFPICS\\image_%1.txt").arg(abs(i)));
//    file2.open(QIODevice::WriteOnly);
//    file2.write(imageBuffer);
//    file2.close();

    /*Check length*/
    if(imageBuffer.length() != IMAGE_PIXELS*2*3){
        console("ERROR: Received image package size does not fit required size.");
        imageBuffer.clear();
        return;
    }

    /*Extract linear uint8-array out of data*/
    QByteArray buffer(3,0x00);
    uint8_t orig[IMAGE_PIXELS*2];
    for(int i = 0; i<(IMAGE_PIXELS*2); i++){
        buffer[0] = (imageBuffer.at(3*i));
        buffer[1] = (imageBuffer.at(3*i+1));
        buffer[2] = (imageBuffer.at(3*i+2));
        orig[i] = (uint8_t) buffer.toInt();
    }

    imageBuffer.clear();

    /*Initialize RGB/Grayscale image*/
    QImage rgb(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB32);
    uint8_t y1 = 0;
    uint8_t y2 = 0;
//    uint8_t cb = 0;
//    uint8_t cr = 0;

    /*Combine to RGB-/Grayscale image*/
    for(int line = 0; line < IMAGE_HEIGHT; line++){
        for(int column = 0; column < IMAGE_WIDTH; column +=2){
            y1   = orig[IMAGE_WIDTH*2*line + 2*column + 0];
//            cb  = orig[IMAGE_WIDTH*2*line + 2*column + 1];
            y2   = orig[IMAGE_WIDTH*2*line + 2*column + 2];
//            cr  = orig[IMAGE_WIDTH*2*line + 2*column + 3];

            /*RGB (doesn't work for whatever reason)*/
//            rgb.setPixel(column, line, getRgbValue(y1, cb, cr));
//            rgb.setPixel(column+1, line, getRgbValue(y2, cb, cr));

            /*Grayscale (works fine)*/
            rgb.setPixel(column, line, qRgb(y1, y1, y1));
            rgb.setPixel(column+1, line, qRgb(y2, y2, y2));
        }
    }

    /*Sven's grayscale picture (working)*/
//    QByteArray buffer(3,0x00);
//    uint8_t orig[IMAGE_WIDTH][IMAGE_HEIGHT];
//    for(int lines = 0; lines<121; lines++){
//        for(int columns = 0; columns<160 ; columns++){
//            buffer[0] = (imageBuffer.at(4*columns+642*lines));
//            buffer[1] = (imageBuffer.at(4*columns+642*lines+1));
//            buffer[2] = (imageBuffer.at(4*columns+642*lines+2));
//            orig[columns][lines] = (uint8_t) buffer.toInt();
//            console(buffer);
//        }
//    }
//    for(int i = 0; i<256; i++){
//        QRgb value = qRgb(i,i,i);
//        rgb.setColor(i, value);
//    }
//    for(int line = 0; line < IMAGE_HEIGHT; line++){
//        for(int column = 0; column < IMAGE_WIDTH; column++){
//            y   = orig[column][line];
//            rgb.setPixel(column, line, y);

//        }
//    }

    /*Update image*/
    currentImage = rgb;
    emit updateImage();
}


/*-------------------------------------------*/
/*YCbCr-RGB conversions. None really working */
/*-------------------------------------------*/

/*Wikipedia JPEG convention*/
//QRgb Imagelink::getRgbValue(uint8_t y, uint8_t cb, uint8_t cr){
//    uint32_t r = (uint32_t) y + 1.402 * ((uint32_t) cr - 128);
//    uint32_t g = (uint32_t) y - 0.34414 * ((uint32_t) cb - 128) - 0.71414 * ((uint32_t) cr - 128);
//    uint32_t b = (uint32_t) y + 1.772 * ((uint32_t) cb - 128);
//    return qRgb(r, g, b);
//}


/*Wikipedia some other convention*/
QRgb Imagelink::getRgbValue(uint8_t y, uint8_t cb, uint8_t cr){
    uint32_t r = (uint32_t) (255/219)*(y-16) + (255/112)*0.701* ((uint32_t) cr - 128);
    uint32_t g = (uint32_t) (255/219)*(y-16) - (255/112)*0.886*(0.114/0.587)*(cb-128) - (255/112)*0.701* (0.299/0.587)*((uint32_t) cr - 128);
    uint32_t b = (uint32_t) (255/219)*(y-16) + (255/112)*0.886*(cb-128) ;
    return qRgb(r, g, b);
}


/*Printing text into console*/
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


/*struct for storing a list of available with their current active/inactive status*/
PortInfo::PortInfo(QString name, bool open){
    portName = name;
    isOpen = open;
}


/*Overwrite == operator to compare objects of type PortInfo more easily*/
bool PortInfo::operator==(PortInfo& test){
    if(this->portName == test.portName)
        return true;
    return false;
}
