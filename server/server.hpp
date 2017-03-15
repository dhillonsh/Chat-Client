#ifndef SERVER_HPP
#define SERVER_HPP

#include <QObject>
#include <QTcpSocket>
#include <QtcpServer>
#include "client.hpp"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void startServer();
    QList<Client *> clientConnections;
    Client *socket;
signals:

public slots:
    void onNewData();
    void clientDisconnected();

protected:
    void incomingConnection(int socketDescriptor);

};

#endif // SERVER_HPP
