#include "server.hpp"
#include <QtNetwork>
#include <QtDebug>
//#include <QtSql>

//TODO:
//when a client leaves, remove from array

Server::Server(QObject *parent) :
    QTcpServer(parent)
    {
    }


void Server::startServer()
{
    if(!this->listen(QHostAddress::Any,11000)) qDebug() << "Could not start server";
    else qDebug() << "Listening...";

    /*
    qDebug() << "test mysql";
    QStringList drivers = QSqlDatabase::drivers();
    qDebug()<<"list drivers";
    qDebug()<<drivers;

    QString driverName = "QSQLITE";
    qDebug()<<"TRY -- addDatabase"<<driverName;
    QSqlError err;
    QSqlDatabase db = QSqlDatabase::addDatabase(driverName, "Browser0");
    err = db.lastError();
    qDebug()<<"lastError for"<<driverName<<"is"<<err;

    driverName = "QMYSQL";
    qDebug()<<" ";qDebug()<<"TRY -- addDatabase"<<driverName;
    db = QSqlDatabase::addDatabase(driverName, "Browser1");
    err = db.lastError();
    qDebug()<<"lastError for"<<driverName<<"is"<<err;
*/
    /*
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "serverConnection");
    db.setHostName("localhost");
    db.setDatabaseName("chatClient");
    db.setUserName("root");
    db.setPassword("people1");
    if(!db.open()) qDebug() << "Failed to connect to database.";
    else qDebug() << "Successfully connected to database";
*/
}

void Server::incomingConnection(int socketDescriptor)
{
    qDebug() << socketDescriptor << " Connecting...";

    socket = new Client(this, socketDescriptor);
    clientConnections.append(socket);
}

void Server::onNewData() {
    qDebug() << "Received new data!";
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray readData = socket->readAll();

    QList<Client *>::iterator i;
    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
        if((*i)->m_socket != socket) (*i)->m_socket->write(readData);
}

void Server::clientDisconnected() {
    qDebug() << "Client has left!";
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

   QList<Client *>::iterator i;
    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
        if((*i)->m_socket == socket) clientConnections.removeAt(clientConnections.indexOf(*i));
    qDebug() << "Size of clientConnections: " << clientConnections.size();
 }
