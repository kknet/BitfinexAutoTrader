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
    m_buttonMapper = new QSignalMapper(this);
    connect(m_buttonMapper,SIGNAL(mapped(int)),this,SLOT(startTrader(int)));
    m_walletWidget = new QWidget();
    QVBoxLayout *l = new QVBoxLayout();
    m_walletWidget->setLayout(l);
    ui->verticalLayout_main->addWidget(m_walletWidget);
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

    if(m_mode == 0)
    {
        if(inc.size() >= 3)
        {
            if(inc.at(1).toString() == "ws")
            {
                QJsonArray wallet = inc.at(2).toArray();
                for(int i = 0; i < wallet.size(); i++)
                {
                    QJsonArray coin = wallet.at(i).toArray();
                    if(coin.at(0).toString() == "exchange" && coin.at(1).toString() != "USD")
                    {
                        QHBoxLayout *l = new QHBoxLayout();
                        QWidget *w = new QWidget();
                        w->setLayout(l);
                        m_walletWidget->layout()->addWidget(w);

                        QLabel *coinLabel = new QLabel(coin.at(1).toString());
                        QLabel *amountLabel = new QLabel(QString::number(coin.at(2).toDouble()));

                        l->addWidget(coinLabel);
                        m_currencyList.insert(i, coinLabel->text());
                        l->addWidget(amountLabel);
                        l->addStretch(0);

                        QLineEdit *le = new QLineEdit();
                        le->setValidator(new QRegExpValidator(QRegExp("(0|([1-9][0-9]*))(\\.[0-9]+)?$"), this));
                        m_lineeditList.insert(i, le);
                        l->addWidget(le);

                        QPushButton *pb = new QPushButton("Start");
                        QObject::connect(pb, SIGNAL(clicked()),m_buttonMapper,SLOT(map()));
                        m_buttonMapper->setMapping(pb, i);
                        l->addWidget(pb);
                    }
                }
            }
        }
    }
    if(m_mode == 1)
    {
        if(inc.size() >= 2 && m_eventNumber >= 0)
        {
            if(inc.at(0).toInt() == m_eventNumber && inc.at(1).toString() == "te")
            {
                QJsonArray trade = inc.at(2).toArray();
                if(trade.size() == 4)
                {
                    double timestamp = trade.at(1).toDouble();
                    double price = trade.at(3).toDouble();

                    if(tradeLine < 0)
                        tradeLine = price;

                    estimatedValue = (simulatedCurrency * price) + simulatedCash;

                    traderList.append(QPair<double,double>(timestamp,price));


                }
            }
        }
        else
        {
            QJsonObject event = doc.object();
            if(event.contains("event") && event.contains("chanId") && event.contains("channel"))
            {
                if(event["event"].toString() == "subscribed" && event["channel"].toString() == "trades")
                {
                    m_eventNumber = event["chanId"].toInt();
                }
            }
        }
    }
}

void MainWindow::startTrader(int id)
{
    if(m_lineeditList.contains(id))
    {
        m_walletWidget->hide();
        m_walletWidget->deleteLater();
        m_mode = 1;
        m_maxTrade = m_lineeditList.value(id)->text().toDouble();

        QJsonObject obj;
        obj["event"] = "subscribe";
        obj["channel"] = "trades";
        obj["symbol"] = "t"+m_currencyList.value(id)+"USD";
        QJsonDocument doc(obj);

        socket->write(doc.toJson(QJsonDocument::Compact));
    }
}
