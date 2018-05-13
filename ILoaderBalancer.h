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
#ifndef LOADERBALANCER_H
#define LOADERBALANCER_H
#pragma once
#include <QtCore>

#define DEFAULT_MEASUREMENT_QUEUE_SIZE 10

/*Interface of performance predictor*/
template<int state_size = 5, class T = double>
class IPredictor {
public:
    virtual ~IPredictor() {}

    virtual void set_state(const QVector<T>& state_) = 0;
    virtual QVector<T> get_state() const = 0;
    virtual bool update_state(const QList<T> measurements) = 0;
    virtual T calc_score() = 0;
};

/*Base implementation of performance predictor*/
template<class T = double, int state_size = 5>
class BasePredictor : public IPredictor<state_size, T> {
    struct Measurement {
        virtual ~Measurement() {
            clear();
        }

        virtual void clear() {
            if(!measurement.isEmpty()) measurement.clear();
        }
        virtual void set_data(const QList<T>& measurement_) {
            time = QDateTime::currentDateTime();
            if(!measurement.isEmpty()) measurement.clear();
            measurement.append(measurement_);
        }
        QDateTime time;
        QVector<T> measurement;
    };
public:
    BasePredictor() {
        state.resize(state_size);
    }
    virtual ~BasePredictor() {
        reset();
    }

    virtual void reset() {
        if(!state.isEmpty()) state.clear();
        if(!coefficient_state_to_score.isEmpty())  coefficient_state_to_score.clear();
        if(measurements.isEmpty()) return;
        foreach(QSharedPointer<Measurement> measurement, measurements) {
            measurement->clear();
        }
        measurements.clear();
    }
    virtual void set_state(const QVector<T>& state_) {
        if(!state.isEmpty()) state.clear();
        state.append(state_);
    }
    virtual QVector<T> get_state() const {
        return state;
    }
    virtual void set_coefficient_state_to_score(const QVector<T>& coefficient_state_to_score_) {
        if(!coefficient_state_to_score.isEmpty()) coefficient_state_to_score.clear();
        coefficient_state_to_score.append(coefficient_state_to_score_);
    }
    virtual T calc_score() {
        T score = 0;
        for(int i = 0; i < state.size(); i++) {
            score += coefficient_state_to_score.at(i) * state.at(i);
        }
        return score;
    }
    virtual bool update_state(const QList<T>& measurement) {
        if(measurements.size() >= DEFAULT_MEASUREMENT_QUEUE_SIZE) {
            measurements.dequeue().reset();
        }
        QSharedPointer<Measurement> new_measurement_ptr(new Measurement());
        new_measurement_ptr->set_data(measurement);
        measurements.enqueue(new_measurement_ptr);
        if(!calc_new_state()) return false;
        return true;
    }
    virtual bool calc_new_state() = 0;
private:
    QVector<T> state;
    QVector<T> coefficient_state_to_score;
    QQueue<QSharedPointer<Measurement> > measurements;
};

/*Node performance descriptor*/
template<class T = double>
struct Node_performance_desc {
    virtual ~Node_performance_desc() {}

    virtual void reset() {
        if(predictor_ptr.isNull()) return;
        predictor_ptr->reset();
        predictor_ptr.reset();
    }
    virtual void set_url(QUrl& address_) {
        address.swap(address_);
    }
    virtual QUrl get_url() const {
        return address;
    }
    virtual T update_performance_score(const QList<T>& measurement, const T& defVal = T()) {
        if(!predictor_ptr->update_state(measurement)) return defVal;
        return predictor_ptr->calc_score();
    }
    virtual T get_performance_score() {
        return predictor_ptr->calc_score();
    }
private:
    QUrl address;
    QSharedPointer<BasePredictor<T>> predictor_ptr;
};

template<class T = double>
struct NodeScore {
    NodeScore(const QString node_id_ = "", const T value = T()) {
        node_id = node_id_;
        score = value;
    }
    static bool moreThen(const NodeScore& n1, const NodeScore& n2) {
        return n1.score > n2.score;
    }
    QString node_id;
    T score;
};

/*Load balancer implementation*/
class BaseLoadBalancer : public QObject {
    Q_OBJECT
public:
    BaseLoadBalancer(QObject* parent = NULL) : QObject(parent) {}
    virtual ~BaseLoadBalancer() {
        reset();
    }

    virtual void reset() {
        if(descriptors.isEmpty()) return;
        foreach(QSharedPointer<Node_performance_desc<> > descriptor, descriptors) {
            descriptor->reset();
            descriptor.reset();
        }
        descriptors.clear();
    }
    virtual void add_new_node_desc(const QString& node_id, QSharedPointer<Node_performance_desc<> >& descriptor) {
        descriptors.insert(node_id, descriptor);
    }
    virtual bool delete_node_desc(const QString& node_id) {
        if(!descriptors.contains(node_id)) return false;
        descriptors[node_id].reset();
        return true;
    }
    virtual bool get_node_desc(const QString& node_id, QSharedPointer<Node_performance_desc<> >& descriptor) {
        if(!descriptors.contains(node_id)) return false;
        descriptor = descriptors[node_id];
        return true;
    }
    template<class T>
    bool update_node_desc(const QString& node_id, const QList<T>& measurement) {
        if(!descriptors.contains(node_id)) return false;
        descriptors.value(node_id)->update_performance_score(measurement);
        return true;
    }
    virtual bool get_best_nodes_id(const int number, QList<NodeScore<> >& best_nodes) {
        if(descriptors.isEmpty()) return false;
        if(!best_nodes.isEmpty()) best_nodes.clear();
        best_nodes.reserve(number);
        QHashIterator<QString, QSharedPointer<Node_performance_desc<> > > desc_iter(descriptors);
        while(desc_iter.hasNext()) {
            desc_iter.next();
            NodeScore<> node(desc_iter.key(), desc_iter.value()->get_performance_score());
            score_list.append(node);
        }
        qSort(score_list.begin(), score_list.end(), NodeScore<>::moreThen);
        for(int i = 0; i < number; ++i) {
            best_nodes.append(score_list[i]);
        }
        return true;
    }
private:
    QHash<QString, QSharedPointer<Node_performance_desc<> > > descriptors;
    QList<NodeScore<> > score_list;
};
#endif // LOADERBALANCER_H
