#include "groundstation.h"
#include "ui_groundstation.h"
#include "compass.h"
#include "debrismap.h"
#include "qcustomplot.h"

Groundstation::Groundstation(QWidget *parent) :
    QMainWindow(parent), link(this),
    ui(new Ui::Groundstation)
{
    ui->setupUi(this);

    connect(&link, SIGNAL(updateConsole()), this, SLOT(connectionUpdateConsole()));

    link.bind();
    link.addTopic(PayloadSensorIMUType);
    link.addTopic(PayloadCounterType);
    link.addTopic(PayloadElectricalType);

    connect(&link, SIGNAL(readReady()), this, SLOT(readoutConnection()));

    currentRoll = 0;
    currentPitch = 0;
    currentYaw = 0;
    currentWz = 0;
    currentLight = 0;

    telemetryActive = 1;
    electromagnetActive = 0;
    thermalKnifeActive = 0;
    lightsensorActive = 0;

    //Set up timer + update
    QTimer *fastTimer = new QTimer(this);
    fastTimer->setInterval(0);
    connect(fastTimer, SIGNAL(timeout()), this, SLOT(fastUpdate()));
    fastTimer->start();

    QTimer *slowTimer = new QTimer(this);
    slowTimer->setInterval(300);
    connect(slowTimer, SIGNAL(timeout()), this, SLOT(slowUpdate()));
    slowTimer->start();

    //---------------
    //Connect Buttons
    //---------------

    //Top Row
    connect(ui->connectionSendButton, SIGNAL(clicked()), this, SLOT(onConnectionSendButtonClicked()));
    connect(ui->activateTelemetryButton, SIGNAL(clicked()), this, SLOT(onActivateTelemetryButtonClicked()));
    connect(ui->deactivateTelemetryButton, SIGNAL(clicked()), this, SLOT(onDeactivateTelemetryButtonClicked()));

    //General Tab
    connect(ui->calibrateMagnetometerButton, SIGNAL(clicked()), this, SLOT(onCalibrateMagnetometerButtonClicked()));
    connect(ui->calibrateAccelerometerButton, SIGNAL(clicked()), this, SLOT(onCalibrateAccelerometerButtonClicked()));
    connect(ui->calibrateGyroscopeButton, SIGNAL(clicked()), this, SLOT(onCalibrateGyroscopeButtonClicked()));

    connect(ui->setHBAButton, SIGNAL(clicked()), this, SLOT(onSetHBAButtonClicked()));
    connect(ui->setHBBButton, SIGNAL(clicked()), this, SLOT(onSetHBBButtonClicked()));
    connect(ui->setHBCButton, SIGNAL(clicked()), this, SLOT(onSetHBCButtonClicked()));

    connect(ui->activateThermalKnifeButton, SIGNAL(clicked()), this, SLOT(onActivateThermalKnifeButtonClicked()));
    connect(ui->deactivateThermalKnifeButton, SIGNAL(clicked()), this, SLOT(onDeactivateThermalKnifeButtonClicked()));
    connect(ui->activateElectromagnetButton, SIGNAL(clicked()), this, SLOT(onActivateElectromagnetButtonClicked()));
    connect(ui->deactivateElectromagnetButton, SIGNAL(clicked()), this, SLOT(onDeactivateElectromagnetButtonClicked()));
    connect(ui->activateLightsensorButton, SIGNAL(clicked()), this, SLOT(onActivateLightsensorButtonClicked()));
    connect(ui->deactivateLightsensorButton, SIGNAL(clicked()), this, SLOT(onDeactivateLightsensorButtonClicked()));

    //Rotation Tab
    connect(ui->rotationVelocityButton, SIGNAL(clicked()), this, SLOT(onRotationVelocityButtonClicked()));

    //OrientationTab
    connect(ui->orientationSetButton, SIGNAL(clicked()), this, SLOT(onOrientationSetButtonClicked()));
    connect(ui->orientationResetButton, SIGNAL(clicked()), this, SLOT(onOrientationResetButtonClicked()));

    //Sun Finder Tab
    connect(ui->sunFinderButton, SIGNAL(clicked()), this, SLOT(onSunFinderButtonClicked()));

    //Mission Tab
    connect(ui->missionStartButton, SIGNAL(clicked()), this, SLOT(onMissionStartButtonClicked()));
    connect(ui->missionAbortButton, SIGNAL(clicked()), this, SLOT(onMissionAbortButtonClicked()));



    //Set up graph widgets
    setupGraphs();

//    //Set up debrisMapWidget with sample points
//    ui->debrisMapWidget->addDebris(45);
//    ui->debrisMapWidget->addDebris(160);
//    ui->debrisMapWidget->addDebris(215.6);
//    ui->debrisMapWidget->debrisCleaned->append(QPoint(30,30));
}

Groundstation::~Groundstation()
{
    delete ui;
}

//--------
//READOUTS
//--------

void Groundstation::readoutConnection(){
//    console("Package handed on to groundstation.");
    PayloadSatellite payload = link.read();
    switch(payload.topic){
    case PayloadSensorIMUType:{
//        console("Package of type \"IMU\" received.");
        PayloadSensorIMU psimu(payload);        
        currentRoll = 360*(psimu.roll/(2*M_PI));
        currentPitch = 360*(psimu.pitch/(2*M_PI));
        currentYaw = 360*(psimu.yaw/(2*M_PI));
        currentWz = 360*(psimu.wz/(2*M_PI));
//        console(QString("Current Orientation: %1").arg(currentYaw));
        break;
    }
    case PayloadCounterType:{
//        console("Package of type \"Counter\" received.");
        PayloadCounter pscount(payload);
        ui->counterLCD->display(pscount.counter);
        break;
    }
    case PayloadElectricalType:{
//        console("Package of type \"Electrical\" received.");
        PayloadElectrical pelec(payload);
        electromagnetActive = pelec.electromagnetOn;
        thermalKnifeActive = pelec.thermalKnifeOn;
        lightsensorActive = pelec.lightsensorOn;
        currentLight = pelec.light;
        break;
    }
    default:
        break;
    }
    return;
}


//--------------------
//BUTTONS/TELECOMMANDS
//--------------------

//Telecommands
void Groundstation::telecommand(int ID, QString msg, int value){
    QString command;
    command = QString("$%1%2%3#").arg(QString::number(ID), msg, QString::number(value));
    link.connectionSendCommand(command);
}

void Groundstation::telecommand(int ID, QString msg, QString sign, int value){
    QString command;
    command = QString("$%1").arg(QString::number(ID));
    command.append(msg);
    command.append(sign);
    command.append(QString("%1#").arg(QString::number(value)));
    link.connectionSendCommand(command);
}

//Top Row
void Groundstation::onConnectionSendButtonClicked(){
    QString message = ui->connectionLineEdit->text();
    link.connectionSendCommand(message);
}

void Groundstation::onActivateTelemetryButtonClicked(){
    if(!telemetryActive){
        console("Activating telemetry.");
        telecommand(5, "TEL", 1);
        telemetryActive = 1;
    }
    else
        console("Telemetry already active.");
}

void Groundstation::onDeactivateTelemetryButtonClicked(){
    if(telemetryActive){
        console("Deactivating telemetry.");
        telecommand(5, "TEL", 0);
        telemetryActive = 0;
    }
    else
        console("Telemetry already inactive.");
}

//General Tab
void Groundstation::onCalibrateMagnetometerButtonClicked(){
    console("Magnetometer calibration started.");
    telecommand(1, "CMA", 1);
}

void Groundstation::onCalibrateAccelerometerButtonClicked(){
    console("Accelerometer calibration started.");
    telecommand(1, "CAC", 1);
}

void Groundstation::onCalibrateGyroscopeButtonClicked(){
    console("Gyroscope calibration started.");
    telecommand(1, "CGY", 1);
}

void Groundstation::onSetHBAButtonClicked(){
    int speedPercent;
    bool ok;
    speedPercent = ui->HBALineEdit->text().toInt(&ok, 10);
    if(ok && (100 >= speedPercent) && (speedPercent >= 0)){
        console(QString("Setting HBridge A motor speed to %1 percent.").arg(speedPercent));
        telecommand(3, "SPA", "+", abs(speedPercent));
    }
    else if(ok && (0 > speedPercent) && (speedPercent >= -100)){
        console(QString("Setting HBridge A motor speed to %1 percent.").arg(speedPercent));
        telecommand(3, "SPA", "-", abs(speedPercent));
    }
    else
        console("Given motor speed percentage invalid.");
}

void Groundstation::onSetHBBButtonClicked(){
    int speedPercent;
    bool ok;
    speedPercent = ui->HBBLineEdit->text().toInt(&ok, 10);
    if(ok && (100 >= speedPercent) && (speedPercent >= 0)){
        console(QString("Setting HBridge B motor speed to %1 percent.").arg(speedPercent));
        telecommand(3, "SPB", "+", abs(speedPercent));
    }
    else if(ok && (0 > speedPercent) && (speedPercent >= -100)){
        console(QString("Setting HBridge B motor speed to %1 percent.").arg(speedPercent));
        telecommand(3, "SPB", "-", abs(speedPercent));
    }
    else
        console("Given motor speed percentage invalid.");
}

void Groundstation::onSetHBCButtonClicked(){
    int speedPercent;
    bool ok;
    speedPercent = ui->HBCLineEdit->text().toInt(&ok, 10);
    if(ok && (100 >= speedPercent) && (speedPercent >= 0)){
        console(QString("Setting HBridge C motor speed to %1 percent.").arg(speedPercent));
        telecommand(3, "SPC", "+", abs(speedPercent));
    }
    else if(ok && (0 > speedPercent) && (speedPercent >= -100)){
        console(QString("Setting HBridge C motor speed to %1 percent.").arg(speedPercent));
        telecommand(3, "SPC", "-", abs(speedPercent));
    }
    else
        console("Given motor speed percentage invalid.");
}

void Groundstation::onActivateThermalKnifeButtonClicked(){
    if(!thermalKnifeActive){
        console("Activating thermal knife.");
        telecommand(3, "AKN", 1);
    }
    else
        console("Thermal knife already active.");
}

void Groundstation::onDeactivateThermalKnifeButtonClicked(){
    if(thermalKnifeActive){
        console("Deactivating thermal knife.");
        telecommand(3, "AKN", 0);
    }
    else
        console("Thermal knife already inactive.");
}

void Groundstation::onActivateElectromagnetButtonClicked(){
    if(!electromagnetActive){
        console("Activating electromagnet.");
        telecommand(3, "AMA", 1);
    }
    else
        console("Electromagnet already active.");
}

void Groundstation::onDeactivateElectromagnetButtonClicked(){
    if(electromagnetActive){
        console("Deactivating electromagnet.");
        telecommand(3, "AMA", 0);
    }
    else
        console("Electromagnet already inactive.");
}

void Groundstation::onActivateLightsensorButtonClicked(){
    if(!lightsensorActive){
        console("Activating lightsensor.");
        telecommand(3, "ALS", 1);
    }
    else
        console("Lightsensor already active.");
}

void Groundstation::onDeactivateLightsensorButtonClicked(){
    if(lightsensorActive){
        console("Deactivating lightsensor.");
        telecommand(3, "ALS", 0);
    }
    else
        console("Lightsensor already inactive.");
}

//Rotation Tab
void Groundstation::onRotationVelocityButtonClicked(){

}

//Orientation Tab
void Groundstation::onOrientationSetButtonClicked(){
    int angle;
    bool ok;
    angle = ui->orientationLineEdit->text().toInt(&ok, 10);
    if(ok && (360 >= angle) && (angle >= 0)){
        console(QString("Setting orientation to %1 degrees.").arg(angle));
        telecommand(3, "STE", angle);
    }
    else
        console("Orientation angle invalid.");
}

void Groundstation::onOrientationResetButtonClicked(){
    console("Resetting to N-S-orientation.");
    telecommand(3, "STE", 0);
}

//Sun Finder Tab
void Groundstation::onSunFinderButtonClicked(){

}

//Mission Tab
void Groundstation::onMissionStartButtonClicked(){

}

void Groundstation::onMissionAbortButtonClicked(){

}


//----------------
//UPDATE FUNCTIONS
//----------------
void Groundstation::fastUpdate(){
        //Compass and debris map update
        ui->compassWidget->angle = currentYaw;
        ui->debrisMapWidget->angle = currentYaw;

        //rotationVelocityWidget update
        double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        static double lastPointKey = 0;
        if (key-lastPointKey > 0.01){ // at most add point every 10 ms
            ui->rotationVelocityWidget->graph(0)->addData(key, currentWz);
            ui->rotationVelocityWidget->graph(0)->removeDataBefore(key-30);
            ui->rotationVelocityWidget->graph(0)->rescaleValueAxis();
            lastPointKey = key;
        }
        ui->rotationVelocityWidget->xAxis->setRange(key+0.25, 30, Qt::AlignRight); //make x-axis range scroll with the data (at a constant range size of 15sec)
        ui->rotationVelocityWidget->replot();

        //sunFinderWidget update time-dependent
        double key2 = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        static double lastPointKey2 = 0;
        if (key-lastPointKey2 > 0.01){ // at most add point every 10 ms
            ui->sunFinderWidget->graph(0)->addData(key2, currentLight);
            ui->sunFinderWidget->graph(0)->removeDataBefore(key2-30);
            ui->sunFinderWidget->graph(0)->rescaleValueAxis();
            lastPointKey2 = key2;
        }
        ui->sunFinderWidget->xAxis->setRange(key2+0.25, 30, Qt::AlignRight); //make x-axis range scroll with the data (at a constant range size of 15sec)
        ui->sunFinderWidget->replot();

        //LED Updates
        ui->lightsensorLED->setChecked(lightsensorActive);
        ui->electromagnetLED->setChecked(electromagnetActive);
        ui->thermalKnifeLED->setChecked(thermalKnifeActive);
        ui->telemetryLED->setChecked(telemetryActive);
}

void Groundstation::slowUpdate(){
    //LCD updates
    if(telemetryActive){
        ui->rotationVelocityLCD->display(currentWz);
        ui->orientationLCD->display(currentYaw);
        ui->pitchLCD->display(currentPitch);
        ui->rollLCD->display(currentRoll);
        ui->solarLCD->display(currentLight);
//        ui->debrisFoundLCD->display(ui->debrisMapWidget->debrisFound->length()+ui->debrisMapWidget->debrisCleaned->length());
//        ui->debrisCleanedLCD->display(ui->debrisMapWidget->debrisCleaned->length());

    }
}


//------------
//GRAPH SETUPS
//------------
void Groundstation::setupGraphs(){
    //Set up rotationVelocityWidget
    ui->rotationVelocityWidget->xAxis->setLabel("Current Time");
    ui->rotationVelocityWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->rotationVelocityWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->rotationVelocityWidget->xAxis->setAutoTickStep(false);
    ui->rotationVelocityWidget->xAxis->setTickStep(5);
    ui->rotationVelocityWidget->yAxis->setLabel("Rotation Velocity");
    ui->rotationVelocityWidget->axisRect()->setupFullAxesBox();
    ui->rotationVelocityWidget->addGraph();
    ui->rotationVelocityWidget->graph(0)->setPen(QPen(Qt::blue));
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->rotationVelocityWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->rotationVelocityWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->rotationVelocityWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->rotationVelocityWidget->yAxis2, SLOT(setRange(QCPRange)));

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
    ui->sunFinderWidget->yAxis->setLabel("Solar Voltage");
    ui->sunFinderWidget->axisRect()->setupFullAxesBox();
    ui->sunFinderWidget->addGraph();
    ui->sunFinderWidget->graph(0)->setPen(QPen(Qt::blue));
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->sunFinderWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->sunFinderWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->yAxis2, SLOT(setRange(QCPRange)));
}


//---------------
//CONSOLE UPDATES
//---------------

void Groundstation::console(QString msg){
    ui->consoleWidget->writeString(msg);
}

void Groundstation::connectionUpdateConsole(){
    ui->consoleWidget->writeString(link.consoleText);
}
