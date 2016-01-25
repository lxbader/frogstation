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

#define XAXIS_VISIBLE_TIME 30

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

    double key;
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

    //Top Row
    void onOpenPortButtonClicked();
    void onClosePortButtonClicked();
    void onActivateTelemetryButtonClicked();
    void onDeactivateTelemetryButtonClicked();
    void onRestartWifiButtonClicked();
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
    void updateImage();
    void telemetryCheck();
    void updateBluetoothLED();
};

#endif // GROUNDSTATION_H
