#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageAuthenticationCode>
#include <QDateEdit>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QHBoxLayout>

#define BITURL "wss://api.bitfinex.com/ws/2"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new SocketConnect(this);
    connect(socket, &SocketConnect::connectedSuccessfully, this, &MainWindow::onConnect);
    connect(socket, &SocketConnect::sendIncomingMessages, this, &MainWindow::parseMessages);
    socket->connectToSocket(BITURL);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnect()
{
    QFile file("d:/Temp/ApiKey.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    int counter = 0;
    QByteArray key;
    QByteArray seckey;
    while (!file.atEnd()) {
        if(counter == 0)
            key = file.readLine().trimmed();
        else
            seckey = file.readLine().trimmed();
        counter++;
    }

    QByteArray authNonce = QString::number(QDateTime::currentMSecsSinceEpoch()).toUtf8();
    QByteArray authPayload = "AUTH" + authNonce;
    QMessageAuthenticationCode code(QCryptographicHash::Sha384);

    code.setKey(seckey);
    code.addData(authPayload);

    QJsonObject obj;
    obj["event"] = "auth";
    obj["apiKey"] = QString(key);
    obj["authSig"] = QString(code.result().toHex());
    obj["authPayload"] = QString(authPayload);
    obj["authNonce"] = QString(authNonce);
    obj["calc"] = 1;
    QJsonDocument doc(obj);

    socket->write(doc.toJson(QJsonDocument::Compact));
}

void MainWindow::parseMessages(QByteArray message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonArray inc = doc.array();

    if(inc.size() >= 3)
    {
        if(inc.at(1).toString() == "ws")
        {
            QJsonArray wallet = inc.at(2).toArray();
            for(int i = 0; i < wallet.size(); i++)
            {
                QJsonArray coin = wallet.at(i).toArray();
                QHBoxLayout *l = new QHBoxLayout();

                QLabel *typeLabel = new QLabel(coin.at(0).toString());
                QLabel *coinLabel = new QLabel(coin.at(1).toString());
                QLabel *amountLabel = new QLabel(QString::number(coin.at(2).toDouble()));

                l->addWidget(typeLabel);
                l->addWidget(coinLabel);
                l->addWidget(amountLabel);

                ui->verticalLayout_main->addLayout(l);
            }
        }
    }
}
