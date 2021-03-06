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
#ifndef OPBROKER
#define OPBROKER
#pragma once
#include <QMap>
#include <QDateTime>
#include <QSharedPointer>

#define FORGET_TIME 1000000
#define FAULT_THRESHOLD 1000000

/*Fault descriptor*/
struct Fault_desc {
    virtual ~Fault_desc() {}

    virtual unsigned int calc_fault_impact() {
        QDateTime current_time = QDateTime::currentDateTime();
        qint64 millisecondsDiff = current_time.msecsTo(fault_time);
        return (millisecondsDiff >= FORGET_TIME) ? 0 : ((double)(FORGET_TIME - millisecondsDiff)/FORGET_TIME) * fault_score;
    }
    QDateTime fault_time;
    unsigned int fault_score;
};

/*Node fault statistic for circuit breaker implementation*/
struct SimpleNodeStatistic {
    virtual ~SimpleNodeStatistic() {
        reset();
    }

    void reset() {
        faults.clear();
    }
    void add_failure(const Fault_desc& fault) {
        faults.append(fault);
    }
    bool is_overflowed() {
        unsigned int total_fault_score = 0;
        calc_total_fault_score(total_fault_score);
        return total_fault_score >= FAULT_THRESHOLD ? true : false;
    }
    void calc_total_fault_score(unsigned int& total_fault_score) {
        total_fault_score = 0;
        QMutableListIterator<Fault_desc> faultsIter(faults);
        while(faultsIter.hasNext()) {
            Fault_desc& fault = faultsIter.next();
            unsigned int impact = fault.calc_fault_impact();
            if(impact == 0)
                faultsIter.remove();
            else
                total_fault_score += impact;
        }
    }
    QString fault_source_id;
    QList<Fault_desc> faults;
};

/*Circuit breaker programming pattern implementation*/
class BaseCircuitBreaker : public QObject {
    Q_OBJECT
public:
    enum CircuitState {
        Closed = 0,
        Open,
        Half_Open
    };
    BaseCircuitBreaker(QObject* parent = 0) : QObject(parent) {
        statistic.reset(new SimpleNodeStatistic());
        state = Closed;
        banned_forever = false;
    }
    virtual ~BaseCircuitBreaker() {}

    virtual void reset() {
        state = Closed;
        statistic->reset();
        banned_forever = false;
    }
    bool register_fault(Fault_desc& fault) {
        if(state == Closed) {
            statistic->add_failure(fault);
            if(statistic->is_overflowed()) {
                state = Open;
                if(!banned_forever) emit start_recovery(statistic->fault_source_id);
                return false;
            }
        }
        else if(state == Half_Open) {
            fault.fault_score = FAULT_THRESHOLD;
            statistic->add_failure(fault);
            state = Open;
            if(!banned_forever) emit start_recovery(statistic->fault_source_id);
            return false;
        }
        return true;
    }
    bool isAllowed(unsigned int& total_fault_score) {
        if(banned_forever) return false;
        if(state == Closed)
            return true;
        else if(state == Half_Open) {
            statistic->calc_total_fault_score(total_fault_score);
            if(total_fault_score < 0.5 * FAULT_THRESHOLD) {
                state = Closed;
            }
            return true;
        }
        else {
            statistic->calc_total_fault_score(total_fault_score);
            if(total_fault_score < FAULT_THRESHOLD) {
                state = Half_Open;
                return true;
            }
        }
        return false;
    }
    CircuitState get_state() {
        return state;
    }
    void set_banned(bool banned_forever_ = true) {
        if(banned_forever_) state = Open;
        banned_forever = banned_forever_;
    }
    QString get_fault_source_id() {
        return statistic->fault_source_id;
    }
signals:
    void start_recovery(QString);
private:
    bool banned_forever;
    CircuitState state;
    QSharedPointer<SimpleNodeStatistic> statistic;
};

#endif // OPBROKER
