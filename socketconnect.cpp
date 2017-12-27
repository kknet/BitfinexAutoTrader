#include "socketconnect.h"
#include <QDebug>

SocketConnect::SocketConnect(QObject *parent) : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &SocketConnect::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &SocketConnect::closed);
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
        [=](QAbstractSocket::SocketError error){ qDebug() << m_socket.errorString(); });
    connect(&m_connectTimer, &QTimer::timeout, this, &SocketConnect::timeout);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &SocketConnect::processTextMessage);
    connect(&m_socket, &QWebSocket::binaryMessageReceived, this, &SocketConnect::processBinaryMessage);
}

void SocketConnect::connectToSocket(QString url)
{
    if(url == "" && m_url == "")
    {
        qDebug() << "Missing url in socket";
        return;
    }

    m_socket.close();

    if(url != "")
    {
        m_url = url;
    }

    m_connectTimer.start(10000);
    m_socket.open(QUrl(m_url));
}

void SocketConnect::onConnected()
{
    m_connectTimer.stop();
    qDebug() << "connected successfully";
    emit connectedSuccessfully();
}

void SocketConnect::closed()
{
    qDebug() << "connection closed";
}

void SocketConnect::timeout()
{
    qDebug() << "attempting reconnect";
    connectToSocket();
}

void SocketConnect::write(QByteArray message)
{
    qDebug() << "Sending message: " << message;
    m_socket.sendTextMessage(message);
}

void SocketConnect::processTextMessage(QString message)
{
    qDebug() << "Message received:" << message;
    emit sendIncomingMessages(message.toUtf8());
}

void SocketConnect::processBinaryMessage(QByteArray message)
{
    qDebug() << "Binary Message received:" << message;
    emit sendIncomingMessages(message);
}
