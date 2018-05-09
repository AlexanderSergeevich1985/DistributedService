#ifndef SERVER_H
#define SERVER_H
#pragma once
#include <QtNetwork>

#include "connection.h"

class TCPServer : public QTcpServer {
    Q_OBJECT
public:
    TCPServer(const quint16 port = 8080, QObject* parent = 0) : QTcpServer(parent) {
        server_start(port);
    }
    virtual ~TCPServer() {
        reset();
        if(this->isListening()) this->close();
    }

    virtual void reset() {
        if(!blocked_addresses.isEmpty()) blocked_addresses.clear();
        QHash<QString, QSharedPointer<TCPConnection> >::iterator iter = active_connections.begin();
        while(iter != active_connections.end()) {
            iter.value().reset();
        }
        active_connections.clear();
    }
    virtual void remove_conn(const QString conn_id) {
        active_connections[conn_id].reset();
        active_connections.remove(conn_id);
    }
    virtual void add_blocked_address(QHostAddress& addr) {
        blocked_addresses.append(addr);
    }
    virtual int remove_blocked_address(QHostAddress& addr) {
        return blocked_addresses.removeAll(addr);
    }
    QSharedPointer<TCPConnection> get_conn_ptr(const QString conn_id) const {
        if(!active_connections.contains(conn_id)) return QSharedPointer<TCPConnection>(NULL);
        return active_connections[conn_id];
    }
    void server_start(const quint16& port = 8080) {
        connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
        QDateTime start_time = QDateTime::currentDateTime();
        if(listen(QHostAddress::Any, port)) {
            qInfo()<< "Server was started at: " << start_time.toString(QString("MMM d, yyyy @ h:m:s.zzz ap"));
        }
        else {
            qDebug()<< "Server failed to start at: " << start_time.toString(QString("MMM d, yyyy @ h:m:s.zzz ap"));
        }
    }
public slots:
    void onNewConnection() {
        if(!this->hasPendingConnections()) return;
        QTcpSocket* new_connection = this->nextPendingConnection();
        if(!blocked_addresses.isEmpty() && blocked_addresses.contains(new_connection->peerAddress())) {
            qInfo() << "Refuse to establish connection with: " << new_connection->peerAddress().toString();
            new_connection->disconnectFromHost();
            return;
        }
        active_connections.insert(new_connection->peerAddress().toString(), QSharedPointer<TCPConnection>(new TCPConnection(new_connection, this)));
    }
private:
    QList<QHostAddress> blocked_addresses;
    QHash<QString, QSharedPointer<TCPConnection> > active_connections;
};

#endif // SERVER_H
