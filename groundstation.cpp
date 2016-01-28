#include "groundstation.h"
#include "ui_groundstation.h"

Groundstation::Groundstation(QWidget *parent) :
    QMainWindow(parent), link(this), imager(this),
    ui(new Ui::Groundstation)
{
    ui->setupUi(this);

    /*Maximize window*/
    setWindowState(windowState() | Qt::WindowMaximized);

    /*Set up console updates from lower classes*/
    connect(&link, SIGNAL(updateConsole()), this, SLOT(connectionUpdateConsole()));
    connect(&imager, SIGNAL(updateConsole()), this, SLOT(imagelinkUpdateConsole()));

    /*Set up Wifi*/
    link.bind();
    link.addTopic(PayloadSensorIMUType);
    link.addTopic(PayloadCounterType);
    link.addTopic(PayloadElectricalType);
    link.addTopic(PayloadMissionType);
    connect(&link, SIGNAL(readReady()), this, SLOT(readoutConnection()));

    /*Set up bluetooth menu and LED*/
    imager.initializePort();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ui->bluetoothComboBox->addItem(info.portName());
    }
    connect(&imager, SIGNAL(updateStatus()), this, SLOT(updateBluetoothLED()));     /*Updating bluetooth LED*/
    connect(&imager, SIGNAL(updateImage()), this, SLOT(updateImage()));             /*Updating image in groundstation*/

    /*Set up timer for telemetry activity check*/
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(telemetryCheck()));
    timer->start();

    /*---------------*/
    /*CONNECT BUTTONS*/
    /*---------------*/

    /*Top Row menus*/
    connect(ui->openPortButton, SIGNAL(clicked()), this, SLOT(onOpenPortButtonClicked()));          /*Open selected bluetooth port*/
    connect(ui->closePortButton, SIGNAL(clicked()), this, SLOT(onClosePortButtonClicked()));        /*Close selected bluetooth port*/
    connect(ui->bluetoothComboBox, SIGNAL(activated(int)), this, SLOT(updateBluetoothLED()));       /*update bluetooth status LED when different port is selected*/
    connect(ui->restartWifiButton, SIGNAL(clicked()), this, SLOT(onRestartWifiButtonClicked()));    /*Wifi restart (for those x>1000 times I hang around in RZUWsec instead of yetenet*/
    connect(ui->emergencyOffButton, SIGNAL(clicked()), this, SLOT(onEmergencyOffButtonClicked()));  /*Turn off moving, power-consuming or dangerous devices instantly*/

    /*Manual Control Tab*/
    connect(ui->deployRacksButton, SIGNAL(clicked()), this, SLOT(onDeployRacksButtonClicked()));
    connect(ui->pullRacksButton, SIGNAL(clicked()), this, SLOT(onPullRacksButtonClicked()));
    connect(ui->stopRacksButton, SIGNAL(clicked()), this, SLOT(onStopRacksButtonClicked()));
    connect(ui->activateElectromagnetButton, SIGNAL(clicked()), this, SLOT(onActivateElectromagnetButtonClicked()));
    connect(ui->deactivateElectromagnetButton, SIGNAL(clicked()), this, SLOT(onDeactivateElectromagnetButtonClicked()));
    connect(ui->takePictureButton, SIGNAL(clicked()), this, SLOT(onTakePictureButtonClicked()));    /*Take picture and receive it via bluetooth*/

    /*Attitude Tab*/
    connect(ui->orientationSetButton, SIGNAL(clicked()), this, SLOT(onOrientationSetButtonClicked()));          /*Setting wished orientation angle...*/
    connect(ui->orientationLineEdit, SIGNAL(returnPressed()), this, SLOT(onOrientationSetButtonClicked()));     /*...also works with return in the lineEdit instead of clicking annoying button*/
    connect(ui->setRotationButton, SIGNAL(clicked()), this, SLOT(onSetRotationButtonClicked()));                /*Setting wished rotation speed...*/
    connect(ui->rotationLineEdit, SIGNAL(returnPressed()), this, SLOT(onSetRotationButtonClicked()));           /*...also works with return in the lineEdit instead of clicking annoying button*/

    /*Sun Finder Tab*/
    connect(ui->sunFinderButton, SIGNAL(clicked()), this, SLOT(onSunFinderButtonClicked()));                    /*Start sun acquisition*/

    /*Mission Tab*/
    connect(ui->missionStartButton, SIGNAL(clicked()), this, SLOT(onMissionStartButtonClicked()));              /*Start mission*/
    connect(ui->missionAbortButton, SIGNAL(clicked()), this, SLOT(onMissionAbortButtonClicked()));              /*Stop mission*/

    /*Set up graph widgets*/
    setupGraphs();

    /*Image Test*/
//    imager.readImage();
}

Groundstation::~Groundstation()
{
    delete ui;
}


/*-------------*/
/*Wifi readouts*/
/*-------------*/

void Groundstation::readoutConnection(){
    console("Package received");
    if(!ui->telemetryLED->isChecked()){
        console("Telemetry online.");
    }
    ui->telemetryLED->setChecked(true);
    PayloadSatellite payload = link.read();
    switch(payload.topic){
    case PayloadSensorIMUType:{
//        console("Package of type \"IMU\" received.");
        PayloadSensorIMU psimu(payload);
        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

        /*accelerometerWidget update*/
        ui->accelerometerWidget->graph(0)->addData(key, psimu.ax/1000);
        ui->accelerometerWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->accelerometerWidget->graph(0)->rescaleValueAxis();
        ui->accelerometerWidget->graph(1)->addData(key, psimu.ay/1000);
        ui->accelerometerWidget->graph(1)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->accelerometerWidget->graph(1)->rescaleValueAxis(true);
        ui->accelerometerWidget->graph(2)->addData(key, psimu.az/1000);
        ui->accelerometerWidget->graph(2)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->accelerometerWidget->graph(2)->rescaleValueAxis(true);
        ui->accelerometerWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->accelerometerWidget->replot();

        /*gyroscopeWidget update*/
        ui->gyroscopeWidget->graph(0)->addData(key, radToDeg(psimu.wx));
        ui->gyroscopeWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->gyroscopeWidget->graph(0)->rescaleValueAxis();
        ui->gyroscopeWidget->graph(1)->addData(key, radToDeg(psimu.wy));
        ui->gyroscopeWidget->graph(1)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->gyroscopeWidget->graph(1)->rescaleValueAxis(true);
        ui->gyroscopeWidget->graph(2)->addData(key, radToDeg(psimu.wz));
        ui->gyroscopeWidget->graph(2)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->gyroscopeWidget->graph(2)->rescaleValueAxis(true);
        ui->gyroscopeWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->gyroscopeWidget->replot();

        /*headingWidget update*/
        ui->headingWidget->graph(0)->addData(key, radToDeg(psimu.headingXm));
        ui->headingWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->headingWidget->graph(0)->rescaleValueAxis();
        ui->headingWidget->graph(1)->addData(key, radToDeg(psimu.headingGyro));
        ui->headingWidget->graph(1)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->headingWidget->graph(1)->rescaleValueAxis(true);
        ui->headingWidget->graph(2)->addData(key, radToDeg(psimu.headingFusion));
        ui->headingWidget->graph(2)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->headingWidget->graph(2)->rescaleValueAxis(true);
        ui->headingWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->headingWidget->replot();

        /*LCD updates*/
        ui->compassWidget->angle = radToDeg(psimu.headingFusion);
        ui->debrisMapWidget->angle = radToDeg(psimu.headingFusion);
        ui->rotationLCD->display(radToDeg(psimu.wz));
        ui->orientationLCD->display(radToDeg(psimu.headingFusion));
        ui->pitchLCD->display(radToDeg(psimu.pitch));
        ui->rollLCD->display(radToDeg(psimu.roll));

        /*LED updates*/
        if(psimu.calibrationActive){
            console("Calibration started. Telemetry switched off");
            ui->telemetryLED->setChecked(false);
        }

        break;
    }
    case PayloadCounterType:{
//        console("Package of type \"Counter\" received.");
        PayloadCounter pscount(payload);
        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        break;
    }
    case PayloadElectricalType:{
//        console("Package of type \"Electrical\" received.");
        PayloadElectrical pelec(payload);
        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

        /*LED updates*/
        ui->lightsensorLED->setChecked(pelec.lightsensorOn);
        ui->electromagnetLED->setChecked(pelec.electromagnetOn);
        ui->thermalKnifeLED->setChecked(pelec.thermalKnifeOn);
        ui->racksDeployedLED->setChecked(pelec.racksOut);
        ui->solarDeployedLED->setChecked(pelec.solarPanelsOut);

        /*LCD updates*/
        ui->lightsensorLCD->display((int) pelec.light);
        ui->solarVoltageLCD->display(pelec.solarPanelVoltage);
        ui->solarCurrentLCD->display(pelec.solarPanelCurrent);
        ui->batteryCurrentLCD->display(pelec.batteryCurrent);
        ui->batteryVoltageLCD->display(pelec.batteryVoltage);
        ui->powerConsumptionLCD->display(pelec.batteryVoltage * pelec.batteryCurrent / 1000);

        /*sunFinderWidget / lightsensor graph update*/
        ui->sunFinderWidget->graph(0)->addData(key, (int) pelec.light);
        ui->sunFinderWidget->graph(0)->removeDataBefore(key-XAXIS_VISIBLE_TIME);
        ui->sunFinderWidget->graph(0)->rescaleValueAxis();
        ui->sunFinderWidget->xAxis->setRange(key+0.25, XAXIS_VISIBLE_TIME, Qt::AlignRight);
        ui->sunFinderWidget->replot();
        break;
    }
    case PayloadMissionType:{
//        console("Package of type \"Mission\" received.");
        PayloadMission pmission(payload);
        key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        Debris tc_debris = Debris(pmission.partNumber, pmission.angle, pmission.isCleaned);
        ui->debrisMapWidget->processDebris(&tc_debris);
    }
    default:
        break;
    }
    return;
}


/*--------------------*/
/*BUTTONS/TELECOMMANDS*/
/*--------------------*/

/*Command-struct to be found in payload.h / .cpp*/

/*Telecommands*/
void Groundstation::telecommand(int ID, int identifier, int value){
    /*WIFI*/
    Command com(ID, identifier, value);
    link.connectionSendCommand(TELECOMMAND_TOPIC_ID, com);

    /*BLUETOOTH*/
//    imager.sendCommand(com);
}

/*Top Row*/
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
    telecommand(ID_ELECTRICAL, 3001, 0);    /*Stop racks*/
    telecommand(ID_ELECTRICAL, 3002, 0);    /*Turn off electromagnet*/
    telecommand(ID_ELECTRICAL, 3003, 0);    /*Turn off thermal knife*/
    telecommand(ID_ELECTRICAL, 3004, 0);    /*Stop main motor*/
}

/*Manual Control Tab*/
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
    console("Activating electromagnet.");
    telecommand(ID_ELECTRICAL, 3002, 1);
}

void Groundstation::onDeactivateElectromagnetButtonClicked(){
    console("Deactivating electromagnet.");
    telecommand(ID_ELECTRICAL, 3002, 0);
}

void Groundstation::onTakePictureButtonClicked(){
    console("Taking picture.");
    telecommand(ID_PICTURE, 4001, 1);
}

/*Attitude Tab*/
void Groundstation::onOrientationSetButtonClicked(){
    int angle;
    bool ok;
    angle = ui->orientationLineEdit->text().toInt(&ok);
    if(ok && (360 >= angle) && (angle >= 0)){
        console(QString("Setting orientation to %1 degrees.").arg(angle));
        telecommand(ID_ATTITUDE, 2002, angle);
    }
    else
        console("Orientation angle invalid.");
}

void Groundstation::onSetRotationButtonClicked(){
    int angle;
    bool ok;
    angle = ui->rotationLineEdit->text().toInt(&ok);
    if(ok && (360 >= angle) && (angle >= -360)){
        console(QString("Setting orientation to %1 degrees.").arg(angle));
        telecommand(ID_ATTITUDE, 2001, angle);
    }
    else
        console("Orientation angle invalid.");
}

/*Mission Tab*/
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

/*------------*/
/*GRAPH SETUPS*/
/*------------*/
void Groundstation::setupGraphs(){
    QFont legendFont = font();      /*Take MainWindow font..*/
    legendFont.setPointSize(7);     /*...and make a bit smaller for legend*/

    /*Set up accelerometerWidget*/
    ui->accelerometerWidget->xAxis->setLabel("Current Time");
    ui->accelerometerWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->accelerometerWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->accelerometerWidget->xAxis->setAutoTickStep(false);
    ui->accelerometerWidget->xAxis->setTickStep(10);
    ui->accelerometerWidget->yAxis->setLabel("Acceleration (g)");
    ui->accelerometerWidget->axisRect()->setupFullAxesBox();
    ui->accelerometerWidget->legend->setVisible(true);
    ui->accelerometerWidget->legend->setFont(legendFont);
    ui->accelerometerWidget->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->accelerometerWidget->legend->setIconSize(10,5);
    ui->accelerometerWidget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignLeft);

    ui->accelerometerWidget->addGraph();
    ui->accelerometerWidget->graph(0)->setPen(QPen(Qt::blue));
    ui->accelerometerWidget->graph(0)->setName("x");
    ui->accelerometerWidget->addGraph();
    ui->accelerometerWidget->graph(1)->setPen(QPen(Qt::red));
    ui->accelerometerWidget->graph(1)->setName("y");
    ui->accelerometerWidget->addGraph();
    ui->accelerometerWidget->graph(2)->setPen(QPen(Qt::green));
    ui->accelerometerWidget->graph(2)->setName("z");

    /*make left and bottom axes transfer their ranges to right and top axes*/
    connect(ui->accelerometerWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->accelerometerWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->accelerometerWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->accelerometerWidget->yAxis2, SLOT(setRange(QCPRange)));


    /*Set up gyroscopeWidget*/
    ui->gyroscopeWidget->xAxis->setLabel("Current Time");
    ui->gyroscopeWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->gyroscopeWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->gyroscopeWidget->xAxis->setAutoTickStep(false);
    ui->gyroscopeWidget->xAxis->setTickStep(10);
    ui->gyroscopeWidget->yAxis->setLabel("Angular Speed (deg/sec)");
    ui->gyroscopeWidget->axisRect()->setupFullAxesBox();
    ui->gyroscopeWidget->legend->setVisible(true);
    ui->gyroscopeWidget->legend->setFont(legendFont);
    ui->gyroscopeWidget->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->gyroscopeWidget->legend->setIconSize(10,5);
    ui->gyroscopeWidget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignLeft);

    ui->gyroscopeWidget->addGraph();
    ui->gyroscopeWidget->graph(0)->setPen(QPen(Qt::blue));
    ui->gyroscopeWidget->graph(0)->setName("x");
    ui->gyroscopeWidget->addGraph();
    ui->gyroscopeWidget->graph(1)->setPen(QPen(Qt::red));
    ui->gyroscopeWidget->graph(1)->setName("y");
    ui->gyroscopeWidget->addGraph();
    ui->gyroscopeWidget->graph(2)->setPen(QPen(Qt::green));
    ui->gyroscopeWidget->graph(2)->setName("z");

    /*make left and bottom axes transfer their ranges to right and top axes*/
    connect(ui->gyroscopeWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->gyroscopeWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->gyroscopeWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->gyroscopeWidget->yAxis2, SLOT(setRange(QCPRange)));


    /*Set up headingWidget*/
    ui->headingWidget->xAxis->setLabel("Current Time");
    ui->headingWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->headingWidget->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->headingWidget->xAxis->setAutoTickStep(false);
    ui->headingWidget->xAxis->setTickStep(10);
    ui->headingWidget->yAxis->setLabel("Heading (deg)");
    ui->headingWidget->axisRect()->setupFullAxesBox();
    ui->headingWidget->legend->setVisible(true);
    ui->headingWidget->legend->setFont(legendFont);
    ui->headingWidget->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->headingWidget->legend->setIconSize(10,5);
    ui->headingWidget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignLeft);

    ui->headingWidget->addGraph();
    ui->headingWidget->graph(0)->setPen(QPen(Qt::blue));
    ui->headingWidget->graph(0)->setName("Xm");
    ui->headingWidget->addGraph();
    ui->headingWidget->graph(1)->setPen(QPen(Qt::red));
    ui->headingWidget->graph(1)->setName("Gyro");
    ui->headingWidget->addGraph();
    ui->headingWidget->graph(2)->setPen(QPen(Qt::green));
    ui->headingWidget->graph(2)->setName("Combined");

    /*make left and bottom axes transfer their ranges to right and top axes*/
    connect(ui->headingWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->headingWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->headingWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->headingWidget->yAxis2, SLOT(setRange(QCPRange)));


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

    /*make left and bottom axes transfer their ranges to right and top axes*/
    connect(ui->sunFinderWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->sunFinderWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->yAxis2, SLOT(setRange(QCPRange)));

//    /*Maybe an angle dependent setup?*/
//    ui->sunFinderWidget->xAxis->setLabel("Angle");
//    ui->sunFinderWidget->xAxis->setAutoTickStep(false);
//    ui->sunFinderWidget->xAxis->setTickStep(90);
//    ui->sunFinderWidget->xAxis->setRange(0, 360);
//    ui->sunFinderWidget->yAxis->setLabel("Solar Voltage");
//    ui->sunFinderWidget->axisRect()->setupFullAxesBox();
//    ui->sunFinderWidget->addGraph();
//    ui->sunFinderWidget->graph(0)->setPen(QPen(Qt::blue));
//
//    /*make left and bottom axes transfer their ranges to right and top axes*/
//    connect(ui->sunFinderWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->sunFinderWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->sunFinderWidget->yAxis2, SLOT(setRange(QCPRange)));
}

/*--------------------*/
/*CONSOLE TEXT UPDATES*/
/*--------------------*/

void Groundstation::console(QString msg){
    ui->consoleWidget->writeString(msg);
}

void Groundstation::connectionUpdateConsole(){
    ui->consoleWidget->writeString(link.consoleText);
}

void Groundstation::imagelinkUpdateConsole(){
    ui->consoleWidget->writeString(imager.consoleText);
}

/*---------------------*/
/*DISPLAY UPDATED IMAGE*/
/*---------------------*/

void Groundstation::updateImage(){
    QImage scaled = imager.currentImage.scaled(ui->missionInputLabel->width(),ui->missionInputLabel->height(),Qt::KeepAspectRatio);
    ui->missionInputLabel->setPixmap(QPixmap::fromImage(scaled));
}

/*-----------*/
/*LED UPDATES*/
/*-----------*/

/*disable telemetry LED when connection is lost after around 3 seconds*/
void Groundstation::telemetryCheck(){
    if((ui->telemetryLED->isChecked()) && (QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0 - key) >= 3){
        ui->telemetryLED->setChecked(false);
        console("Telemetry lost.");
    }
}

/*update bluetooth activity LED when a different port is selected from list*/
void Groundstation::updateBluetoothLED(){
    imager.activePortName = ui->bluetoothComboBox->currentText();
    ui->bluetoothLED->setChecked(imager.isOpen());
}

/*Radiants to degrees conversion*/
float Groundstation::radToDeg(float rad){
    return (rad*180)/M_PI;
}
