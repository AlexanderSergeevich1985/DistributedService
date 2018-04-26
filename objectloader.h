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
