#include "client.hpp"

Client::Client(QObject *parent, int socketDescriptor)
        : m_socket(new QTcpSocket())
{
    m_socket->setSocketDescriptor(socketDescriptor);
    this->username = "";

    //Send a reference of Client instead of just the socket, so the username can be fetched in server.cpp
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(receivedData()));
    connect(this, SIGNAL(sendReceivedData()), parent, SLOT(onNewData()));

    //Send a reference of Client instead of just the socket, so the username can be fetched in server.cpp
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
    connect(this, SIGNAL(sendSocketClose()), parent, SLOT(clientDisconnected()));
}

void Client::socketClosed() {
    emit sendSocketClose();
}

void Client::receivedData() {
    emit sendReceivedData();
}
