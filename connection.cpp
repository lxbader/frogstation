#include "connection.h"
#include <QNetworkInterface>
#include <QDateTime>
#include <QtEndian>

Connection::Connection(QObject *parent, bool checkChecksum)
    : QObject(parent), localAddress(LOCAL_IP), remoteAddress(SATELLITE_IP), port(PORT), udpSocket(this), bound(false), checkChecksum(checkChecksum){
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
//    console("Package received.");
    QByteArray buffer(1023, 0x00);
    udpSocket.readDatagram(buffer.data(), buffer.size());

    PayloadSatellite payload(buffer);

//    console("---------------------------------------");
//    console("Datagram header information:");
//    console("---------------------------------------");
//    console(QString("Checksum: %1").arg(payload.checksum));
//    console(QString("SenderNode: %1").arg(payload.senderNode));
//    console(QString("Timestamp: %1").arg(payload.timestamp));
//    console(QString("SenderThread: %1").arg(payload.senderThread));
//    console(QString("TopicID: %1").arg(payload.topic));
//    console(QString("TTL: %1").arg(payload.ttl));
//    console(QString("UserDataLen: %1").arg(payload.userDataLen));

    quint16 checksum = 0;
    for(int i = 2; i < 26 + payload.userDataLen; ++i){
        bool lowestBit = checksum & 1;
        checksum >>= 1;
        if(lowestBit)
            checksum |= 0x8000;

        checksum += buffer[i];
    }

    if((!checkChecksum || checksum == payload.checksum) && topics.contains(payload.topic)){
//        console("Enqueuing package");
        payloads.enqueue(payload);
        emit readReady();
    }
//    else
//        console("ERROR: Package damaged");
}

int Connection::connectionSendData(quint32 topicId, const QByteArray &data){
    QByteArray buffer(1023, 0x00);

    *((quint32*)(buffer.data() + 2)) = qToBigEndian(0);                                                             // TODO payload.senderNode;
    *((quint64*)(buffer.data() + 6)) = qToBigEndian(QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000000);    //sendedTime
    *((quint32*)(buffer.data() + 14)) = qToBigEndian(0);                                                            //senderThreadID
    *((quint32*)(buffer.data() + 18)) = qToBigEndian(topicId);                                                      //topicID
    *((quint16*)(buffer.data() + 22)) = qToBigEndian(64);                                                           //maxStepsToForward
    *((quint16*)(buffer.data() + 24)) = qToBigEndian(data.length());                                                //len
    memcpy(buffer.data() + 26, data.constData(), data.length());                                                    //userData
    *(buffer.data() + 26 + data.length()) = 0x00;

    quint16 checksum = 0;
    for(int i = 2; i < 26 + data.length(); ++i){
        bool lowestBit = checksum & 1;
        checksum >>= 1;
        if(lowestBit)
            checksum |= 0x8000;

        checksum += buffer[i];
    }
    *((quint16*)(buffer.data() + 0)) = checksum;                                                                    //checksum

    int i = udpSocket.writeDatagram(buffer.constData(), remoteAddress, port);
    console(QString("Number of sent bytes: %1").arg(i));
    QString transmit = QString::fromLocal8Bit(buffer);
    console(QString("Sent datagram: \"%1\"").arg(transmit));
    return i;
}

void Connection::connectionSendCommand(QString command){
    console(QString("Sending command \"%1\"").arg(command));
    int i = udpSocket.writeDatagram(command.toLocal8Bit(), remoteAddress, port);
    if(i == command.toLocal8Bit().size())
        console("Transmission successful.");
    else
        console("Transmission failed.");
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
