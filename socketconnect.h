#ifndef SOCKETCONNECT_H
#define SOCKETCONNECT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QAbstractSocket>

class SocketConnect : public QObject
{
    Q_OBJECT
public:
    explicit SocketConnect(QObject *parent = nullptr);
    void connectToSocket(QString url = "");

signals:
    void connectedSuccessfully();
    void sendIncomingMessages(QByteArray message);

public slots:
    void onConnected();
    void closed();
    void timeout();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void write(QByteArray message);

private:
    QString m_url = "";
    QWebSocket m_socket;
    QTimer m_connectTimer;
};

#endif // SOCKETCONNECT_H
