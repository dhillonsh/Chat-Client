#include "server.hpp"

Server::Server(QObject *parent) :
    QTcpServer(parent)
    {
    }

/*
 * Start the server, and get prompted to input a port to listen on.
 * Check if the database exists and if not, create it along with the table `users`.
 */
void Server::startServer()
{
    int port = -1;
    char buffer[100];

    do {
        std::cout << "Enter a port to start the server on: ";
        if(!fgets(buffer, sizeof(buffer), stdin)) continue;
        port = atoi(buffer);
    } while(port <= 0 || !this->listen(QHostAddress::Any,port));
    qDebug() << "Listening on port" << port;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("chatClient.db");
    if(!db.open()) {
        qDebug() << "Failed to connect to database. " << db.lastError();
        exit(1);
    } else qDebug() << "Successfully connected to database";

    if(db.tables().indexOf("users") == -1) {
        QSqlQuery query;
        query.exec("CREATE TABLE users (username text, password text);");
    }
    if(db.tables().indexOf("users") == -1) {
        qDebug() << "There was an issue when creating the `users` table in the `chatClient.db` database.";
        exit(1);
    }

    this->roomList.insert("lobby", QList<Client*>());
}

/*
 * When a new client connects, accept and append to clientConnections list
 */
void Server::incomingConnection(qintptr socketDescriptor)
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

    QList<QByteArray>::iterator iterator;
    for(iterator = readData.begin(); iterator != readData.end(); ++iterator) {
        if((*iterator).count() == 0) break;

        QMap<QString, QString> packetMap = readPacket(*iterator);

        QMap<QString, QString> map;
        if(packetMap.value("header") == "login") {
                QMap<QString, QString> userExistsMap = userExists(packetMap.value("username"), packetMap.value("password"));
                socket->write(createPacket("login", userExistsMap));
                if(userExistsMap.value("status") == "success") {
                    client->username = packetMap.value("username");
                    client->room = "lobby";
                    QList<Client*> newList = this->roomList.value("lobby");
                    newList.append(client);
                    this->roomList["lobby"] = newList;

                    sendUserJoined(client);

                    QList<Client*>::iterator i;
                    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
                        if((*i)->username.count() > 0 && (*i)->username != client->username && (*i)->room == client->room) {
                            map = {{"type", "joined"}, {"username", (qobject_cast<Client*>(*i))->username}};
                            socket->write(createPacket("user", map));
                        }
                }
         } else if(packetMap.value("header") == "register") {
            QMap<QString, QString> registerMap = registerNewUser(packetMap.value("username"), packetMap.value("password"));
            socket->write(createPacket("register", registerMap));
            if(registerMap.value("status") == "success") {
                client->username = packetMap.value("username");
                client->room = "lobby";
                QList<Client*> newList = this->roomList.value("lobby");
                newList.append(client);
                this->roomList["lobby"] = newList;

                sendUserJoined(client);

                QList<Client*>::iterator i;
                for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
                    if((*i)->username.count() > 0 && (*i)->username != client->username && (*i)->room == client->room) {
                        map = {{"type", "joined"}, {"username", (qobject_cast<Client*>(*i))->username}};
                        socket->write(createPacket("user", map));
                    }
            }
        } else if(packetMap.value("header") == "message") {
                QList<Client*>::iterator i;
                for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
                    if((*i)->m_socket != socket && (*i)->room == client->room) (qobject_cast<Client*>(*i))->m_socket->write(*iterator);
        } else if(packetMap.value("header") == "room") {
            QString roomName = packetMap.value("name").toLower();
            client->room = roomName;
            QList<Client*> newList;
            if(this->roomList.contains(roomName)) QList<Client*> newList = this->roomList.value(roomName);

            newList.append(client);
            this->roomList[roomName] = newList;
            map = {{"name", packetMap.value("name")}};
            socket->write(createPacket("room", map));
            sendUserLeft(client->username);
            sendUserJoined(client);

            QList<Client*>::iterator i;
            for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
                if((*i)->username.count() > 0 && (*i)->username != client->username && (*i)->room == client->room) {
                    map = {{"type", "joined"}, {"username", (qobject_cast<Client*>(*i))->username}};
                    socket->write(createPacket("user", map));
                }
        }
    }
}

/*
 * Read a xml packet and create a QMap of its contents
 */
QMap<QString, QString> Server::readPacket(QByteArray packet) {
    QMap<QString, QString> map;
    QXmlStreamReader xmlReader(packet);

    xmlReader.readNextStartElement();
    map.insert("header", xmlReader.name().toString());

    while(xmlReader.readNextStartElement()) {
        map.insert(xmlReader.name().toString(), xmlReader.readElementText());
    }
    return map;
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
void Server::sendUserJoined(Client* client) {
    QMap<QString, QString> map = {{"type", "joined"}, {"username", client->username}};

    QByteArray sendData = createPacket("user", map);
    QList<Client*>::iterator i;
    for (i = clientConnections.begin(); i != clientConnections.end(); ++i)
        if((*i)->m_socket != client->m_socket && (*i)->room == client->room) {
            (qobject_cast<Client*>(*i))->m_socket->write(sendData);
        }
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
QMap<QString, QString> Server::userExists(QString username, QString password) {
    QMap<QString, QString> map;
    QSqlQuery query(db);

    query.prepare("SELECT username FROM users WHERE username=(:username) COLLATE NOCASE AND password=(:password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if(query.exec()) {
        if(query.next()) {
            map.insert("message", "");
            map.insert("status", "success");
            map.insert("username", query.value(0).toString());
        } else {
            map.insert("status", "failure");
            map.insert("message", "Invalid username/password combination.");
        }
    } else {
        qDebug() << "Error in `userExists`" << query.lastError();
        map.insert("status", "failure");
        map.insert("message", "Server Issue.");
    }
    return map;
}

/*
 * Attempt to register a user in the database
 */
QMap<QString, QString> Server::registerNewUser(QString username, QString password) {
    QMap<QString, QString> map;
    QMap<QString, QString> userExistsMap = userExists(username, password);
    QSqlQuery query(db);

    query.prepare("SELECT username FROM users WHERE username=(:username)");
    query.bindValue(":username", username);
    if(query.exec() && query.next()) {
        map.insert("status", "failure");
        map.insert("message", "Username already exists.");
    } else {
        query = QSqlQuery(db);
        query.prepare("INSERT INTO users (username, password) VALUES ((:username),(:password))");
        query.bindValue(":username", username);
        query.bindValue(":password", password);

        if(query.exec()) {
            map.insert("message", "");
            map.insert("status", "success");
            map.insert("username", username);
        } else {
            qDebug() << "Error in `registerNewUser`" << query.lastError();
            map.insert("status", "failure");
            map.insert("message", "Server Issue.");
        }
    }
    return map;
}

/*
 * When a client disconnects, remove them from the clientConnections list and call sendUserLeft to all other clients
 */
void Server::clientDisconnected() {
    Client* client = qobject_cast<Client*>(sender());

    int idx = clientConnections.indexOf(client);
    if (idx != -1) {
        sendUserLeft(clientConnections[idx]->username);
        if(this->roomList.contains(client->room)) {
            QList<Client*> newList = this->roomList.value(client->room);
            newList.removeAt(newList.indexOf(client));
            if(newList.count() == 0) this->roomList.remove(client->room);
            else this->roomList[client->room] = newList;
        }
        clientConnections[idx]->m_socket->close();
        clientConnections.removeAt(idx);
    }
    client->deleteLater();
 }
