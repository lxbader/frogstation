#include "groundstation.h"
#include "ui_groundstation.h"

Groundstation::Groundstation(QWidget *parent) :
    QMainWindow(parent), link(this), imager(this),
    ui(new Ui::Groundstation)
{
    ui->setupUi(this);
    setWindowState(windowState() | Qt::WindowMaximized);

    connect(&link, SIGNAL(updateConsole()), this, SLOT(connectionUpdateConsole()));
    connect(&imager, SIGNAL(updateConsole()), this, SLOT(imagelinkUpdateConsole()));

    link.bind();
    link.addTopic(PayloadSensorIMUType);
    link.addTopic(PayloadCounterType);
    link.addTopic(PayloadElectricalType);
//    link.addTopic(PayloadImageType);
    connect(&link, SIGNAL(readReady()), this, SLOT(readoutConnection()));

    imager.initializePort();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ui->bluetoothComboBox->addItem(info.portName());
    }

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(telemetryCheck()));
    timer->start();

    connect(&imager, SIGNAL(updateStatus()), this, SLOT(updateBluetoothLED()));

    //---------------
    //Connect Buttons
    //---------------

    //Top Row
    connect(ui->openPortButton, SIGNAL(clicked()), this, SLOT(onOpenPortButtonClicked()));
    connect(ui->closePortButton, SIGNAL(clicked()), this, SLOT(onClosePortButtonClicked()));
    connect(ui->bluetoothComboBox, SIGNAL(activated(int)), this, SLOT(updateBluetoothLED()));
    connect(ui->restartWifiButton, SIGNAL(clicked()), this, SLOT(onRestartWifiButtonClicked()));
    connect(ui->emergencyOffButton, SIGNAL(clicked()), this, SLOT(onEmergencyOffButtonClicked()));

    //Manual Control Tab
    connect(ui->deployRacksButton, SIGNAL(clicked()), this, SLOT(onDeployRacksButtonClicked()));
    connect(ui->pullRacksButton, SIGNAL(clicked()), this, SLOT(onPullRacksButtonClicked()));
    connect(ui->stopRacksButton, SIGNAL(clicked()), this, SLOT(onStopRacksButtonClicked()));
    connect(ui->activateElectromagnetButton, SIGNAL(clicked()), this, SLOT(onActivateElectromagnetButtonClicked()));
    connect(ui->deactivateElectromagnetButton, SIGNAL(clicked()), this, SLOT(onDeactivateElectromagnetButtonClicked()));
    connect(ui->takePictureButton, SIGNAL(clicked()), this, SLOT(onTakePictureButtonClicked()));

    //Attitude Tab
    connect(ui->orientationSetButton, SIGNAL(clicked()), this, SLOT(onOrientationSetButtonClicked()));
    connect(ui->orientationResetButton, SIGNAL(clicked()), this, SLOT(onOrientationResetButtonClicked()));
    connect(ui->setRotationButton, SIGNAL(clicked()), this, SLOT(onSetRotationButtonClicked()));
    connect(ui->stopRotationButton, SIGNAL(clicked()), this, SLOT(onStopRotationButtonClicked()));

    //Sun Finder Tab
    connect(ui->sunFinderButton, SIGNAL(clicked()), this, SLOT(onSunFinderButtonClicked()));

    //Mission Tab
    connect(ui->missionStartButton, SIGNAL(clicked()), this, SLOT(onMissionStartButtonClicked()));
    connect(ui->missionAbortButton, SIGNAL(clicked()), this, SLOT(onMissionAbortButtonClicked()));

    connect(&imager, SIGNAL(updateImage()), this, SLOT(updateImage()));

    //Set up graph widgets
    setupGraphs();

    //Test Image
    //imager.readImage();
}

Groundstation::~Groundstation()
{
    delete ui;
}

//--------
//READOUTS
//--------

void Groundstation::readoutConnection(){
    ui->telemetryLED->setChecked(true);
    PayloadSatellite payload = link.read();
    switch(payload.topic){
    case PayloadSensorIMUType:{
        //console("Package of type \"IMU\" received.");
        PayloadSensorIMU psimu(payload);
        float currentWz = 360*(psimu.wz/(2*M_PI));
        float currentRoll = 360*(psimu.roll/(2*M_PI));
        float currentPitch = 360*(psimu.pitch/(2*M_PI));
        float currentHeading = 360-360*(psimu.yaw/(2*M_PI));

        ui->compassWidget->angle = currentHeading;
        ui->debrisMapWidget->angle = currentHeading;
        ui->rotationLCD->display(currentWz);
        ui->orientationLCD->display(currentHeading);
        ui->pitchLCD->display(currentPitch);
        ui->rollLCD->display(currentRoll);

        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        ui->rotationWidget->graph(0)->addData(key, currentWz);
        ui->rotationWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->rotationWidget->graph(0)->rescaleValueAxis();
        ui->rotationWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->rotationWidget->replot();

        ui->orientationWidget->graph(0)->addData(key, currentHeading);
        ui->orientationWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->orientationWidget->graph(0)->rescaleValueAxis();
        ui->orientationWidget->graph(1)->addData(key, psimu.mz);
        ui->orientationWidget->graph(1)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->orientationWidget->graph(1)->rescaleValueAxis(true);
        ui->orientationWidget->graph(2)->addData(key, psimu.az);
        ui->orientationWidget->graph(2)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->orientationWidget->graph(2)->rescaleValueAxis(true);
        ui->orientationWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->orientationWidget->replot();

        break;
    }
    case PayloadCounterType:{
        //console("Package of type \"Counter\" received.");
        PayloadCounter pscount(payload);
        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        break;
    }
    case PayloadElectricalType:{
        //console("Package of type \"Electrical\" received.");
        PayloadElectrical pelec(payload);
        ui->solarLCD->display((int) pelec.light);
        ui->lightsensorLED->setChecked(pelec.lightsensorOn);
        ui->electromagnetLED->setChecked(pelec.electromagnetOn);
        ui->thermalKnifeLED->setChecked(pelec.thermalKnifeOn);

        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        ui->sunFinderWidget->graph(0)->addData(key, (int) pelec.light);
        ui->sunFinderWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->sunFinderWidget->graph(0)->rescaleValueAxis();
        ui->sunFinderWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->sunFinderWidget->replot();
        break;
    }
//    case PayloadImageType:{
//        console("Package of type \"Image\" received.");
//        PayloadImage pimage(payload);
//        displayImage(pimage.image, ui->missionInputLabel);
//        break;
//    }
    default:
        break;
    }
    return;
}


//--------------------
//BUTTONS/TELECOMMANDS
//--------------------

//Telecommands
void Groundstation::telecommand(int ID, int identifier, int value){
    /*WIFI*/
    Command com(ID, identifier, value);
    link.connectionSendCommand(TELECOMMAND_TOPIC_ID, com);

    /*BLUETOOTH*/
//    imager.sendCommand(com);

    /*OLD BLUETOOTH TYPE BY HAND*/
//    imager.sendData(ui->HBALineEdit->text().toLocal8Bit());
}

//Top Row
void Groundstation::onOpenPortButtonClicked(){
    imager.activePortName = ui->bluetoothComboBox->currentText();
    imager.openPort();
}

void Groundstation::onClosePortButtonClicked(){
    imager.activePortName = ui->bluetoothComboBox->currentText();
    imager.closePort();
}

void Groundstation::onRestartWifiButtonClicked(){
    link.bind();
}

void Groundstation::onEmergencyOffButtonClicked(){
    console("EMERGENCY OFF: Disengaging all electrical components.");
    telecommand(ID_ELECTRICAL, 3001, 0); //Stop racks
//    telecommand(ID_ELECTRICAL, 3004, false, 0); //Turn off electromagnet
//    telecommand(ID_ELECTRICAL, 3005, false, 0); //Turn off thermal knife
//    telecommand(ID_ELECTRICAL, 3006, false, 0); //Stop main motor
}

//Manual Control Tab
void Groundstation::onDeployRacksButtonClicked(){
    console("Deploying racks...");
    telecommand(ID_ELECTRICAL, 3001, 1);
}

void Groundstation::onPullRacksButtonClicked(){
    console("Pulling in racks...");
    telecommand(ID_ELECTRICAL, 3001, -1);
}

void Groundstation::onStopRacksButtonClicked(){
    console("Stopping racks...");
    telecommand(ID_ELECTRICAL, 3001, 0);
}

void Groundstation::onActivateElectromagnetButtonClicked(){
//    console("Activating electromagnet.");
//    telecommand(ID_ELECTRICAL, 3004, true, 0);
}

void Groundstation::onDeactivateElectromagnetButtonClicked(){
//    console("Deactivating electromagnet.");
//    telecommand(ID_ELECTRICAL, 3004, false, 0);
}

void Groundstation::onTakePictureButtonClicked(){
//    console("Taking picture.");
//    telecommand(ID_PICTURE, 4001, true, 0);
}

//Attitude Tab
void Groundstation::onOrientationSetButtonClicked(){
//    float angle;
//    bool ok;
//    angle = ui->orientationLineEdit->text().toFloat(&ok);
//    if(ok && (360 >= angle) && (angle >= 0)){
//        console(QString("Setting orientation to %1 degrees.").arg(angle));
//        telecommand(ID_ATTITUDE, 2002, true, angle);
//    }
//    else
//        console("Orientation angle invalid.");
}

void Groundstation::onOrientationResetButtonClicked(){
//    console("Resetting to N-S-orientation.");
//    telecommand(ID_ATTITUDE, 2002, true, 0);
}

void Groundstation::onSetRotationButtonClicked(){
//    float angle;
//    bool ok;
//    angle = ui->rotationLineEdit->text().toFloat(&ok);
//    if(ok && (360 >= angle) && (angle >= -360)){
//        console(QString("Setting orientation to %1 degrees.").arg(angle));
//        telecommand(ID_ATTITUDE, 2001, true, angle);
//    }
//    else
//        console("Orientation angle invalid.");
}

void Groundstation::onStopRotationButtonClicked(){
//    console("Stopping rotation.");
//    telecommand(ID_ATTITUDE, 2001, true, 0);
}

//Mission Tab
void Groundstation::onSunFinderButtonClicked(){
//    console("Sun acquisition routine started.");
//    telecommand(ID_MISSION, 5001, true, 0);
}

void Groundstation::onMissionStartButtonClicked(){
//    console("Mission started.");
//    telecommand(ID_MISSION, 5002, true, 0);
}

void Groundstation::onMissionAbortButtonClicked(){
//    console("Mission stopped.");
//    telecommand(ID_MISSION, 5002, false, 0);
}

//------------
//GRAPH SETUPS
//------------
void Groundstation::setupGraphs(){
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(7); // and make a bit smaller for legend

    //Set up rotationWidget
    ui->rotationWidget->xAxis->setLabel("Current Time");
    ui->rotationWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->rotationWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->rotationWidget->xAxis->setAutoTickStep(false);
    ui->rotationWidget->xAxis->setTickStep(10);
    ui->rotationWidget->yAxis->setLabel("Rotation Velocity (deg/sec)");
    ui->rotationWidget->axisRect()->setupFullAxesBox();
//    ui->rotationWidget->legend->setVisible(true);
//    ui->rotationWidget->legend->setFont(legendFont);
//    ui->rotationWidget->legend->setBrush(QBrush(QColor(255,255,255,230)));
//    ui->rotationWidget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignLeft);

    ui->rotationWidget->addGraph();
    ui->rotationWidget->graph(0)->setPen(QPen(Qt::blue));
//    ui->rotationWidget->graph(0)->setName("Combined");
    ui->rotationWidget->addGraph();
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->rotationWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->rotationWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->rotationWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->rotationWidget->yAxis2, SLOT(setRange(QCPRange)));

    //Set up orientationWidget
    ui->orientationWidget->xAxis->setLabel("Current Time");
    ui->orientationWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->orientationWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->orientationWidget->xAxis->setAutoTickStep(false);
    ui->orientationWidget->xAxis->setTickStep(10);
    ui->orientationWidget->yAxis->setLabel("Heading (deg)");
    ui->orientationWidget->axisRect()->setupFullAxesBox();
    ui->orientationWidget->legend->setVisible(true);
    ui->orientationWidget->legend->setFont(legendFont);
    ui->orientationWidget->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->orientationWidget->legend->setIconSize(10,5);
    ui->orientationWidget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignLeft);

    ui->orientationWidget->addGraph();
    ui->orientationWidget->graph(0)->setPen(QPen(Qt::blue));
    ui->orientationWidget->graph(0)->setName("Combined");
    ui->orientationWidget->addGraph();
    ui->orientationWidget->graph(1)->setPen(QPen(Qt::red));
    ui->orientationWidget->graph(1)->setName("Magnetometer");
    ui->orientationWidget->addGraph();
    ui->orientationWidget->graph(2)->setPen(QPen(Qt::green));
    ui->orientationWidget->graph(2)->setName("Accelerometer");
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->orientationWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->orientationWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->orientationWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->orientationWidget->yAxis2, SLOT(setRange(QCPRange)));

//    //Set up sunFinderWidget
//    ui->sunFinderWidget->xAxis->setLabel("Angle");
//    ui->sunFinderWidget->xAxis->setAutoTickStep(false);
//    ui->sunFinderWidget->xAxis->setTickStep(90);
//    ui->sunFinderWidget->xAxis->setRange(0, 360);
//    ui->sunFinderWidget->yAxis->setLabel("Solar Voltage");
//    ui->sunFinderWidget->axisRect()->setupFullAxesBox();
//    ui->sunFinderWidget->addGraph();
//    ui->sunFinderWidget->graph(0)->setPen(QPen(Qt::blue));
//    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(ui->sunFinderWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->sunFinderWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->yAxis2, SLOT(setRange(QCPRange)));

    //Set up sunFinderWidget time-dependent
    ui->sunFinderWidget->xAxis->setLabel("Current Time");
    ui->sunFinderWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->sunFinderWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->sunFinderWidget->xAxis->setAutoTickStep(false);
    ui->sunFinderWidget->xAxis->setTickStep(5);
    ui->sunFinderWidget->yAxis->setLabel("Lightsensor Data");
    ui->sunFinderWidget->axisRect()->setupFullAxesBox();
    ui->sunFinderWidget->addGraph();
    ui->sunFinderWidget->graph(0)->setPen(QPen(Qt::blue));
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->sunFinderWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->sunFinderWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->yAxis2, SLOT(setRange(QCPRange)));
}

//--------------------
//CONSOLE TEXT UPDATES
//--------------------

void Groundstation::console(QString msg){
    ui->consoleWidget->writeString(msg);
}

void Groundstation::connectionUpdateConsole(){
    ui->consoleWidget->writeString(link.consoleText);
}

void Groundstation::imagelinkUpdateConsole(){
    ui->consoleWidget->writeString(imager.consoleText);
}

//---------------------
//DISPLAY UPDATED IMAGE
//---------------------

void Groundstation::updateImage(){
    QImage scaled = imager.currentImage.scaled(ui->missionInputLabel->width(),ui->missionInputLabel->height(),Qt::KeepAspectRatio);
    ui->missionInputLabel->setPixmap(QPixmap::fromImage(scaled));
}

//-----------
//LED UPDATES
//-----------

//disable telemetry LED when connection is lost
void Groundstation::telemetryCheck(){
    if((ui->telemetryLED->isChecked()) && (QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - key) >= 3){
        ui->telemetryLED->setChecked(false);
        console("No telemetry data available.");
    }
}

void Groundstation::updateBluetoothLED(){
    imager.activePortName = ui->bluetoothComboBox->currentText();
    ui->bluetoothLED->setChecked(imager.isOpen());
}
