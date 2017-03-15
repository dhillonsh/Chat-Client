#ifndef SERVER_HPP
#define SERVER_HPP

#include <QObject>
#include <QtcpServer>
#include <QtNetwork>
#include <QtDebug>
#include <QtSql>
#include <QXmlStreamWriter>

#include "client.hpp"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void startServer();
    QList<Client*> clientConnections;
    Client *socket;

private:
    bool userExists(QString, QString);
    void sendUserJoined(QTcpSocket*, QString);
    void sendUserLeft(QString);
    QByteArray createPacket(QString, QMap<QString, QString>);
signals:

public slots:
    void onNewData();
    void clientDisconnected();

protected:
    void incomingConnection(int socketDescriptor);

};

#endif // SERVER_HPP
