/**The MIT License (MIT)
Copyright (c) 2018 by AleksanderSergeevich
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef CONNECTION_H
#define CONNECTION_H
#pragma once
#include <QtNetwork>

class IStreamParser {
public:
    virtual ~IStreamParser() {}

    virtual void reset() = 0;
    virtual void parse(QString& str) = 0;
    virtual void parse(QByteArray& buffer) = 0;
    virtual bool isEndMsg() = 0;
    virtual bool isValidMsg() = 0;
};

class TCPConnection : public QObject {
    Q_OBJECT
public:
    TCPConnection(IStreamParser* parser = NULL, QObject* parent = NULL) : QObject(parent) {
        socket_ptr.reset(new QTcpSocket(this));
        setup_socket();
        parser_ptr.reset(parser);
    }
    virtual ~TCPConnection() {
        close();
    }

    virtual bool close() {
        if(!socket_ptr.isNull() && socket_ptr->isOpen()) {
            socket_ptr->close();
        }
        socket_ptr->deleteLater();
    }
    virtual void setup_parser(IStreamParser* parser) {
        parser_ptr.reset(parser);
    }
    virtual bool setup_socket() {
        if(socket_ptr.isNull()) {
            qDebug() << "Impossible to initialize socket";
            return false;
        }
        connect(socket_ptr.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
        connect(socket_ptr.data(), SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(socket_ptr.data(), SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64 bytes)));
        connect(socket_ptr.data(), SIGNAL(connected()), this, SLOT(onConnected()));
        connect(socket_ptr.data(), SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        return true;
    }
signals:
    void msg_sent(void);
    void msg_received(void);
public slots:
    void connectToHost(const QString& host_address, const qint16 port) {
        socket_ptr->connectToHost(host_address, port);
    }
private slots:
    void onConnected() {
        time_conn_est = QDateTime::currentDateTime();
        qInfo() << "Connection was established at: " << time_conn_est.toString(QString("MMM d, yyyy @ h:m:s.zzz ap"));
    }
    void onDisconnected() {
        time_conn_est = QDateTime::currentDateTime();
        qInfo() << "Connection was closed at: " << time_conn_est.toString(QString("MMM d, yyyy @ h:m:s.zzz ap"));
        close();
    }
    void onSocketError(QAbstractSocket::SocketError) {
        if(socket_ptr->state() == QAbstractSocket::ConnectedState) {
            socket_ptr->close();
        }
        qDebug() << "Socket error occured: " << socket_ptr->errorString();
    }
    void onBytesWritten(qint64 bytes) {
        sent_size += bytes;
        if(sent_size == msg_size) emit msg_sent();
    }
    void onReadyRead() {
        quint64 bas = socket_ptr->bytesAvailable();
        if(bas == 0) return;
        QString lastLine;
        while(socket_ptr->canReadLine() && !parser_ptr->isEndMsg()) {
            lastLine = socket_ptr->readLine();
            parser_ptr->parse(lastLine);
        }
        if(!parser_ptr->isValidMsg()) {
            parser_ptr->reset();
            return;
        }
        if(parser_ptr->isEndMsg()) emit msg_received();
    }
private:
    QDateTime time_conn_est;
    size_t msg_size, sent_size;
    QString last_error;
    QSharedPointer<QTcpSocket> socket_ptr;
    QSharedPointer<IStreamParser> parser_ptr;
};

class SSLConnection : public QObject {
    Q_OBJECT
public:
    SSLConnection(IStreamParser* parser = NULL, QObject* parent = NULL) : QObject(parent) {
        socket_ptr.reset(new QSslSocket(this));
        setup_socket();
        parser_ptr.reset(parser);
    }
    virtual ~SSLConnection() {
        close();
    }

    virtual bool close() {
        if(!socket_ptr.isNull() && socket_ptr->isOpen()) {
            socket_ptr->close();
        }
        socket_ptr->deleteLater();
    }
    virtual void setup_parser(IStreamParser* parser) {
        parser_ptr.reset(parser);
    }
    virtual bool setup_socket() {
        if(socket_ptr.isNull()) {
            qDebug() << "Impossible to initialize socket";
            return false;
        }
        connect(socket_ptr.data(), SIGNAL(encrypted()), this, SLOT(onEncrypted()));
        connect(socket_ptr.data(), SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)));
        connect(socket_ptr.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
        connect(socket_ptr.data(), SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(socket_ptr.data(), SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten()));
        connect(socket_ptr.data(), SIGNAL(connected()), this, SLOT(onConnected()));
        connect(socket_ptr.data(), SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        return true;
    }
    virtual bool send_Msg(QByteArray& msg) {
        msg_size = msg.size();
        try {
            socket_ptr->write(msg);
        }
        catch(const std::exception& ex) {
            qDebug() << "Exception occured when message sending: " << ex.what();
        }
    }
signals:
    void msg_sent(void);
    void msg_received(void);
public slots:
    void secureConnect(const QString& host_address, const qint16 port) {
        socket_ptr->connectToHostEncrypted(host_address, port);
    }
private slots:
    void onConnected() {
        time_conn_est = QDateTime::currentDateTime();
        qInfo() << "Connection was established at: " << time_conn_est.toString(QString("MMM d, yyyy @ h:m:s.zzz ap"));
    }
    void onDisconnected() {
        time_conn_est = QDateTime::currentDateTime();
        qInfo() << "Connection was closed at: " << time_conn_est.toString(QString("MMM d, yyyy @ h:m:s.zzz ap"));
        close();
    }
    void onEncrypted() {
        const QSslCipher cipher = socket_ptr->sessionCipher();
        qInfo() << "Connection encrypted: " << QString("%1, %2").arg(cipher.authenticationMethod()).arg(cipher.name());
    }
    void onSslErrors(const QList<QSslError>& errors) {
        foreach (QSslError error, errors) {
            qDebug() << "SSL error occured: " << error.errorString();
        }
        socket_ptr->ignoreSslErrors();
        if(socket_ptr->state() != QAbstractSocket::ConnectedState) {
            qInfo() << "Connection was broken: " << socket_ptr->errorString();
        }
        else {
            socket_ptr->close();
        }
    }
    void onSocketError(QAbstractSocket::SocketError) {
        if(socket_ptr->state() == QAbstractSocket::ConnectedState) {
            socket_ptr->close();
        }
        qDebug() << "Socket error occured: " << socket_ptr->errorString();
    }
    void onBytesWritten(qint64 bytes) {
        sent_size += bytes;
        if(sent_size == msg_size) emit msg_sent();
    }
    void onReadyRead() {
        quint64 bas = socket_ptr->bytesAvailable();
        if(bas == 0) return;
        QString lastLine;
        while(socket_ptr->canReadLine() && !parser_ptr->isEndMsg()) {
            lastLine = socket_ptr->readLine();
            parser_ptr->parse(lastLine);
        }
        if(!parser_ptr->isValidMsg()) {
            parser_ptr->reset();
            return;
        }
        if(parser_ptr->isEndMsg()) emit msg_received();
    }
private:
    QDateTime time_conn_est;
    size_t msg_size, sent_size;
    QSharedPointer<IStreamParser> parser_ptr;
    QSharedPointer<QSslSocket> socket_ptr;
};

#endif // CONNECTION_H
