#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <QtGlobal>
#include <QByteArray>
#include <QtEndian>
#include "stdint.h"

enum PayloadType{
    PayloadCounterType = 5001,
    PayloadSensorIMUType = 5002,
    PayloadElectricalType = 5003,
//    PayloadImageType = 5004,
};

struct PayloadCounter;
struct PayloadSensorIMU;
struct PayloadElectrical;
//struct PayloadImage;

struct PayloadSatellite{
    quint16 checksum;
    quint32 senderNode;
    quint64 timestamp;
    quint32 senderThread;
    quint32 topic;
    quint16 ttl;
    quint16 userDataLen;
    quint8 userData[998];
    PayloadSatellite();
    PayloadSatellite(const QByteArray &buffer);
};

struct PayloadCounter{
    int counter;
    PayloadCounter(const PayloadSatellite payload);
};

struct PayloadSensorIMU{
    float ax;
    float ay;
    float az;
    float mx;
    float my;
    float mz;
    float wx;
    float wy;
    float wz; //rad/sec
    float roll;
    float pitch;
    float yaw;
    PayloadSensorIMU(const PayloadSatellite payload);
};

struct PayloadElectrical{
    bool lightsensorOn;
    bool electromagnetOn;
    bool thermalKnifeOn;
    int16_t light;
    PayloadElectrical(const PayloadSatellite payload);
};

//struct PayloadImage{
//    uint8_t image[121*160*2];
//    PayloadImage(const PayloadSatellite payload);
//};

struct Command{
    int id;
    QByteArray identifier;
    bool active;
    float value;
    Command(int tc_id, QByteArray tc_identifier, bool tc_active, float tc_value);
};

#endif // PAYLOAD_H
