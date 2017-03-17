#ifndef SERVER_HPP
#define SERVER_HPP

#include <QObject>
#include <QTcpServer>
#include <QtNetwork>
#include <QtDebug>
#include <QtSql>
#include <QXmlStreamWriter>
#include <iostream>
#include "client.hpp"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void startServer();
    QList<Client*> clientConnections;
    QString room;
    Client *socket;

private:
    QMap<QString, QList<Client*>> roomList;
    QMap<QString, QString> userExists(QString, QString);
    QMap<QString, QString> registerNewUser(QString, QString);
    void sendUserJoined(Client*);
    void sendUserLeft(QString);
    QSqlDatabase db;
    QMap<QString, QString> readPacket(QByteArray);
    QByteArray createPacket(QString, QMap<QString, QString>);

public slots:
    void onNewData();
    void clientDisconnected();

protected:
    void incomingConnection(int socketDescriptor);
};

#endif // SERVER_HPP
