#ifndef GROUNDSTATION_H
#define GROUNDSTATION_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QtNetwork>
#include <QImage>

#include <stdio.h>
#include <math.h>

#include "connection.h"
#include "imagelink.h"

using namespace std;

namespace Ui {
    class Groundstation;
}

class Groundstation : public QMainWindow
{
    Q_OBJECT
    Connection link;
    Imagelink imager;


public:
    explicit Groundstation(QWidget *parent = 0);
    ~Groundstation();



private:
    Ui::Groundstation *ui;

    float currentax;
    float currentay;
    float currentaz;
    float currentmx;
    float currentmy;
    float currentmz;
    float currentwx;
    float currentwy;
    float currentwz;
    float currentRoll;
    float currentPitch;
    float currentYaw;
    int16_t currentLight;

    bool telemetryActive;
    bool thermalKnifeActive;
    bool electromagnetActive;
    bool lightsensorActive;

    void telecommand(int ID, QString msg, int value);
    void telecommand(int ID, QString msg, QString sign, int value);
    void setupGraphs();
    void console(QString msg);

    int displayImage(uint8_t orig[121*160*2], QLabel* label);

private slots:
    //Connection
    void readoutConnection();
    void connectionUpdateConsole();

    //Bluetooth
    void imagelinkUpdateConsole();
//    void onBluetoothConnectButtonClicked();

    //Top Row
    void onOpenPortButtonClicked();
    void onClosePortButtonClicked();
    void onActivateTelemetryButtonClicked();
    void onDeactivateTelemetryButtonClicked();
    void onEmergencyOffButtonClicked();

    //General Tab
    void onCalibrateMagnetometerButtonClicked();
    void onCalibrateAccelerometerButtonClicked();
    void onCalibrateGyroscopeButtonClicked();

    void onSetHBAButtonClicked();
    void onDeployMRackButtonClicked();
    void onPullMRackButtonClicked();
    void onStopMRackButtonClicked();
    void onDeployCRackButtonClicked();
    void onPullCRackButtonClicked();
    void onStopCRackButtonClicked();

    void onActivateThermalKnifeButtonClicked();
    void onDeactivateThermalKnifeButtonClicked();
    void onActivateElectromagnetButtonClicked();
    void onDeactivateElectromagnetButtonClicked();
    void onActivateLightsensorButtonClicked();
    void onDeactivateLightsensorButtonClicked();

    void onTakePictureButtonClicked();

    //Attitude Tab
    void onOrientationSetButtonClicked();
    void onOrientationResetButtonClicked();
    void onSetRotationButtonClicked();
    void onStopRotationButtonClicked();

    //Sun Finder Tab
    void onSunFinderButtonClicked();

    //Mission Tab
    void onMissionStartButtonClicked();
    void onMissionAbortButtonClicked();

    //Updates
    void fastUpdate();
    void slowUpdate();
    void updateImage();
};

#endif // GROUNDSTATION_H
