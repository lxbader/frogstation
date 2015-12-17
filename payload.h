#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <QtGlobal>

enum PayloadType{
    PayloadCounterType = 5001,
    PayloadSensorIMUType = 5002,
    PayloadLightType = 5003, //sure?
};

struct PayloadSensorIMU;
struct PayloadCounter;
struct PayloadLight;

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
    PayloadSatellite(const PayloadSensorIMU payload);
    PayloadSatellite(const PayloadCounter payload);
};

struct PayloadSensorIMU{
    float wz; //rad/sec
    float roll;
    float pitch;
    float yaw;
    PayloadSensorIMU(const PayloadSatellite payload);
};

struct PayloadCounter{
    int counter;
    PayloadCounter(const PayloadSatellite payload);
};

struct PayloadLight{
    float light;
    PayloadLight(const PayloadSatellite payload);
};

#endif // PAYLOAD_H
