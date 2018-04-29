#ifndef COLLECTOR
#define COLLECTOR
#pragma once
#include <QMap>
#include <QSharedPointer>

#include "action.h"

struct Conditions {
    virtual ~Conditions() {
        op_confirmation.clear();
        in_data_available.clear();
    }

    virtual bool conditions_satisfy() {
        QMap<QString, bool>::iterator opc_iter = op_confirmation.begin();
        while(opc_iter != op_confirmation.end()) {
            if(!opc_iter.value()) return false;
        }
        QMap<QString, QString>::iterator ind_iter = in_data_available.begin();
        while(ind_iter != in_data_available.end()) {
            if(ind_iter.value() == QString("Undefined")) return false;
        }
    }
    QMap<QString, bool> op_confirmation;
    QMap<QString, QString> in_data_available;
};

class WorkFlowCollector : public QObject {
    Q_OBJECT
public:
    WorkFlowCollector(QObject* parent = NULL) : QObject(parent), signal_count(0), num_recieved_signal(0) {}
    virtual ~WorkFlowCollector() {
        conditions.reset();
    }

    virtual void register_data_condition(const IAction* action) {
        conditions->in_data_available.insert(action->get_action_id(), "Undefined");
        connect(action, SIGNAL(dataset_ready(const QStringList&)), this, SLOT(dataset_ready(const QStringList&)));
        ++signal_count;
    }
    virtual void register_confirmation(const QString guardian_id, bool guardian_state = false) {
        conditions->op_confirmation.insert(action->get_action_id(), guardian_state);
    }
    virtual void register_confirmation(const IAction* action) {
        conditions->op_confirmation.insert(action->get_action_id(), false);
        connect(action, SIGNAL(op_ready(const QString&)), this, SLOT(op_ready(const QString&)));
        ++signal_count;
    }
    virtual void register_out_action(const IAction* action) {
        connect(this, SIGNAL(start(void)), action, SLOT(execute(void)));
    }
    virtual void check_state() {
        if(conditions->conditions_satisfy()) emit start();
    }
public slots:
    void dataset_ready(const QStringList& dataset_id) {
        this->conditions->in_data_available.insert(dataset_id.pop_front(), dataset_id.pop_front());
        ++num_recieved_signal;
        if(num_recieved_signal >= signal_count) check_state();
    }
    void op_ready(const QString& action_id) {
        this->conditions->op_confirmation.insert(action_id, true);
        ++num_recieved_signal;
        if(num_recieved_signal >= signal_count) check_state();
    }
signals:
    void start(void);
private:
    unsigned int signal_count;
    unsigned int num_recieved_signal;
    QSharedPointer<Conditions> conditions;
};

#endif // COLLECTOR


