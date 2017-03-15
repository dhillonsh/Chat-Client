#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <QTcpSocket>
#include <QDebug>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0, int socketDescriptor = -1);
    QTcpSocket *m_socket;
};

#endif // CLIENT_HPP
