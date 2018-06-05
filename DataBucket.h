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
#ifndef DATABUCKET
#define DATABUCKET
#pragma once
#include <QtCore>

#include <iostream>

#define DEFAULTBUCKETSIZE 100

template<class T = int>
struct DataBucket {
    explicit DataBucket(unsigned int size_ = DEFAULTBUCKETSIZE) : filled(0) {
        size = size_;
        data = new T(size);
        data_ptr.reset(data);
    }
    virtual ~DataBucket() {
        reset();
        delete data;
    }

    virtual void reset() {
        if(!data_ptr.isNull()) data_ptr.reset();
    }
    virtual int add_value(T& value, T& old_value) {
        if(filled == 0) {
            data[0] = value;
            ++filled;
            return 0;
        }
        else if(filled < size) {
            int index = calc_value_index(value);
            int count = filled-index;
            for(int i = count; i > 0; --i) {
                data[index+i] = data[index+i-1];
            }
            data[index] = value;
            ++filled;
            return 0;
        }
        else {
            int index = calc_value_index(value);
            if(index == size) {
                old_value = value;
                return 1;
            }
            else if(index == 0) {
                old_value = value;
                return -1;
            }
            else if(index >= size/2) {
                int count = size-index-1;
                old_value = data[size-1];
                for(int i = count; i > 0; --i) {
                    data[index+i] = data[index+i-1];
                }
                data[index] = value;
                return 1;
            }
            else {
                --index;
                int count = index;
                old_value = data[0];
                for(int i = count; i > 0; --i) {
                    data[index-i] = data[index-i+1];
                }
                data[index] = value;
                return -1;
            }
        }
    }
    void get_data_ptr() {
        return data_ptr;
    }
    void print_values() {
        T* data = data_ptr.data();
        for(int i = 0; i < filled; ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }
protected:
    int calc_value_index(T& value) {
        int left_bound = 0;
        int right_bound = filled-1;
        int middle = 0;
        int index;
        while(left_bound < right_bound) {
            middle = (left_bound + right_bound) / 2;
            if(data[middle] == value) {
                return middle+1;
            }
            else if(data[middle] > value) {
                right_bound = middle;
            }
            else {
                left_bound = middle+1;
            }
        }
        return data[right_bound] > value ? right_bound : right_bound+1;
    }
private:
    T* data;
    QSharedPointer<T> data_ptr;
    unsigned int size;
    unsigned int filled;
};

#endif // DATABUCKET

