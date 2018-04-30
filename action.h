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
#ifndef ACTION_H
#define ACTION_H
#pragma once
#include <QMap>
#include <QSharedPointer>
#include <QTimer>
#include <QDebug>
#include "objectmanager.h"

class IAction : public QObject {
    Q_OBJECT
public:
    IAction(QObject* parent = NULL) : QObject(parent) {
        is_allowed = false;
    }
    virtual ~IAction() {}
    virtual bool Execute_command() = 0;
    virtual const QString get_action_id() const = 0;
    void set_is_allowed(const bool is_Allowed) {
        is_allowed = is_Allowed;
    }
    bool get_is_allowed() {
        return is_allowed;
    }
public slots:
    virtual void execute(void) {
        if(is_allowed) this->Execute_command();
    }
signals:
    void error(const char* err);
    void dataset_ready(const QStringList& dataset_id);
    void op_ready(const QString& dataset_id);
private:
    bool is_allowed;
};

class BaseAction : public IAction {
    Q_OBJECT
public:
    BaseAction(QObject* parent = NULL) : IAction(parent) {}
    virtual ~BaseAction() {}
    virtual bool Execute_command() {
        try {
            QMetaObject::invokeMethod(object.data(), "work", Q_ARG(void*, NULL));
        }
        catch(const std::runtime_error& err) {
            emit error(err.what());
            return false;
        }
        return true;
    }
    void set_object(QObject* object) {
        this->object.reset(object);
    }
    QSharedPointer<QObject> get_object() {
        return object;
    }
    void set_action_id(QString action_id) {
        this->action_id = action_id;
    }
    const QString get_action_id() const {
        return action_id;
    }
    void set_data_container(Idata_wraper* arguments) {
        this->arguments.reset(arguments);
    }
    QSharedPointer<Idata_wraper> get_data_container() {
        return arguments;
    }
private:
    QSharedPointer<QObject> object;
    QString action_id;
    QSharedPointer<Idata_wraper> arguments;
};

class TimerAction : public BaseAction {
    Q_OBJECT
public:
    TimerAction(QObject* parent = NULL) : BaseAction(parent) {
        timer.reset(new QTimer(this));
        connect(timer.data(), SIGNAL(timeout()), this, SLOT(doIt()));
    }
public slots:
    void execute() {
        timer->start(1000);
    }
protected slots:
    void doIt() {
        this->Execute_command();
    }
private:
    QSharedPointer<QTimer> timer;
};

class ICommand : public QObject {
    Q_OBJECT
public:
    ICommand(QObject* parent = NULL) : QObject(parent) {
        is_allowed = false;
    }
    virtual ~ICommand() {}
    void set_is_allowed(const bool is_Allowed) {
        is_allowed = is_Allowed;
    }
    bool get_is_allowed() {
        return is_allowed;
    }
    void set_command_id(const QString& command_id) {
        this->command_id = command_id;
    }
    const QString get_command_id() const {
        return command_id;
    }
public slots:
    void Execute(void) {
        if(is_allowed) emit execute_command();
    }
signals:
    void execute_command(void);
private:
    bool is_allowed;
    QString command_id;
};

class ISwitch : public QObject {
public:
    virtual void add_command(const QString& command_id, ICommand* command_ptr) = 0;
    virtual bool delete_command(const QString& command_id) = 0;
    virtual void add_action(const QString& action_id, IAction* action_ptr) = 0;
    virtual bool delete_action(const QString& action_id) = 0;
    virtual bool bind_command_to_action(const QString& command_id, const QString& action_id) = 0;
    virtual bool unbind_command_to_action(const QString& command_id, const QString& action_id) = 0;
    virtual bool execute_command(const QString& command_id) = 0;
};

class ActionChooser : public ISwitch {
public:
    ActionChooser() {}
    virtual ~ActionChooser() {
        clear();
    }
    void add_command(const QString& command_id, ICommand* command_ptr) {
        commands.insert(command_id, QSharedPointer<ICommand>(command_ptr));
    }
    void add_action(const QString& action_id, IAction* action_ptr) {
        actions.insert(action_id, QSharedPointer<IAction>(action_ptr));
    }
    bool delete_command(const QString& command_id) {
        if(!commands.contains(command_id)) return false;
        commands.remove(command_id);
        return true;
    }
    bool delete_action(const QString& action_id) {
        if(!commands.contains(action_id)) return false;
        commands.remove(action_id);
        return true;
    }
    bool bind_command_to_action(const QString& command_id, const QString& action_id) {
        if(!commands.contains(command_id) || !actions.contains(action_id)) return false;
        active_connections.insert(command_id, action_id);
        connect(commands[command_id].data(), SIGNAL(execute_command()), actions[action_id].data(), SLOT(execute()));
    }
    bool unbind_command_to_action(const QString& command_id, const QString& action_id) {
        if(!commands.contains(command_id) || !actions.contains(action_id)) return false;
        disconnect(commands[command_id].data(), SIGNAL(execute_command()), actions[action_id].data(), SLOT(execute()));
    }
    virtual bool execute_command(const QString& command_id) {
        if(!commands.contains(command_id)) return false;
        const QSharedPointer<ICommand>& command = commands.value(command_id);
        command->Execute();
    }
    void clear() {
        foreach(QSharedPointer<ICommand> command_ptr, commands) {
            command_ptr.reset();
        }
        commands.clear();
        foreach(QSharedPointer<IAction> action_ptr, actions) {
            action_ptr.reset();
        }
        actions.clear();
    }
private:
    QMap<QString, QSharedPointer<ICommand> > commands;
    QMap<QString, QSharedPointer<IAction> > actions;
    QHash<QString, QString> active_connections;
};

#endif // ACTION_H
