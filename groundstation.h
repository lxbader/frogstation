#ifndef GROUNDSTATION_H
#define GROUNDSTATION_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QtNetwork>

#include <stdio.h>
#include <math.h>

#include "connection.h"

using namespace std;

namespace Ui {
    class Groundstation;
}

class Groundstation : public QMainWindow
{
    Q_OBJECT
    Connection link;

public:
    explicit Groundstation(QWidget *parent = 0);
    ~Groundstation();

    void console(QString msg);

private:
    Ui::Groundstation *ui;

    float currentRoll;
    float currentPitch;
    float currentYaw;
    float currentWz;
    float currentSolarVoltage;

    bool sunFinderActive;
    bool telemetryActive;

    void telecommand(int ID, QString msg, int value);
    void telecommand(int ID, QString msg, QString sign, int value);
    void setupGraphs();

private slots:
    void onRotationVelocityButtonClicked();
    void onMotorSpeedButtonClicked();

    void onOrientationSetButtonClicked();
    void onOrientationResetButtonClicked();

    void onSunFinderButtonClicked();

    void onMissionStartButtonClicked();
    void onMissionAbortButtonClicked();

    void onConnectionSendButtonClicked();
    void onActivateTelemetryButtonClicked();
    void onDeactivateTelemetryButtonClicked();

    void fastUpdate();
    void slowUpdate();

    void readoutConnection();
    void connectionUpdateConsole();
};

#endif // GROUNDSTATION_H
