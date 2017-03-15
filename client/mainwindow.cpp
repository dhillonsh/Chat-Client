#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
        ui->setupUi(this);
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
    QString ip = ui->ipInput->toPlainText();
    QString port = ui->portInput->toPlainText();

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
    QMap<QString, QString> map = {{"username", ui->usernameLoginField->toPlainText()}, {"password", ui->passwordLoginField->toPlainText()}};
    socket->write(createPacket("login", map));
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
    ui->errorMessageInput->setPlainText("Logged out.");
    this->username = "";
    ui->userList->clear();
    ui->messageList->clear();
    this->socket->close();
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
}

/*
 * When data is ready to be read from the socket
 */
void MainWindow::readyRead()
{
    QList<QByteArray> readData = socket->readAll().split('\n');
    qDebug() << "Read: " << readData;

    QList<QByteArray>::iterator i;
    for (i = readData.begin(); i != readData.end(); ++i) {
        if((*i).count() == 0) break;

        QXmlStreamReader xmlReader(*i);
        xmlReader.readNextStartElement();

        QString packetType = xmlReader.name().toString();
        if(packetType == "login") {
            xmlReader.readNextStartElement();
            QString status = xmlReader.readElementText();
            if(status == "success") {
                xmlReader.readNextStartElement();
                this->username = xmlReader.readElementText();
                ui->pagesWidget->setCurrentIndex(2);
                ui->userList->append(this->username);
            } else ui->loginError->setPlainText("Unable to login, invalid credentials.");
        } else if(packetType == "user") {
            xmlReader.readNextStartElement();
            QString type = xmlReader.readElementText();
            xmlReader.readNextStartElement();
            QString user = xmlReader.readElementText();
            if(type == "joined") {
                QStringList lines = ui->userList->toPlainText().split("\n");
                bool userFound = false;
                int i = 0;
                while(i < lines.size()) {
                    QString line = lines.at(i);
                    if(line == user) {
                        userFound = true;
                        break;
                    } else i++;
                }
                if(!userFound) ui->userList->append(user);
            } else if(type == "left") {
                QStringList lines = ui->userList->toPlainText().split("\n");
                qDebug() << user << "has left!";
                int i = 0;
                while(i < lines.size()) {
                    QString line = lines.at(i);
                    if(line == user) {
                        lines.removeAt(i);
                        break;
                    } else i++;
                }
                ui->userList->setPlainText(lines.join("\n"));
            }
        } else if(packetType == "message") {
            xmlReader.readNextStartElement();
            QString message = xmlReader.readElementText();
            xmlReader.readNextStartElement();
            QString user = xmlReader.readElementText();
            addText(user + ": " + message);
        }
    }
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
