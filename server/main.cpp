#include <QCoreApplication>
#include "server.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server chatServer;
    chatServer.startServer();

    return a.exec();
}
