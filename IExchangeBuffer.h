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
#ifndef IEXCHANGEBUFFER_H
#define IEXCHANGEBUFFER_H
#pragma once
#include <QSharedPointer>
#include <QQueue>

#include <atomic>

template<class T>
class IExchangeBuffer {
public:
    virtual ~IExchangeBuffer() {}

    virtual bool try_to_read_item(QSharedPointer<T>& item) = 0;
    virtual bool add_item(QSharedPointer<T>& item) = 0;
};

template<class T>
class BaseExchangeBuffer : public IExchangeBuffer<T> {
public:
    virtual ~BaseExchangeBuffer() {
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
            if(first_queue.isEmpty()) {
                read_queue_id = 0;
                return false;
            }
            item = first_queue.dequeue();
            if(first_queue.isEmpty()) read_queue_id = 0;
        }
        else if(read_queue_id == 2) {
            if(second_queue.isEmpty()) {
                read_queue_id = 0;
                return false;
            }
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
