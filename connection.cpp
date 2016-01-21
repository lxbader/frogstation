#include "connection.h"
#include <QNetworkInterface>
#include <QDateTime>
#include <QtEndian>

Connection::Connection(QObject *parent, bool checkChecksum)
    : QObject(parent), localAddress(LOCAL_IP), remoteAddress(SATELLITE_IP), port(PORT), udpSocket(this), bound(false), checkChecksum(checkChecksum), consoleText(""){
}

void Connection::bind(){
    console(QString("Binding ground station to IP %1 at port %2.").arg(LOCAL_IP).arg(PORT));
    if(udpSocket.bind(localAddress, port)){
        console("Binding successful.");
        bound = true;
    }
    else{
        console("ERROR: Binding not possible.");
        return;
    }
    connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(connectionReceive()));
}

void Connection::connectionReceive(){
    QByteArray buffer(1023, 0x00);
    udpSocket.readDatagram(buffer.data(), buffer.size());

    PayloadSatellite payload(buffer);

    quint16 checksum = 0;
    for(int i = 2; i < 26 + payload.userDataLen; ++i){
        bool lowestBit = checksum & 1;
        checksum >>= 1;
        if(lowestBit)
            checksum |= 0x8000;

        checksum += buffer[i];
    }

    if((!checkChecksum || checksum == payload.checksum) && topics.contains(payload.topic)){
        payloads.enqueue(payload);
        emit readReady();
    }
}

int Connection::connectionSendData(quint32 topicId, const QByteArray &data){
    QByteArray buffer(1023, 0x00);

    *((quint32*)(buffer.data() + 2)) = qToBigEndian((quint32)1);
    *((quint64*)(buffer.data() + 6)) = qToBigEndian((quint64)QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000000);
    *((quint32*)(buffer.data() + 14)) = qToBigEndian((quint32)1);
    *((quint32*)(buffer.data() + 18)) = qToBigEndian((quint32)topicId);
    *((quint16*)(buffer.data() + 22)) = qToBigEndian((quint16)10);
    *((quint16*)(buffer.data() + 24)) = qToBigEndian((quint16)data.length());    
    memcpy(buffer.data() + 26, data.constData(), data.length());
    *(buffer.data() + 26 + data.length()) = 0x00;

    quint32 checksum = 0;
    for(int i = 2; i < 26 + data.length(); ++i){
        if (checksum & 01)
            checksum = checksum >> 1 | 0x8000;
        else
            checksum >>= 1;

        checksum += (quint8)buffer[i];
        checksum &= 0xFFFF;
    }
    *((quint16*)(buffer.data() + 0)) = qToBigEndian((quint16)checksum);

    //console(QString("%1").arg(buffer));
    //console(buffer);

    return udpSocket.writeDatagram(buffer.constData(), buffer.size(), remoteAddress, port);
}

void Connection::connectionSendCommand(QString command){
    QByteArray data = command.toLocal8Bit();
    console(QString("Sending telecommand \"%1\".").arg(command));
    connectionSendData(TELECOMMAND_TOPIC_ID, data);
    return;
}

void Connection::addTopic(PayloadType topicId){
    topics.insert(topicId);
}

bool Connection::isBound(){
    return bound;
}

bool Connection::isReadReady(){
    return payloads.size();
}

PayloadSatellite Connection::read(){
    if(!payloads.size())
        return PayloadSatellite();
    return payloads.dequeue();
}

void Connection::console(QString msg){
    consoleText = msg;
    emit updateConsole();
}
