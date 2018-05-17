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

    virtual void reset() {
        data_ptr.reset();
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
            item = data_ptr.data()[index - size];
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
