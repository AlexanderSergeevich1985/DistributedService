#ifndef REMOTESMARTPOINTER_H
#define REMOTESMARTPOINTER_H
#pragma once
#include <cstddef>
#include <iostream>
#include <csignal>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <QTcpServer>
#include <QMap>
#include <QCryptographicHash>
#include <QTime>
#include <QHostInfo>

#define DEFAULT_MESSAGE_SIZE 2048

struct Message;

template<typename T>
class RemoteDataWraper : public QObject {
public:
    enum State {
        blocked = 0,
        readable,
        writable
    };
    enum ControlMsgType {
        handShake = 0,
        change_state,
        control_message,
        data_message
    };
    RemoteDataWraper(QObject* parent = 0) : QObject(parent), connection_to_remote_server(new QTcpSocket(this)),
                                            data_ptr(0), data_size(0) {
        state = readable;
    }
    RemoteDataWraper(T* data_ptr_, size_t size = 0, QObject* parent = 0) : QObject(parent),
                                  connection_to_remote_server(new QTcpSocket(this)), data_ptr(data_ptr_) {
        state = readable;
        if(size == 0)
            data_size = (sizeof(data_ptr)/sizeof(*data_ptr));
        else
            data_size = size;
    }
    virtual ~RemoteDataWraper() {
        if(data_ptr != nullptr) {
            delete data_ptr;
            data_ptr = nullptr;
        }
        disconnected();
    }
    bool replace(const T& new_item, T& deleted_item, size_t index = 0) {
        if(state != writable || data_ptr == NULL || index < 0 || index >= data_size) return false;
        deleted_item = data_ptr[index];
        try {
            data_ptr[index] = new_item;
        }
        catch(const std::exception* ex) {
            return false;
        }
        return true;
    }
    bool replace_last(const T& new_item, T& deleted_item, size_t index = 0) {
        return insert(new_item, deleted_item, data_size - 1);
    }
    bool get_item(T& item, size_t index = 0) {
        if(state != readable || data_ptr == NULL || index < 0 || index >= data_size) return false;
        item = data_ptr[index];
        return true;
    }
    bool get_last_item(T& item, size_t index = 0) {
        return get_item(item, data_size - 1);
    }
    size_t size() {
        return data_size;
    }
    void connect_to_remote_server(const QString host, const int port) {
        connection_to_remote_server->abort();
        connection_to_remote_server->setReadBufferSize(DEFAULT_MESSAGE_SIZE);
        connect(connection_to_remote_server, SIGNAL(connected()), this, SLOT(connected()));
        connect(connection_to_remote_server, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connection_to_remote_server->connectToHost(host, port);
    }
    virtual QString generate_uid() {
        QString host_time = QTime::currentTime().toString("hh:mm:ss.zzz");
        QString host_id = QHostInfo::localDomainName();
        QByteArray uid();
        uid.fromStdString(host_time.append(host_id).toStdString());
        return QString(QCryptographicHash::hash(uid, QCryptographicHash::Md5).toHex());
    }
    bool register_new_object() {
        QString object_uid = generate_uid();
        QMap<QString, QString> msgs;
        msgs.insert(QString("UID"), object_uid);
        msgs.insert(QString("Action"), QString("Create"));
        QByteArray parcel;
        compose_msg(object_uid, parcel);
        connection_to_remote_server->write(parcel);
    }
    bool delete_object() {
        QMap<QString, QString> msgs;
        msgs.insert(QString("UID"), uid_object);
        msgs.insert(QString("Action"), QString("Delete"));
        QByteArray parcel;
        compose_msg(uid_object, parcel);
        connection_to_remote_server->write(parcel);
    }
    bool sync_object_state() {
        QMap<QString, QString> msgs;
        msgs.insert(QString("UID"), uid_object);
        msgs.insert(QString("Action"), QString("Synchronization"));
        QByteArray parcel;
        compose_msg(uid_object, parcel);
        connection_to_remote_server->write(parcel);
    }
public slots:
    void connected() {
        qDebug() << "Connection to server has been established";
        connect(connection_to_remote_server, SIGNAL(readyRead()), this, SLOT(make_handshake()));
    }
    void disconnected() {
        qDebug() << "Connection to server has been dropped";
        if(connection_to_remote_server != NULL && connection_to_remote_server->isOpen()) {
            connection_to_remote_server->flush();
            connection_to_remote_server->disconnectFromHost();
            connection_to_remote_server->close();
        }
    }
    void make_handshake() {
        if(parse_message_from_server(handShake)) {
            disconnect(connection_to_remote_server, SIGNAL(readyRead()), this, SLOT(make_handshake()));
            connect(connection_to_remote_server, SIGNAL(readyRead()), this, SLOT(ready_to_read()));
        }
    }
    void ready_to_read() {
        if(parse_message_from_server(control_message)) {
            choose_action();
        }
    }
protected:
    virtual void compose_msg(const QString& msg, QByteArray& parcel) = 0;
    virtual void compose_msg(const QMap<QString, QString>& msgs, QByteArray& parcel) = 0;
    virtual bool parse_message_from_server(const ControlMsgType msg_type) = 0;
    virtual bool unload_data_to_remote_node(const QString& node_id) = 0;
    virtual bool reload_data_from_remote_node(const QString& node_id) = 0;
    virtual bool request_state_change(State& state) = 0;
    virtual bool load_localnet_nodes_list(State& state) = 0;
    virtual bool choose_action() = 0;
private:
    T* data_ptr;
    size_t data_size;
    QString uid_object;
    State state;
    QList<Message> last_msg_list;

    QTcpSocket* connection_to_remote_server = nullptr;
    QMap<QString, QTcpSocket> localnet_nodes;
};


struct Rcount {
    Rcount() {
        count = 0;
    }
    Rcount(int count) {
        this->count = count;
    }
    void AddRef() {
        count++;
    }
    int Release() {
        return --count;
    }
    int count_hold() {
        return count;
    }
private:
    int count;
};

struct Locker {
    Locker() {
        writer_count = reader_count = 0;
        pthread_spin_init(&spinlock, 0);
    }
    ~Locker() {
        pthread_spin_destroy(&spinlock);
    }
    bool add_writer() {
        pthread_spin_lock(&spinlock);
        if(writer_count == 0 && reader_count == 0) {
            writer_count = 1;
            std::cout << "Lock writer counter: " << writer_count << std::endl;
            pthread_spin_unlock(&spinlock);
            return true;
        }
        return false;
    }
    unsigned int add_reader() {
        int reader_count_t = -1;
        pthread_spin_lock(&spinlock);
        if(writer_count == 0) {
            ++reader_count;
            reader_count_t = reader_count;
        }
        pthread_spin_unlock(&spinlock);
        return reader_count_t;
    }
    bool release_writer() {
        pthread_spin_lock(&spinlock);
        if(writer_count == 1) {
            writer_count = 0;
            std::cout << "Release writer counter: " << writer_count << std::endl;
            pthread_spin_unlock(&spinlock);
            return true;
        }
        return false;
    }
    int release_reader() {
        int reader_count_t = -1;
        pthread_spin_lock(&spinlock);
        if(reader_count > 0) {
            --reader_count;
            reader_count_t = reader_count;
        }
        pthread_spin_unlock(&spinlock);
        return reader_count_t;
    }
    bool is_writer() {
        int writer_count_t = 0;
        pthread_spin_lock(&spinlock);
        writer_count_t = writer_count;
        pthread_spin_unlock(&spinlock);
        return (writer_count_t == 1) ? true : false;
    }
    int count_read() {
        int reader_count_t = 0;
        pthread_spin_lock(&spinlock);
        reader_count_t = reader_count;
        pthread_spin_unlock(&spinlock);
        return reader_count_t;
    }
private:
    unsigned int writer_count;
    unsigned int reader_count;
    pthread_spinlock_t spinlock;
};

template<typename T>
class SharedPointer {
public:
    SharedPointer() : data_ptr(0), reference(0) {
        reference = new Rcount();
        reference->AddRef();
    }
    SharedPointer(T* new_data, size_t size = 0) : data_ptr(new_data), reference(0) {
        reference = new Rcount();
        reference->AddRef();
        if(size == 0)
            data_size = (sizeof(data_ptr)/sizeof(*data_ptr));
        else
            data_size = size;
    }
    SharedPointer(const SharedPointer<T>& other_ptr) : data_ptr(other_ptr.get_ptr()), reference(other_ptr.get_counter_ptr()) {
        if(reference != NULL) reference->AddRef();
        this->data_size = other_ptr.get_data_size();
    }
    SharedPointer(size_t data_size) : data_ptr(new T[data_size]()), reference(0) {
        this->data_size = data_size;
        reference = new Rcount();
        reference->AddRef();
    }
    virtual ~SharedPointer() {
        clear();
    }
    bool clear() {
        if(reference != NULL) {
            if(reference->Release() == 0) {
                if(data_ptr != NULL) delete data_ptr;
                data_ptr = NULL;
                if(reference != NULL) delete reference;
                reference = NULL;
            }
        }
    }
    SharedPointer<T>& operator = (const SharedPointer<T>& other_ptr) {
        if(this != &other_ptr) {
            if(reference->Release() == 0) {
                delete data_ptr;
                delete reference;
            }
            data_ptr = other_ptr.get_ptr();
            reference = other_ptr.get_counter_ptr();
            reference->AddRef();
            this->data_size = other_ptr.get_data_size();
        }
        return *this;
    }
    virtual bool reset(T* new_data_ptr = NULL) {
        if(reference != NULL) {
            if(reference->Release() == 0) {
                delete data_ptr;
                data_ptr = new_data_ptr;
                return true;
            }
        }
        return false;
    }
    virtual bool reset(size_t size) {
        if(size > 0 && reference != NULL) {
            if(reference->Release() == 0) {
                delete data_ptr;
                data_ptr = new T[size]();
                return true;
            }
        }
        return false;
    }
    bool id_valid() {
        return (data_ptr != NULL) ? true : false;
    }
    int count_hold() {
        return (reference != NULL) ? reference->count_hold() : 0;
    }
    T& operator* () {
        return *data_ptr;
    }
    T* operator-> () {
        return data_ptr;
    }
    T* get_ptr() {
        return data_ptr;
    }
    Rcount* get_counter_ptr() {
        return reference;
    }
    size_t get_data_size() {
        return data_size;
    }
private:
    T* data_ptr;
    Rcount* reference;
    size_t data_size;
};

template<typename T>
class LockedSharedPointer : public SharedPointer<T> {
    enum State {
        holder = 0,
        reader,
        writer
    };
public:
    LockedSharedPointer() : SharedPointer<T>() {
        state = holder;
        locker = new Locker();
    }
    LockedSharedPointer(T* new_data, size_t size = 0) : SharedPointer<T>(new_data, size) {
        state = holder;
        locker = new Locker();
    }
    LockedSharedPointer(const SharedPointer<T>& other_ptr) : SharedPointer<T>(other_ptr) {
        state = holder;
        locker = other_ptr.get_locker_ptr();
    }
    LockedSharedPointer(size_t data_size) : SharedPointer<T>(data_size) {
        state = holder;
        locker = new Locker();
    }
    ~LockedSharedPointer() {
        if(locker != NULL && locker->add_writer()) {
            this->clear();
            delete locker;
            locker = NULL;
        }
    }
    LockedSharedPointer<T>& operator = (const LockedSharedPointer<T>& other_ptr) {
        if(this == &other_ptr) return *this;
        while(check_and_release_self_state());
        SharedPointer<T>::operator = (other_ptr);
        locker = other_ptr.get_locker_ptr();
        return *this;
    }
    bool reset(T* new_data_ptr = NULL) {
        if(state != holder) {
            if(state == writer) {
                if(SharedPointer<T>::reset(new_data_ptr)) {
                    locker->release_writer();
                    return true;
                }
                return false;
            }
            else if(state == reader) {
                if(safe_release_reader() == -1) return NULL;
            }
        }
        if(locker->add_writer()) {
            if(SharedPointer<T>::reset(new_data_ptr)) {
                locker->release_writer();
                return true;
            }
        }
        locker->release_writer();
        return false;
    }
    bool reset(size_t size) {
        if(state != holder) {
            if(state == writer) {
                if(SharedPointer<T>::reset(size)) {
                    locker->release_writer();
                    return true;
                }
                return false;
            }
            else if(state == reader) {
                if(safe_release_reader() == -1) return NULL;
            }
        }
        if(locker->add_writer()) {
            if(SharedPointer<T>::reset(size)) {
                locker->release_writer();
                return true;
            }
        }
        locker->release_writer();
        return false;
    }
    Rcount* get_locker_ptr() {
        return locker;
    }
    T* safe_read_data_ptr() {
        if(state != holder) {
            if(state == reader) {
                return this->get_ptr();
            }
            else if(state == writer) {
                if(!safe_release_writer()) return NULL;
            }
        }
        int read_count = 0;
        int counter = 0;
        while(read_count == 0 && counter < 10) {
            ++counter;
            read_count = locker->add_reader();
            usleep(10*counter);
        }
        return read_count == 0 ? this->get_ptr() : NULL;
    }
    T* safe_write_data_ptr() {
        if(state != holder) {
            if(state == writer) {
                return this->get_ptr();
            }
            else if(state == reader) {
                if(safe_release_reader() == -1) return NULL;
            }
        }
        int counter = 0;
        bool flag = false;
        while(!(flag = locker->add_writer()) && counter < 10) {
            ++counter;
            usleep(10*counter);
        }
        if(flag) {
            state = writer;
            return this->get_ptr();
        }
        else
            return NULL;
    }
    int safe_release_reader() {
        int reader_count = -1;
        if(reader_count = locker->release_reader() != -1) {
            state = holder;
        }
        return reader_count;
    }
    bool safe_release_writer() {
        if(locker->release_writer()) {
            state = holder;
            return true;
        }
        else
            return false;
    }
    bool clear_state() {
        return check_and_release_self_state();
    }
protected:
    bool check_and_release_self_state() {
        if(state != holder) {
            if(state == reader) {
                return safe_release_reader() == -1 ? false : true;
            }
            else if(state == writer) {
                return safe_release_writer();
            }
        }
        return true;
    }
private:
    Locker* locker;
    State state;
};

#endif // REMOTESMARTPOINTER_H
