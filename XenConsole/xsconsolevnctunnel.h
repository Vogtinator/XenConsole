#ifndef XSCONSOLEVNCTUNNEL_H
#define XSCONSOLEVNCTUNNEL_H

#include <QThread>
#include <QUrl>
#include <QTcpServer>
#include <QSslSocket>

class XSConsoleVNCTunnel : public QThread
{
    Q_OBJECT
public:
    XSConsoleVNCTunnel(QString url, QString username, QString password);

    int startListening();
    void stopListening();

    bool isConnected();

    ~XSConsoleVNCTunnel();

public slots:
    void connection();
    void writeToServer();
    void writeToClient();
    void clientClosed();
    
private:
    QTcpSocket *vnc_server;
    QTcpSocket *current_client;
    QTcpServer server;
    QString host, query;
    int port;
    bool got_rfb_header;
};

#endif // XSCONSOLEVNCTUNNEL_H
