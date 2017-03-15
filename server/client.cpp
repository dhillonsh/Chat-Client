#include "client.hpp"

Client::Client(QObject *parent, int socketDescriptor)
        : m_socket(new QTcpSocket())
{
    m_socket->setSocketDescriptor(socketDescriptor);
    connect(m_socket, SIGNAL(readyRead()), parent, SLOT(onNewData()));
    connect(m_socket, SIGNAL(disconnected()), parent, SLOT(clientDisconnected()));
}
