#include "json_item.h"

JSON_item::JSON_item(JSON_item* parent) {
    this->m_parentItem = parent;
    this->item_error.error = QJsonParseError::NoError;
}

JSON_item::JSON_item(const QList<QVariant>& data, JSON_item* parent) : m_parentItem(parent) {
    this->m_itemData = data;
    this->item_error.error = QJsonParseError::NoError;
}

JSON_item::~JSON_item() {
    qDeleteAll(m_childItems);
}

bool JSON_item::isValid() const {
    return item_error.error == QJsonParseError::NoError ? true : false;
}

void JSON_item::set_error_code(QJsonParseError::ParseError err) {
    this->item_error.error = err;
}

QJsonParseError JSON_item::get_error_code() const {
    return item_error;
}

void JSON_item::appendChild(JSON_item* item) {
    if(item->parentItem() == NULL) {
        item->setParentItem(this);
    }
    item->parentItem()->m_childItems.append(item);
}

bool JSON_item::isParentItem(JSON_item* item) {
    return item->parentItem() == this ? true : false;
}

void JSON_item::setParentItem(JSON_item* item) {
    this->m_parentItem = item;
}

void JSON_item::removeChild(int row) {
    JSON_item* item = this->child(row);
    m_childItems.removeAt(row);
    if(item) delete item;
}

JSON_item* JSON_item::child(int row) {
    return m_childItems.value(row);
}

bool JSON_item::isRoot() const {
    return this->m_parentItem == NULL ? true : false;
}

JSON_item* JSON_item::parentItem() {
    return this->m_parentItem;
}

QVariant JSON_item::data(int column) const {
    return m_itemData.value(column);
}

bool JSON_item::hasChildren() const {
    return !m_childItems.isEmpty();
}

int JSON_item::childCount() const {
    return m_childItems.count();
}

int JSON_item::row() const {
    if(m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<JSON_item*>(this));
    return 0;
}

void JSON_item::setKey(const QString& key) {
    this->item_key = key;
}

void JSON_item::setValue(const QString& value) {
    this->item_value = value;
}

void JSON_item::setType(const QJsonValue::Type& type) {
    this->item_type = type;
}

DataContainer* JSON_item::set_array_value() {
    return &array_data;
}

QString JSON_item::key() const {
    return item_key;
}

/*bool JSON_item::get_string(QString& str, JSON_item* read_ptr) {
    if(!str.isEmpty()) str.clear();
    return false;
}*/

QString JSON_item::value() const {
    return item_value;
}

QJsonValue::Type JSON_item::type() const {
    return item_type;
}

JSON_item* JSON_item::load(const QJsonValue& value, JSON_item* parent) {
    JSON_item* rootItem = new JSON_item(parent);
    rootItem->setKey("root");
    if(value.isObject()) {
        for(QString key : value.toObject().keys()) {
            QJsonValue v = value.toObject().value(key);
            JSON_item* child = load(v,rootItem);
            child->setKey(key);
            child->setType(v.type());
            rootItem->appendChild(child);
        }
    }
    else if(value.isArray()) {
        int index = 0;
        for(QJsonValue v : value.toArray()) {
            JSON_item* child = load(v,rootItem);
            child->setKey(QString::number(index));
            child->setType(v.type());
            rootItem->appendChild(child);
            ++index;
        }
    }
    else {
        rootItem->setValue(value.toVariant().toString());
        rootItem->setType(value.type());
    }
    return rootItem;
}
