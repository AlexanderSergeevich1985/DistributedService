#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H
#include <QSharedPointer>
#include <QStringList>
#include <QRegExp>
#include <QRegularExpressionMatch>
#include <QDataStream>
#include <QCryptographicHash>

#include <atomic>

struct Idata_wraper {
    enum States {
        blocked = 0,
        readable,
        writable
    };
    virtual ~Idata_wraper() {}
    virtual const std::type_info& type_info() const = 0;
    virtual const std::string type_identificator() const = 0;
    virtual QString get_object_hash() = 0;
    virtual void set_state(unsigned int state) = 0;
    virtual void set_state_with_check(unsigned int state) = 0;
    virtual const unsigned int get_state() const = 0;
    virtual bool is_acsessible() = 0;
    virtual bool is_readible() = 0;
    virtual bool is_writible() = 0;
    virtual void reset() = 0;
};

/*
 * Обертка для структур данных
 * Data structure wrapper, designed for type erosion of different data structures used for storing data
*/
template<class T>
struct Data_wraper_base : public Idata_wraper {
    Data_wraper_base() {
        this->state = States::blocked;
        this->type = QString("Undefined");
    }
    Data_wraper_base(T* object_ptr, const unsigned int state_ = States::blocked, const QString type = QString()) : data_ptr(object_ptr) {
        if(object_ptr == nullptr) {
            this->state = States::blocked;
            this->type = QString("Undefined");
        }
        else if(type.isNull() || type.isEmpty()) {
            this->type = QString(type_info().name());
        }
    }
    virtual ~Data_wraper_base() {}
    virtual const std::type_info& type_info() const {
        return typeid(data_ptr.data());
    }
    virtual const std::string type_identificator() const {
        return type.toStdString();
    }
    virtual void set_object(T* object_ptr, const QString type = QString()) {
        if(object_ptr == nullptr) {
            this->state = States::blocked;
            this->type = QString("Undefined");
            data_ptr.reset();
            return;
        }
        else if(type.isNull() || type.isEmpty()) {
            this->type = QString(type_info().name());
        }
        data_ptr.reset(object_ptr);
    }
    virtual T* get_object() const {
        if(data_ptr.isNull()) return NULL;
        return data_ptr.data();
    }
    virtual QString get_object_hash() {
        QByteArray object_data;
        object_data.clear();
        QDataStream stream(&object_data, QIODevice::WriteOnly);
        stream.writeRawData((char*)(data_ptr.data()), sizeof(*(data_ptr.data())));
        return QString(QCryptographicHash::hash(object_data, QCryptographicHash::Md5).toHex());
    }
    virtual void set_state(unsigned int state) {
        this->state = state;
    }
    virtual void set_state_with_check(unsigned int state) {
        if(!is_acsessible()) return;
        this->state = state;
    }
    virtual const unsigned int get_state() const {
        return this->state;
    }
    virtual bool is_acsessible() {
        return state == blocked ? false : true;
    }
    virtual bool is_readible() {
        return state == readable ? true : false;
    }
    virtual bool is_writible() {
        return state == writable ? true : false;
    }
    virtual void reset() {
        if(!data_ptr.isNull()) {
            data_ptr.reset();
            this->state = States::blocked;
            this->type = QString("Undefined");
        }
    }
private:
    QString type;
    std::atomic<unsigned int> state;
    QSharedPointer<T> data_ptr;
};

#endif // OBJECTMANAGER_H
