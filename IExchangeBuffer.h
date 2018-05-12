#ifndef IEXCHANGEBUFFER_H
#define IEXCHANGEBUFFER_H
#pragma once
#include <QSharedPointer>
#include <QQueue>

#include <atomic>


template<class T>
class ExchangeBuffer {
public:
    virtual ~ExchangeBuffer() {
        reset();
    }

    virtual void reset() {
        read_queue_id = 0;
        write_queue_id = 1;
        if(!first_queue.isEmpty()) {
            foreach(QSharedPointer<T> item, first_queue) {
                item.reset();
            }
            first_queue.clear();
        }
        if(second_queue.isEmpty()) return;
        foreach(QSharedPointer<T> item, second_queue) {
            item.reset();
        }
        second_queue.clear();
    }
    virtual bool try_to_read_item(QSharedPointer<T>& item) {
        if(read_queue_id == 0)
            return false;
        else if(read_queue_id == 1) {
            if(first_queue.isEmpty()) return false;
            item = first_queue.dequeue();
            if(first_queue.isEmpty()) read_queue_id = 0;
        }
        else if(read_queue_id == 2) {
            if(second_queue.isEmpty()) return false;
            item = second_queue.dequeue();
            if(second_queue.isEmpty()) read_queue_id = 0;
        }
        return true;
    }
    virtual bool add_item(QSharedPointer<T>& item) {
        if(write_queue_id == 1) {
            first_queue.enqueue(item);
            if(read_queue_id == 0) {
                read_queue_id = write_queue_id;
                write_queue_id = 2;
            }
        }
        else if(write_queue_id == 2) {
            second_queue.enqueue(item);
            if(read_queue_id == 0) {
                read_queue_id = write_queue_id;
                write_queue_id = 1;
            }
        }
    }
private:
    std::atomic<int> read_queue_id;
    std::atomic<int> write_queue_id;
    QQueue<QSharedPointer<T>> first_queue;
    QQueue<QSharedPointer<T>> second_queue;
};

#endif // IEXCHANGEBUFFER
