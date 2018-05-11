#ifndef IPROTOCOL_H
#define IPROTOCOL_H
#pragma once
#include <QtCore>

class IMsgPacket {
public:
    virtual ~IMsgPacket() {}

    virtual QByteArray get_msg() = 0;
    virtual void set_msg_fields_values(const QLinkedList<QVariant>& args) = 0;
};

class IMsgFactory {
public:
    virtual ~IMsgFactory() {}

    virtual bool create_msg(const QString& msg_type, IMsgPacket& msg_packet) = 0;
};

struct Questionnaire {
    struct QuestionnaireField {
        virtual ~QuestionnaireField() {
            reset();
        }

        void reset() {
            if(!fields.isEmpty()) fields.clear();
        }
        unsigned int msg_id;
        QMap<QString, QVariant> fields;
    };
    virtual ~Questionnaire() {
        reset();
    }

    void reset() {
        if(fields_store.isEmpty()) return;

        foreach(QSharedPointer<QuestionnaireField> field_ptr, fields_store) {
            field_ptr->reset();
        }
        fields_store.clear();
    }
    QLinkedList<QSharedPointer<QuestionnaireField> > fields_store;
};

class ISessionStates {
public:
    enum State {
        Listen,
        Speak,
        Ignore
    };
    ISessionStates() : msgs_info_ptr(new Questionnaire) {}
    virtual ~ISessionStates() {}

    virtual bool calc_new_state(const QSharedPointer<IMsgPacket> received_msg, State& new_state) = 0;
    virtual bool get_new_msg(QByteArray& send_msg) = 0;
    virtual long get_timeout() = 0;
    QSharedPointer<Questionnaire> get_retrieved_info() const {
        return msgs_info_ptr;
    }
private:
    QSharedPointer<Questionnaire> msgs_info_ptr;
};

class BaseProtocol {
public:
    virtual ~BaseProtocol() {}

    void set_protocol_id(const QString protocol_id_) {
        protocol_id = protocol_id_;
    }
    const QString get_protocol_id() const {
        return protocol_id;
    }
    void set_state_holder(QSharedPointer<ISessionStates>& state_ptr_) {
        state_ptr = state_ptr_;
    }
    const QSharedPointer<ISessionStates> get_state_holder() const {
        return state_ptr;
    }
    virtual bool set_new_msg(const QSharedPointer<IMsgPacket> received_msg, QByteArray& send_msg, long& timeout) {
        ISessionStates::State new_state;
        if(!state_ptr->calc_new_state(received_msg, new_state)) return false;
        switch(new_state) {
        case ISessionStates::Listen:
            if(!send_msg.isEmpty()) send_msg.clear();
            timeout = 0;
            break;
        case ISessionStates::Speak:
            if(!state_ptr->get_new_msg(send_msg)) return false;
            break;
        case ISessionStates::Ignore:
            timeout = state_ptr->get_timeout();
            break;
        default:
            break;
        }
        return true;
    }
private:
    QString protocol_id;
    QSharedPointer<ISessionStates> state_ptr;
};

#endif // IPROTOCOL_H
