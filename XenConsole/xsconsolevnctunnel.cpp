#include <QDebug>

#include "xsconsolevnctunnel.h"

XSConsoleVNCTunnel::XSConsoleVNCTunnel(QString url, QString username, QString password)
    : QThread(nullptr), vnc_server(nullptr), current_client(nullptr)
{
    QUrl qurl = url;
    //bool ssl = qurl.scheme() == "https";
    bool ssl = false;
    port = qurl.port(ssl ? 443 : 80);

    QString auth = QString::fromUtf8(QString("%1:%2").arg(username).arg(password).toUtf8().toBase64());
    query = QString("CONNECT %1?%2 HTTP/1.1\r\nAuthorization: Basic %3\r\nHost: %4\r\n\r\n").arg(QString::fromUtf8(qurl.encodedPath())).arg(QString::fromUtf8(qurl.encodedQuery())).arg(auth).arg(qurl.host());
    host = qurl.host();

    if(ssl)
        vnc_server = new QSslSocket;
    else
        vnc_server = new QTcpSocket;

    connect(vnc_server, SIGNAL(readyRead()), this, SLOT(writeToClient()));
}

XSConsoleVNCTunnel::~XSConsoleVNCTunnel()
{
    stopListening();

    delete vnc_server;
}

int XSConsoleVNCTunnel::startListening()
{
    qDebug() << "Starting tunnel..";

    connect(&server, SIGNAL(newConnection()), this, SLOT(connection()));

    if(!server.listen(QHostAddress::LocalHost))
        return -1;

    this->start();

    return server.serverPort();
}

void XSConsoleVNCTunnel::connection()
{
    if(vnc_server->isOpen())
        stopListening();

    got_rfb_header = false;

    vnc_server->connectToHost(host, port);

    if(!vnc_server->waitForConnected())
        return;

    vnc_server->write(query.toUtf8());
    vnc_server->waitForBytesWritten();

    current_client = server.nextPendingConnection();
}

void XSConsoleVNCTunnel::writeToClient()
{
    size_t sz = vnc_server->bytesAvailable();
    char buf[sz];
    char *b = buf;
    sz = vnc_server->read(buf, sz);
    if(!got_rfb_header && QString(buf).startsWith("RFB"))
    {
        got_rfb_header = true;

        connect(current_client, SIGNAL(readyRead()), this, SLOT(writeToServer()));
        connect(current_client, SIGNAL(aboutToClose()), this, SLOT(clientClosed()));

        writeToServer();
    }

    if(!got_rfb_header)
        return;

    while(sz)
    {
        size_t s = current_client->write(b, sz);
        sz -= s;
        b += s;
    }
}

void XSConsoleVNCTunnel::writeToServer()
{
    size_t sz = current_client->bytesAvailable();
    char buf[sz];
    char *b = buf;
    sz = current_client->read(buf, sz);
    while(sz)
    {
        size_t s = vnc_server->write(b, sz);
        sz -= s;
        b += s;
    }
}

bool XSConsoleVNCTunnel::isConnected()
{
    return current_client != nullptr;
}

void XSConsoleVNCTunnel::stopListening()
{
    qDebug() << "Stopping tunnel..";
    if(current_client)
        clientClosed();

    vnc_server->close();
}

void XSConsoleVNCTunnel::clientClosed()
{
    current_client->disconnect(this);
    current_client->deleteLater();
    current_client = nullptr;
}
