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
#ifndef ITOPOLOGY_H
#define ITOPOLOGY_H
#pragma once
#include <QtCore>

#include "objectmanager.h"

/*The main idea behind what present below conclude in ability to change
 * the graph data structure by changing stored metadata and way how
 * the metadata interpreted without changing graph topology*/

struct IDataScheme {
    struct DataFieldDesc {
        unsigned int field_id;
        bool available;
        QMetaType field_type;
    };
    virtual ~IDataScheme() {
        reset();
    }

    virtual void reset() {
        if(!fields_desc.isEmpty()) fields_desc.clear();
    }
    QString data_scheme_id;
    QList<DataFieldDesc> fields_desc;
};

struct MetaInfoContainer {
    virtual ~MetaInfoContainer() {
        reset();
    }

    virtual void reset() {
        if(!values.isEmpty()) values.clear();
    }
    template<typename T>
    void set(const T& val, const int index = -1) {
        if(index < 0)
            values.append(QVariant::fromValue(val));
        else
            values[index] = QVariant::fromValue(val);
    }
    template<typename T>
    T unpack(const QVariant& var, const T& defVal = T()) {
        if(var.isValid() && var.canConvert<T>()) {
            return var.value<T>();
        }
        return defVal;
    }
    QString data_scheme_id;
    QList<QVariant> values;
};

class IConvertor {
public:
    virtual ~IConvertor() {
        reset();
    }

    virtual void reset() {
        if(!data_scheme.isNull()) data_scheme.reset();
    }
    void set_data_scheme(QSharedPointer<IDataScheme>& data_scheme_) {
        data_scheme = data_scheme_;
    }
    virtual bool convert(Idata_wraper* item, MetaInfoContainer* data) = 0;
private:
    QSharedPointer<IDataScheme> data_scheme;
};

template<class T = MetaInfoContainer>
struct INode;

template<class T = MetaInfoContainer, class T2 = MetaInfoContainer>
struct ILink {
    virtual ~ILink() {}

    virtual void reset() {
        if(!first_node_ptr.isNull()) first_node_ptr.reset();
        if(!second_node_ptr.isNull()) second_node_ptr.reset();
    }
    unsigned int link_type;
    QString link_id;
    QSharedPointer<T> meta_info;
    QSharedPointer<INode<T2> > first_node_ptr;
    QSharedPointer<INode<T2> > second_node_ptr;
};

template<class T>
struct INode {
    virtual ~INode() {
        reset();
    }

    virtual void reset() {
        if(!linked_nodes_id.isEmpty()) linked_nodes_id.clear();
    }
    unsigned int node_type;
    QString node_id;
    QSharedPointer<T> meta_info;
    QList<QString> linked_nodes_id;
};

template<class T = MetaInfoContainer, class T2 = MetaInfoContainer>
struct ITopology {
    typedef ILink<T,T2> Link;
    virtual ~ITopology() {
        reset();
    }

    virtual void reset() {
        if(!nodes.isEmpty()) {
            foreach(INode<T2> item, nodes) {
                item.reset();
            }
            nodes.clear();
        }
        if(links.isEmpty()) return;
        foreach(Link item, links) {
            item.reset();
        }
        links.clear();
    }
    QMap<QString, INode<T2> > nodes;
    QMap<QString, ILink<T, T2> > links;
};

class ITopologyController {
public:
    virtual ~ITopologyController() {}

    virtual void reset() {
        if(!topology_ptr.isNull()) {
            topology_ptr->reset();
            topology_ptr.reset();
        }
    }
    void set_node_convertor(QSharedPointer<IConvertor>& node_convertor_ptr_) {
        node_convertor_ptr = node_convertor_ptr_;
    }
    void set_link_convertor(QSharedPointer<IConvertor>& link_convertor_ptr_) {
        link_convertor_ptr = link_convertor_ptr_;
    }
    void set_topology(QSharedPointer<ITopology<> >& topology_ptr_) {
        topology_ptr = topology_ptr_;
    }
    virtual bool get_nodes_id(QList<QString>& nodes_id) {
        if(!topology_ptr->nodes.isEmpty()) return false;
        if(!nodes_id.isEmpty()) nodes_id.clear();
        nodes_id.append(topology_ptr->nodes.keys());
        return true;
    }
    virtual bool reset_node_metainfo(const QString& node_id, QSharedPointer<MetaInfoContainer>& new_metainfo) {
        if(!topology_ptr->nodes.contains(node_id)) return false;
        topology_ptr->nodes[node_id].meta_info = new_metainfo;
        return true;
    }
    virtual bool get_node_metainfo(const QString& node_id, QSharedPointer<Idata_wraper>& node_data) {
        if(!topology_ptr->nodes.contains(node_id)) return false;
        node_convertor_ptr->convert(node_data.data(), topology_ptr->nodes[node_id].meta_info.data());
        return true;
    }
    virtual bool get_links_id(QList<QString>& links_id) {
        if(!topology_ptr->links.isEmpty()) return false;
        if(!links_id.isEmpty()) links_id.clear();
        links_id.append(topology_ptr->links.keys());
        return true;
    }
    virtual bool reset_link_metainfo(const QString& link_id, QSharedPointer<MetaInfoContainer>& new_metainfo) {
        if(!topology_ptr->links.contains(link_id)) return false;
        topology_ptr->nodes[link_id].meta_info = new_metainfo;
        return true;
    }
    virtual bool get_link_metainfo(const QString& link_id, QSharedPointer<Idata_wraper>& link_data) {
        if(!topology_ptr->nodes.contains(link_id)) return false;
        link_convertor_ptr->convert(link_data.data(), topology_ptr->links[link_id].meta_info.data());
        return true;
    }
protected:
    QSharedPointer<IConvertor> node_convertor_ptr;
    QSharedPointer<IConvertor> link_convertor_ptr;
    QSharedPointer<ITopology<> > topology_ptr;
};

#endif // ITOPOLOGY_H
