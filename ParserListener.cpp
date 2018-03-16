#include "json_stream/Listener/ParserListener.h"
#include "json_stream/JsonListener.h"

#include <QDebug>

void JSON_Reader_Listener::whitespace(char c) {
    qDebug() << "whitespace";
}

void JSON_Reader_Listener::startDocument() {
    qDebug() << "start document";
    reset_object();
}

void JSON_Reader_Listener::key(QString key) {
    qDebug() << "key: " << key;
    JSON_item* item = new JSON_item();
    item->setParentItem(obj_item);
    item->setType(QJsonValue::String);
    item->setKey(key);
    obj_item->appendChild(item);
}

void JSON_Reader_Listener::value(QString value) {
    qDebug() << "value: " << value;
    if(obj_item->child(obj_item->childCount()-1)->type() == QJsonValue::String)
        obj_item->child(obj_item->childCount()-1)->setValue(value);
    //else
      //  obj_item->set_array_value()->set(value);
}

void JSON_Reader_Listener::endArray() {
    qDebug() << "end array. ";
    obj_item = obj_item->parentItem();
}

void JSON_Reader_Listener::endObject() {
    qDebug() << "end object. ";
    obj_item = obj_item->parentItem();
}

void JSON_Reader_Listener::endDocument() {
    qDebug() << "end document. ";
    if(root_item != obj_item) root_item->set_error_code(QJsonParseError::GarbageAtEnd);
}

void JSON_Reader_Listener::startArray() {
    qDebug() << "start array. ";
    JSON_item* item = new JSON_item();
    item->setParentItem(obj_item);
    item->setType(QJsonValue::Array);
    obj_item->appendChild(item);
    obj_item = item;
}

void JSON_Reader_Listener::startObject() {
    qDebug() << "start object. ";
    JSON_item* item = new JSON_item();
    item->setParentItem(obj_item);
    item->setType(QJsonValue::Object);
    obj_item->appendChild(item);
    obj_item = item;
}

void JSON_Reader_Listener::reset_object() {
    if(root_item != NULL) {
        delete root_item;
        root_item = NULL;
    }
    root_item = new JSON_item();
    obj_item = root_item;
}

JSON_item* JSON_Reader_Listener::get_model() {
    return root_item;
}
