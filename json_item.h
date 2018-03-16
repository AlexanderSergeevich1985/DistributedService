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

#ifndef JSON_ITEM_H
#define JSON_ITEM_H

#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QIcon>
#include <QIODevice>
#include <QLinkedList>

#include <QDebug>

#include <QLoggingCategory>

/*Часть реализации JSON parser мной написанная*/

/*
 * Контайнер для хранения пар ключ(строка)-параметр
 * Container for storing of key(string)-value pairs
*/
class KeyValueDataContainer {
public:
    template<typename T>
    void set(const QString& key, const T& val) {
        m_vals[key] = QVariant::fromValue(val);
    }
    template<typename T>
    T get(const QString& key, const T& defVal = T()) {
        if(m_vals.contains(key)) {
            return unpack<T>(m_vals[key], defVal);
        }
        return defVal;
    }
    template<typename T>
    T unpack(const QVariant& var, const T& defVal = T()) {
        if(var.isValid() && var.canConvert<T>()) {
            return var.value<T>();
        }
        return defVal;
    }
private:
    QHash<QString, QVariant> m_vals;
};

/*
 * Контайнер для хранения различных параметров
 * Container for different types values storing
*/
class DataContainer {
public:
    template<typename T>
    void set(const T& val) {
        m_vals.append(QVariant::fromValue(val));
    }
    template<typename T>
    T get(const int& index, const T& defVal = T()) {
        if(index > 0 && index < m_vals.size()) {
            return unpack<T>(m_vals.at(index), defVal);
        }
        return defVal;
    }
    template<typename T>
    T unpack(const QVariant& var, const T& defVal = T()) {
        if(var.isValid() && var.canConvert<T>()) {
            return var.value<T>();
        }
        return defVal;
    }
private:
    QLinkedList<QVariant> m_vals;
};

/*
 * Объект для хранения распарсенного json сообщения
 * Object for storing of parsed json message
*/
class JSON_item {
public:
    JSON_item(JSON_item* parent = 0);
    JSON_item(const QList<QVariant> &data, JSON_item* parent = 0);
    ~JSON_item();
    bool isParentItem(JSON_item* item);
    void setParentItem(JSON_item* item);
    void appendChild(JSON_item* item);
    void removeChild (int row);
    JSON_item* child(int row);
    bool isRoot() const;
    bool isValid() const;
    void set_error_code(QJsonParseError::ParseError err);
    QJsonParseError get_error_code() const;
    JSON_item* parentItem();
    bool hasChildren() const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    void setKey(const QString& key);
    void setValue(const QString& value);
    void setType(const QJsonValue::Type& type);
    DataContainer* set_array_value();
    QString key() const;
    QString value() const;
    QJsonValue::Type type() const;
    static JSON_item* load(const QJsonValue& value, JSON_item * parent = 0);
protected:
private:
    QString item_key;
    QString item_value;
    QJsonValue::Type item_type;
    QList<JSON_item*> m_childItems;
    QList<QVariant> m_itemData;
    DataContainer array_data;
    JSON_item* m_parentItem;
    QJsonParseError item_error;
};

/*
 * Объект для записи распарсенного json сообщения в устройство ввода/вывода
 * Object for writing of parsed json message to input/output device
*/
class JSON_printer {
public:
    JSON_printer(JSON_item* model = 0) : root_item(model), device(NULL) {
        level = 0;
    }
    void set_json_model(JSON_item* model) {
        root_item = model;
    }
    bool print_model() {
        if(root_item == NULL || !root_item->isValid()) return false;
        print_items(root_item);
        return true;
    }
    void print_items(JSON_item* obj_item_parent) {
        if(obj_item_parent == NULL || !obj_item_parent->isValid()) return;
        unsigned int counter = 0;
        unsigned int item_count = obj_item_parent->childCount();
        JSON_item* obj_item_current = NULL;
        while(counter < item_count) {
            obj_item_current = obj_item_parent->child(counter);
            ++counter;
            if(!obj_item_current->hasChildren()) {
                QString str(level, QChar(' '));
                str.append(QChar('"'));
                str.append(obj_item_current->key());
                str.append(QChar('\"'));
                str.append(QString(": "));
                str.append(obj_item_current->value());
                if(counter != item_count) str.append(',');
                qDebug() << str;
                str.append('\n');
                QByteArray msg = str.toUtf8();
                write_to_device(msg);
            }
            else {
                QString str(level, QChar(' '));
                if(obj_item_current->type() == QJsonValue::Object)
                    str.append('{');
                else if(obj_item_current->type() == QJsonValue::Array)
                    str.append('[');
                qDebug() << str;
                str.append('\n');
                QByteArray msg = str.toUtf8();
                write_to_device(msg);
                ++level;
                print_items(obj_item_current);
                --level;
                str.clear();
                str = QString(level, QChar(' '));
                if(obj_item_current->type() == QJsonValue::Object)
                    str.append('}');
                else if(obj_item_current->type() == QJsonValue::Array)
                    str.append(']');
                if(counter != item_count) str.append(',');
                qDebug() << str;
                str.append('\n');
                msg = str.toUtf8();
                write_to_device(msg);
            }
        }
    }
    void set_device(QIODevice* device) {
        this->device = device;
    }
    bool write_to_device(QByteArray& msg) {
        if(device == NULL) return false;
        try {
            if(!device->isOpen()) device->open(QIODevice::Append | QIODevice::Text);
            device->write(msg);
        }
        catch(std::exception& exception) {
            qCritical() << exception.what();
            return false;
        }
        return true;
    }
private:
    JSON_item* root_item;
    QIODevice* device;
    unsigned int level;
};

#endif // JSON_ITEM_H
