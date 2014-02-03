#include <QDebug>
#include <QSslConfiguration>

#include "xsconsolevnctunnel.h"

XSConsoleVNCTunnel::XSConsoleVNCTunnel(QString url, QString username, QString password)
    : QThread(nullptr), vnc_server(nullptr), current_client(nullptr)
{
    QUrl qurl = url;
    ssl = qurl.scheme() == "https";
    port = qurl.port(ssl ? 443 : 80);

    QString auth = QString::fromUtf8(QString("%1:%2").arg(username).arg(password).toUtf8().toBase64());
    query = QString("CONNECT %1?%2 HTTP/1.1\r\nAuthorization: Basic %3\r\nHost: %4\r\n\r\n").arg(QString::fromUtf8(qurl.encodedPath())).arg(QString::fromUtf8(qurl.encodedQuery())).arg(auth).arg(qurl.host());
    host = qurl.host();

    QSslConfiguration conf = vnc_ssl_server.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    vnc_ssl_server.setSslConfiguration(conf);

    if(ssl)
        vnc_server = &vnc_ssl_server;
    else
        vnc_server = &vnc_nossl_server;

    connect(vnc_server, SIGNAL(readyRead()), this, SLOT(writeToClient()));
}

XSConsoleVNCTunnel::~XSConsoleVNCTunnel()
{
    stopListening();
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

    QTcpSocket *socket = server.nextPendingConnection();

    got_rfb_header = false;
    if(ssl)
    {
        vnc_ssl_server.connectToHostEncrypted(host, port);
        if(!vnc_ssl_server.waitForEncrypted())
        {
            qDebug() << "ERROR: " << vnc_ssl_server.sslErrors()[0].errorString();
            socket->close();
            return;
        }
    }
    else
    {
        vnc_server->connectToHost(host, port);
        if(!vnc_server->waitForConnected())
        {
            socket->close();
            return;
        }
    }

    vnc_server->write(query.toUtf8());

    vnc_server->waitForBytesWritten();

    current_client = socket;

    writeToServer();
}

void XSConsoleVNCTunnel::writeToClient()
{
    while(!vnc_server->atEnd())
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
}

void XSConsoleVNCTunnel::writeToServer()
{
    while(!current_client->atEnd())
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
