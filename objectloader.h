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

struct FileMapper {
    enum SizeTypes {
        small = 0,
        middle,
        large,
        very_large
    };
    struct ChunkState {
        bool isDownloaded() const {
            return bytes_left == 0 ? true : false;
        }
        bool change_state(const size_t& bytes_written) {
            if(bytes_written > bytes_left) return false;
            bytes_left -= bytes_written;
        }
        size_t offset;
        size_t bytes_left;
    };
    virtual ~FileMapper() {}

    void init_mapper(size_t& size) {
        if(size == 0) return;
        chunk_count = (size == CHUNK_SIZE ? size/CHUNK_SIZE : size/CHUNK_SIZE + 1);
        ending_size = size % CHUNK_SIZE;
        if(size < 1024)
            size_type = small;
        else if(size < 1048576)
            size_type = middle;
        else if(size < 524288000)
            size_type = very_large;
        next_chunk = 0;
    }
    bool chunk_state(size_t chunk_id) {
        if(!chunk_map.contains(chunk_id)) return false;
        return chunk_map.value(chunk_id).isDownloaded();
    }
    bool isDownloaded() const {
        QMapIterator<size_t, ChunkState> iter(chunk_map);
        while(iter.hasNext()) {
            iter.next();
            if(!iter.value().isDownloaded()) return false;
        }
        return true;
    }
    bool has_next_chunk() const {
        return next_chunk == chunk_count ? false : true;
    }
    size_t get_next_chunk() {
        if(next_chunk != chunk_count) ++next_chunk;
        return next_chunk;
    }
    void set_chunk_state(const size_t chunk_id, const size_t bytes_written) {
        chunk_map[chunk_id].change_state(bytes_written);
        if(chunk_map[chunk_id].isDownloaded()) finished_chunk.insert(chunk_id, chunk_id);
    }
private:
    size_t chunk_count;
    size_t ending_size;
    size_t next_chunk;
    SizeTypes size_type;
    QMap<size_t, ChunkState> chunk_map;
    QList<size_t> finished_chunk;
};

#endif // OBJECTLOADER_H
