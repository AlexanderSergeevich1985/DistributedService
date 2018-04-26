#ifndef OBJECTLOADER_H
#define OBJECTLOADER_H
#pragma once
#include <QDateTime>
#include <QSharedPointer>
#include <QUrl>

#include "objectmanager.h"

struct Object_Desc {
    virtual ~Object_Desc() {}

    QDateTime creation_time;
    QDateTime modification_time;
    QDateTime exportation_time;
    QString action_id;
    QString creator_id;
    QString object_id;
    size_t obj_size;
    QSharedPointer<Idata_wraper> data_ptr;
};

struct Sources_Desc {
    virtual ~Sources_Desc() {}

    QList<QUrl> sources;
};

class RawObject : public QObject {
    Q_OBJECT
public:
    RawObject(QObject* parent = NULL) : QObject(parent), is_Downloaded(false), bytes_loaded(0) {}
    virtual ~RawObject() {
        delete raw_data;
    }
    virtual void init_object(const size_t& size) {
        raw_data = new char[size];
        obj_desc.obj_size = size;
    }
    virtual bool isDownloaded() {
        return is_Downloaded;
    }
    char* get_raw_data() const {
        return raw_data;
    }
protected:
    virtual bool write_data(const size_t offset, const size_t size, const char* data) {
        if(offset + size >= obj_desc.obj_size) return false;
        memcpy(&raw_data[offset], data, size);
        bytes_loaded += size;
        if(bytes_loaded == obj_desc.obj_size) is_Downloaded = true;
        return true;
    }
private:
    char* raw_data;
    bool is_Downloaded;
    size_t bytes_loaded;
    Object_Desc obj_desc;
};

#endif // OBJECTLOADER_H
