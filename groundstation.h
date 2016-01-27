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

#define ID_CALIBRATE 1
#define ID_ATTITUDE 2
#define ID_ELECTRICAL 3
#define ID_PICTURE 4
#define ID_MISSION 5

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
    void telecommand(int ID, int identifier, int value);
    void setupGraphs();
    void console(QString msg);

    int displayImage(uint8_t orig[121*160*2], QLabel* label);
    float radToDeg(float rad);

private slots:
    /*Connection*/
    void readoutConnection();
    void connectionUpdateConsole();

    /*Bluetooth*/
    void imagelinkUpdateConsole();

    /*Buttons Top Row*/
    void onOpenPortButtonClicked();
    void onClosePortButtonClicked();
    void onRestartWifiButtonClicked();
    void onEmergencyOffButtonClicked();

    /*Buttons Manual Control Tab*/
    void onDeployRacksButtonClicked();
    void onPullRacksButtonClicked();
    void onStopRacksButtonClicked();

    void onActivateElectromagnetButtonClicked();
    void onDeactivateElectromagnetButtonClicked();

    void onTakePictureButtonClicked();

    /*Buttons Attitude Tab*/
    void onOrientationSetButtonClicked();
    void onSetRotationButtonClicked();

    /*Buttons Mission Tab*/
    void onSunFinderButtonClicked();
    void onMissionStartButtonClicked();
    void onMissionAbortButtonClicked();

    /*Updates*/
    void updateImage();
    void telemetryCheck();
    void updateBluetoothLED();
};



#endif // GROUNDSTATION_H
