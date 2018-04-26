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
    virtual bool is_acsessible() const = 0;
    virtual bool is_readible() const = 0;
    virtual bool is_writible() const = 0;
    virtual void reset() = 0;
    virtual void* Value() = 0;
    template<class T>
    bool isConvertable() {
        return typeid(T) == type_info();
    }
    template<class T>
    T* value() {
        return static_cast<T*>(Value());
    }
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
        return typeid(*data_ptr.data());
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
    virtual bool is_acsessible() const {
        return state == blocked ? false : true;
    }
    virtual bool is_readible() const {
        return state == readable ? true : false;
    }
    virtual bool is_writible() const {
        return state == writable ? true : false;
    }
    virtual void reset() {
        if(!data_ptr.isNull()) {
            data_ptr.reset();
            this->state = States::blocked;
            this->type = QString("Undefined");
        }
    }
    virtual void* Value() {
        return static_cast<void*>(data_ptr.data());
    }
private:
    QString type;
    std::atomic<unsigned int> state;
    QSharedPointer<T> data_ptr;
};

/*Container for storing set of different objects used in work operation*/
class BaseDataContainer {
public:
    void set(const QString& key, Idata_wraper* val) {
        QSharedPointer<Idata_wraper> val_ptr(val);
        map_vals.insert(key, val_ptr);
    }
    template<typename T>
    T* get(const QString& key, T* defVal = new T()) {
        if(map_vals.contains(key)) {
            return unpack<T>(map_vals[key], defVal);
        }
        return defVal;
    }
    template<typename T>
    T* unpack(const QSharedPointer<Idata_wraper>& wrap_ptr, T* defVal = new T()) {
        if(!wrap_ptr.isNull() && wrap_ptr->isConvertable<T>()) {
            return wrap_ptr->value<T>();
        }
        return defVal;
    }
private:
    QMap<QString, QSharedPointer<Idata_wraper> > map_vals;
};

#endif // OBJECTMANAGER_H
