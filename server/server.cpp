#include "server.hpp"

//TODO:
//login verification
//register verification
//be able to choose any port you want when itiating
    // loop until a port can be opened

Server::Server(QObject *parent) :
    QTcpServer(parent)
    {
    }


/*
 * Start the server, verify that the given port can be listened on.
 * Check the if the database exists.
 */
void Server::startServer()
{
    if(!this->listen(QHostAddress::Any,11000)) qDebug() << "Could not start server" << this->serverError();
    else qDebug() << "Listening...";

    /*
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("chatClient");
        bool connected = db.open();
    if(!connected) qDebug() << "Failed to connect to database. " << db.lastError();
    else qDebug() << "Successfully connected to database";

qDebug() << "Tables: " << db.tables();
QSqlDatabase dbTest = QSqlDatabase::database();
QSqlQuery query(dbTest);


        query.exec("SELECT * FROM users");
        while(query.next()) {
                qDebug() << "In loop";
                qDebug() << query.value(0).toString();
                break;
        }
        qDebug() << "Out of check";
*/
}

/*
 * When a new client connects, accept and append to clientConnections list
 */
void Server::incomingConnection(int socketDescriptor)
{
    qDebug() << socketDescriptor << " Connecting...";

    socket = new Client(this, socketDescriptor);
    clientConnections.append(socket);
}

/* Slot, signaled from client.cpp
 * Reads data sent from a client
 */
void Server::onNewData() {
    Client* client = qobject_cast<Client*>(sender());
    QTcpSocket *socket = client->m_socket;
    QList<QByteArray> readData = socket->readAll().split('\n');

    qDebug() << "Read: " << readData;

    QList<QByteArray>::iterator iterator;
    for(iterator = readData.begin(); iterator != readData.end(); ++iterator) {
        if((*iterator).count() == 0) break;

        QXmlStreamReader xmlReader(*iterator);
        xmlReader.readNextStartElement();
        QStringRef packetType = xmlReader.name();

        QMap<QString, QString> map;
        if(packetType == "login") {
                xmlReader.readNextStartElement();
                QString username = xmlReader.readElementText();

                xmlReader.readNextStartElement();
                QString password = xmlReader.readElementText();

                qDebug() << username << password;
                if(userExists(username, password)) {
                    map = {{"status", "success"}, {"username", username}};
                    qDebug() << "Writing back: " << createPacket("login", map);
                    socket->write(createPacket("login", map));
                    sendUserJoined(socket, username);

                    client->username = username;
                    QList<Client*>::iterator i;
                    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
                        if((*i)->username.count() > 0 && (*i)->username != username) {
                            map = {{"type", "joined"}, {"username", (qobject_cast<Client*>(*i))->username}};
                            socket->write(createPacket("user", map));
                        }
                } else {
                    map = {{"status", "failure"}};
                    qDebug() << "Writing back: " << createPacket("login", map);
                    socket->write(createPacket("login", map));
                }

         } else if(packetType == "message") {
                qDebug() << "Received message packet";
                QList<Client*>::iterator i;
                for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
                    if((*i)->m_socket != socket) (qobject_cast<Client*>(*i))->m_socket->write(*iterator);
        }
    }
}

/*
 * Create a xml packet under the type "header" with elements defined by "map"
 */
QByteArray Server::createPacket(QString header, QMap<QString, QString> map) {
    QString packet;
    QXmlStreamWriter xmlWriter(&packet);

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement(header);

    QMapIterator<QString, QString> i(map);
    while (i.hasNext()) {
        i.next();
        xmlWriter.writeTextElement(i.key(), i.value());
    }

    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();

    return packet.toUtf8();
}

/*
 * Update all clients that a new user joined. Skip the user that joined
 */
void Server::sendUserJoined(QTcpSocket *socket, QString username) {
    QMap<QString, QString> map = {{"type", "joined"}, {"username", username}};

    QByteArray sendData = createPacket("user", map);
    QList<Client*>::iterator i;
    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
        if((*i)->m_socket != socket) (qobject_cast<Client*>(*i))->m_socket->write(sendData);
}

/*
 * Update all clients that a user has left.
 */
void Server::sendUserLeft(QString username) {
    if(username.count() == 0) return;
    qDebug() << username << "has left.";

    QMap<QString, QString> map = {{"type", "left"}, {"username", username}};

    QByteArray sendData = createPacket("user", map);

    QList<Client*>::iterator i;
    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
        (qobject_cast<Client*>(*i))->m_socket->write(sendData);
}

/*
 * Check if a user exists in the database
 */
bool Server::userExists(QString username, QString password) {
    return true;
}

/*
 * When a client disconnects, remove them from the clientConnections list and call sendUserLeft to all other clients
 */
void Server::clientDisconnected() {
    qDebug() << "Client has left!";
    Client* client = qobject_cast<Client*>(sender());

    int idx = clientConnections.indexOf(client);
    if (idx != -1) {
        sendUserLeft(clientConnections[idx]->username);
        clientConnections[idx]->m_socket->close();
        clientConnections.removeAt(idx);
    }
    client->deleteLater();
 }
