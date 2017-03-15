#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
        ui->setupUi(this);

        ui->ipInput->setValidator(new QRegExpValidator( QRegExp("[A-Za-z0-9.]*"), this ));
        ui->portInput->setValidator(new QRegExpValidator( QRegExp("[0-9.]*"), this ));

        ui->usernameLoginField->setValidator(new QRegExpValidator( QRegExp("[A-Za-z0-9_]*"), this ));
        ui->passwordLoginField->setValidator(new QRegExpValidator( QRegExp("[A-Za-z0-9]*"), this ));

        ui->usernameRegisterField->setValidator(new QRegExpValidator( QRegExp("[A-Za-z0-9_]*"), this ));
        ui->passwordRegisterField->setValidator(new QRegExpValidator( QRegExp("[A-Za-z0-9]*"), this ));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addText(QString str) {
    ui->messageList->append(str);
}

/*
 * When the "connect" button is clicked, attempt to connect to the given ip:port.
 * Page: 0
 */
void MainWindow::on_connectButton_clicked()
{
    QString ip = ui->ipInput->text();
    QString port = ui->portInput->text();

    socket = new QTcpSocket(this);
    socket->connectToHost(ip, quint16(port.toInt()));

    if (socket->waitForConnected( 5 )) {
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()),Qt::DirectConnection);
        ui->errorMessageInput->setPlainText("Connected to server.");
        ui->pagesWidget->setCurrentIndex(1);
        ui->ipInput->clear();
        ui->portInput->clear();
    } else ui->errorMessageInput->setPlainText("Unable to connect.");
    connect(socket, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
}

/*
 * When the "login" button is clicked, send the given username:password to the server for verification.
 * Page: 1
 */
void MainWindow::on_loginButton_clicked()
{
    QMap<QString, QString> map = {{"username", ui->usernameLoginField->text()}, {"password", ui->passwordLoginField->text()}};
    socket->write(createPacket("login", map));
}

/*
 * When the "register" button is clicked, send the given username:password to the server for verification.
 * Page: 1
 */
void MainWindow::on_registerButton_clicked()
{
    QMap<QString, QString> map = {{"username", ui->usernameRegisterField->text()}, {"password", ui->passwordRegisterField->text()}};
    socket->write(createPacket("register", map));
}

/*
 * When the "send" button is clicked, add the text to the chat box and send it to the server.
 * Page: 2
 */
void MainWindow::on_messageSend_clicked()
{
    QString data = ui->messageInput->toPlainText();

    QTextDocument filter;
    filter.setHtml(data);
    data = filter.toPlainText();

    addText(username + ": " + data);
    ui->messageInput->clear();

    QMap<QString, QString> map = {{"message", data}, {"username", this->username}};
    qDebug() << "Sending message packet: " << createPacket("message", map);
    socket->write(createPacket("message", map));
}

/*
 * When the "logout" button is clicked, return to Page 0.
 * Page: 2
 */
void MainWindow::on_logoutButton_clicked()
{
    this->socket->close();
    this->username = "";
    ui->userList->clear();
    ui->messageList->clear();
    ui->errorMessageInput->setPlainText("Logged out.");
    ui->pagesWidget->setCurrentIndex(0);
}

/*
 * If the client disconnects from the server (e.x. server is stopped), return to Page 0.
 */
void MainWindow::serverDisconnected() {
    ui->pagesWidget->setCurrentIndex(0);
    ui->errorMessageInput->setPlainText("Server disconnected.");
    this->username = "";
    ui->userList->clear();
    ui->messageList->clear();
    ui->loginError->clear();
    ui->usernameRegisterField->clear();
    ui->passwordRegisterField->clear();
}

/*
 * When data is ready to be read from the socket
 */
void MainWindow::readyRead()
{
    QList<QByteArray> readData = socket->readAll().split('\n');
    qDebug() << "Read: " << readData;

    QList<QByteArray>::iterator iterator;
    for (iterator = readData.begin(); iterator != readData.end(); ++iterator) {
        if((*iterator).count() == 0) break;

        QMap<QString, QString> packetMap = readPacket(*iterator);

        if(packetMap.value("header") == "login") {
            if(packetMap.value("status") == "success") {
                this->username = packetMap.value("username");
                ui->pagesWidget->setCurrentIndex(2);
                ui->userList->append(this->username);
            } else ui->loginError->setPlainText(packetMap.value("message"));
        } else if(packetMap.value("header") == "register") {
            if(packetMap.value("status") == "success") {
                this->username = packetMap.value("username");
                ui->pagesWidget->setCurrentIndex(2);
                ui->userList->append(this->username);
            } else ui->registerError->setPlainText(packetMap.value("message"));
        } else if(packetMap.value("header") == "user") {
            if(packetMap.value("type") == "joined") {
                QStringList lines = ui->userList->toPlainText().split("\n");
                bool userFound = false;
                int i = 0;
                while(i < lines.size()) {
                    QString line = lines.at(i);
                    if(line == packetMap.value("username")) {
                        userFound = true;
                        break;
                    } else i++;
                }
                if(!userFound) ui->userList->append(packetMap.value("username"));
            } else if(packetMap.value("type") == "left") {
                QStringList lines = ui->userList->toPlainText().split("\n");
                int i = 0;
                while(i < lines.size()) {
                    QString line = lines.at(i);
                    if(line == packetMap.value("username")) {
                        lines.removeAt(i);
                        break;
                    } else i++;
                }
                ui->userList->setPlainText(lines.join("\n"));
            }
        } else if(packetMap.value("header") == "message") {
            addText(packetMap.value("username") + ": " + packetMap.value("message"));
        }
    }
}

/*
 * Read a xml packet and create a QMap of its contents
 */
QMap<QString, QString> MainWindow::readPacket(QByteArray packet) {
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
QByteArray MainWindow::createPacket(QString header, QMap<QString, QString> map) {
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
