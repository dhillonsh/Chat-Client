#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QTcpSocket>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QLabel>
#include <QScrollBar>
#include <QFile>
#include <QDirIterator>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QString room;
    QString username;
    QTcpSocket *socket;
    void clearChatBox();
    void restoreDefaults();
    void addText(QString);
    QString parseMessage(QString);
    Ui::MainWindow *ui;
    QMap<QString, QString> readPacket(QByteArray);
    QByteArray createPacket(QString, QMap<QString, QString>);

private slots:
    void moveChatBoxScroll(int, int);
    void on_messageSend_clicked();
    void on_connectButton_clicked();
    void serverDisconnected();
    void on_loginButton_clicked();
    void on_logoutButton_clicked();
    void readyRead();
    void on_registerButton_clicked();
    void on_openNewRoomButton_clicked();
    void on_joinRoomButton_clicked();
};

#endif // MAINWINDOW_HPP
