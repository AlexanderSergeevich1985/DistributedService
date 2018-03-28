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
#ifndef CERTIFICATE_H
#define CERTIFICATE_H
#include <QTime>
#include <algorithm>
#include <QDebug>

#define MINRANDTIMES 5
#define MAXRANDTIMES 20

class RandNumGen {
public:
    virtual ~RandNumGen() {
    }
    /*Generate value for initializing random number generating process*/
    virtual uint gen_process() {
        QTime time = QTime::currentTime();
        return (uint)time.msec();
    }
    /*Generate limited arbitrary value for initializing random number generating process*/
    int rand_int_limited(int low, int high) {
        qsrand(gen_process());
        return qrand() % ((high + 1) - low) + low;
    }
    /*Random number main generating process*/
    int rand_gen_cycle(const uint times) {
        QList<uint> rand_values;
        qsrand(gen_process());
        /*Generate random numbers n times*/
        for(int i = 0; i < times; ++i) {
            qsrand(qrand());
            rand_values.append(qrand());
        }
        /*Shuffle random numbers and retrieve one*/
        std::random_shuffle(rand_values.begin(), rand_values.end());
        uint first = rand_values.takeFirst();
        /*Shuffle random numbers and retrieve one*/
        std::random_shuffle(rand_values.begin(), rand_values.end());
        uint second = rand_values.takeFirst();
        uint min_value = std::min(first, second);
        uint max_value = std::max(first, second);
        qsrand(qrand());
        /*Generate output random number*/
        return qrand() % ((max_value + 1) - min_value) + min_value;
    }
    /*Random number main generating process with limited number of repeats*/
    int rand_gen_cycle_limited(const uint max_times = MAXRANDTIMES) {
        uint times = rand_int_limited(MINRANDTIMES, max_times);
        return rand_gen_cycle(times);
    }
    /*Arbitrary limited value generating process with limited number of repeats*/
    int rand_int_cycle_limited(int low, int high) {
        uint times = rand_int_limited(MINRANDTIMES, MAXRANDTIMES);
        return rand_gen_cycle(times) % ((high + 1) - low) + low;
    }
};

#endif // CERTIFICATE_H
