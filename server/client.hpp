#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <QTcpSocket>
#include <QDebug>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0, int socketDescriptor = -1);
    QString room;
    QTcpSocket *m_socket;
    QString username;
signals:
    void sendSocketClose();
    void sendReceivedData();
private slots:
    void socketClosed();
    void receivedData();
};

#endif // CLIENT_HPP
