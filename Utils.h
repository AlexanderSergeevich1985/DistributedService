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
#ifndef UTILS_H
#define UTILS_H
#pragma once
#include <QtCore>

#define STARTCAPACITY 10
#define STEP 2

namespace specialstruct {

/*Mixture of dynamic array and linked list data structure*/
template<class T = double>
class MixArray {
public:
    MixArray(const unsigned int init_size = STARTCAPACITY) : size(init_size), filled(0) {
        data_ptr.reset(new T[init_size]());
    }
    virtual ~MixArray() {
        reset();
    }
    
    virtual void reset(const unsigned int init_size = 0) {
        if(!next_chunk_ptr.isNull()) {
            next_chunk_ptr->reset();
            next_chunk_ptr.reset();
        }
        if(!data_ptr.isNull()) {
            filled = 0;
            size = init_size;
            if(init_size != 0) {
                data_ptr.reset(new T[init_size]());
            }
            else
                data_ptr.reset();
        }
    }
    virtual void add_item(T& item) {
        ++filled;
        if(filled < size)
            data_ptr.data()[filled] = item;
        else if(next_chunk_ptr.isNull()) {
            next_chunk_ptr.reset(new MixArray<T>(size*STEP));
            next_chunk_ptr->add_item(item);
        }
        else
            next_chunk_ptr->add_item(item);
    }
    virtual bool get_item(const int index, T& item) {
        if(index < filled)
            item = data_ptr.data()[index];
        else if(!next_chunk_ptr.isNull()) {
            return next_chunk_ptr->get_item(index - size, item);
        }
        else {
            return false;
        }
        return true;
    }
private:
    unsigned int size, filled;
    QSharedPointer<T> data_ptr;
    QSharedPointer<MixArray<T> > next_chunk_ptr;
};

}

#endif // UTILS_H
