#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QTcpSocket>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>

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
    void addText(QString);
    QTcpSocket *socket;
    Ui::MainWindow *ui;
    QString username;
    QMap<QString, QString> readPacket(QByteArray);
    QByteArray createPacket(QString, QMap<QString, QString>);

private slots:
    void on_messageSend_clicked();
    void on_connectButton_clicked();
    void serverDisconnected();
    void on_loginButton_clicked();
    void on_logoutButton_clicked();
    void readyRead();
    void on_registerButton_clicked();
};

#endif // MAINWINDOW_HPP
