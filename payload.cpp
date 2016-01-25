#include "payload.h"

PayloadSatellite::PayloadSatellite() : checksum(0), senderNode(0), timestamp(0), senderThread(0), topic(0), ttl(0), userDataLen(0){
    userData[0] = 0;
}

PayloadSatellite::PayloadSatellite(const QByteArray &buffer) : checksum(0), senderNode(0), timestamp(0), senderThread(0), topic(0), ttl(0), userDataLen(0){
    userData[0] = 0;
    if(buffer.size() < 1023)
        return;

    checksum = qFromBigEndian(*((quint16*)(buffer.constData() + 0)));
    senderNode = qFromBigEndian(*((quint32*)(buffer.constData() + 2)));
    timestamp = qFromBigEndian(*((quint64*)(buffer.constData() + 6)));
    senderThread = qFromBigEndian(*((quint32*)(buffer.constData() + 14)));
    topic = qFromBigEndian(*((quint32*)(buffer.constData() + 18)));
    ttl = qFromBigEndian(*((quint16*)(buffer.constData() + 22)));
    userDataLen = qFromBigEndian(*((quint16*)(buffer.constData() + 24)));
    memcpy(userData, buffer.constData() + 26, userDataLen);
    userData[userDataLen] = 0x00;
}

PayloadCounter::PayloadCounter(const PayloadSatellite payload): counter(0){
    if(payload.userDataLen != sizeof(PayloadCounter) || payload.topic != PayloadCounterType)
        return;
    counter = *(int*)(payload.userData);
}

PayloadSensorIMU::PayloadSensorIMU(const PayloadSatellite payload):ax(0), ay(0), az(0), mx(0), my(0), mz(0), wx(0), wy(0), wz(0), roll(0), pitch(0), yaw(0){
    if(payload.userDataLen != sizeof(PayloadSensorIMU) || payload.topic != PayloadSensorIMUType)
        return;
    ax = *(float*)(payload.userData);
    ay = *(float*)(payload.userData + 1 * sizeof(float));
    az = *(float*)(payload.userData + 2 * sizeof(float));
    mx = *(float*)(payload.userData + 3 * sizeof(float));
    my = *(float*)(payload.userData + 4 * sizeof(float));
    mz = *(float*)(payload.userData + 5 * sizeof(float));
    wx = *(float*)(payload.userData + 6 * sizeof(float));
    wy = *(float*)(payload.userData + 7 * sizeof(float));
    wz = *(float*)(payload.userData + 8 * sizeof(float));
    roll = *(float*)(payload.userData + 9 * sizeof(float));
    pitch = *(float*)(payload.userData + 10 * sizeof(float));
    yaw = *(float*)(payload.userData + 11 * sizeof(float));
}

PayloadElectrical::PayloadElectrical(const PayloadSatellite payload): lightsensorOn(0), electromagnetOn(0), thermalKnifeOn(0), light(0){
    if(payload.userDataLen != sizeof(PayloadElectrical) || payload.topic != PayloadElectricalType)
        return;
    lightsensorOn = *(bool*)(payload.userData);
    electromagnetOn = *(bool*)(payload.userData + 1 * sizeof(bool));
    thermalKnifeOn = *(bool*)(payload.userData + 2 * sizeof(bool));
    light = *(int16_t*)(payload.userData + 3 * sizeof(bool));
}

//PayloadImage::PayloadImage(const PayloadSatellite payload){
//    for(int i = 0; i<(121*160*2); i++){
//        image[i] = *(uint8_t*)(payload.userData + i * sizeof(uint8_t));
//    }
//}

Command::Command(int tc_id, QByteArray tc_identifier, bool tc_active, float tc_value): id(tc_id), identifier(tc_identifier), active(tc_active), value(tc_value){

}

