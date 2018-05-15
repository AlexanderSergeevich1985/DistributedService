#ifndef REPLICATIONMODEL_H
#define REPLICATIONMODEL_H
#pragma once
#include <QtCore>

#include "objectmanager.h"

/*This file contains description of tree data structure used for representation of replication model and model itself*/

template<class T = Idata_wraper>
class ReplicaDescriptor {
public:
    virtual ~ReplicaDescriptor() {
        reset();
    }

    virtual void reset() {
        if(!replica_ptr.isNull()) replica_ptr.reset();
    }
private:
    QDateTime creation_time;
    QString replica_id;
    unsigned int version_number;
    QSharedPointer<T> replica_ptr;
};

class LeafReplicaHolder {
public:
    virtual ~LeafReplicaHolder() {
        reset();
    }

    virtual void reset() {
        if(replica_descriptors.isEmpty()) return;
        foreach(ReplicaDescriptor<> item, replica_descriptors) {
            item.reset();
        }
        replica_descriptors.clear();
    }
    bool is_contains_replica_version(const unsigned int& replica_version) {
        if(!replica_descriptors.contains(replica_version)) return false;
        return true;
    }
    bool get_replica_desc_ptr(const unsigned int& replica_version, QSharedPointer<ReplicaDescriptor<> >& rep_desc_ptr) {
        if(!replica_descriptors.contains(replica_version)) return false;
        rep_desc_ptr.reset(&replica_descriptors[replica_version]);
        return true;
    }
    void set_replica_desc(const unsigned int replica_version, ReplicaDescriptor<>& replica_desc) {
        replica_descriptors.insert(replica_version, replica_desc);
    }
    bool delete_replica_desc(const unsigned int replica_version) {
        if(!replica_descriptors.contains(replica_version)) return false;
        replica_descriptors[replica_version].reset();
        replica_descriptors.remove(replica_version);
        return true;
    }
    bool get_versions_list(QList<unsigned int>& versions_list) {
        if(replica_descriptors.isEmpty()) return false;
        if(!versions_list.isEmpty()) versions_list.clear();
        versions_list.append(replica_descriptors.keys());
        return true;
    }
    void set_node_holder_id(QString node_holder_id_) {
        node_holder_id = node_holder_id_;
    }
    const QString get_node_holder_id() const {
        return node_holder_id;
    }
private:
    QString node_holder_id;
    QMap<unsigned int, ReplicaDescriptor<> > replica_descriptors;
};

struct SiblingNodeDesc {
    virtual ~SiblingNodeDesc() {}

    virtual void reset() {
        node_holder_url.clear();
        if(!available_versions.isEmpty()) available_versions.clear();
    }
    QString node_holder_id;
    QUrl node_holder_url;
    /*Available versions of replica and count of defined version replica in branch of replica tree*/
    QMap<unsigned int, unsigned int> available_versions;
};

class InternalReplicaHolder : public LeafReplicaHolder {
public:
    virtual ~InternalReplicaHolder() {
        reset();
    }

    virtual void reset() {
        LeafReplicaHolder::reset();
        if(replica_child_nodes_desc.isEmpty()) return;
        foreach(SiblingNodeDesc item, replica_child_nodes_desc) {
            item.reset();
        }
        replica_child_nodes_desc.clear();
    }
    bool get_sibling_node_desc(const QString node_id, QSharedPointer<SiblingNodeDesc>& sibling_node_desc_ptr) {
        if(!replica_child_nodes_desc.contains(node_id)) return false;
        sibling_node_desc_ptr.reset(&replica_child_nodes_desc[node_id]);
        return true;
    }
    void set_sibling_node_desc(const QString node_id, SiblingNodeDesc& sibling_node_desc) {
        replica_child_nodes_desc.insert(node_id, sibling_node_desc);
    }
    bool delete_sibling_node_desc(const QString node_id) {
        if(!replica_child_nodes_desc.contains(node_id)) return false;
        replica_child_nodes_desc[node_id].reset();
        replica_child_nodes_desc.remove(node_id);
        return true;
    }
    bool get_child_nodes_id(QList<QString>& nodes_id_list) {
        if(replica_child_nodes_desc.isEmpty()) return false;
        if(!nodes_id_list.isEmpty()) nodes_id_list.clear();
        nodes_id_list.append(replica_child_nodes_desc.keys());
        return true;
    }
    bool is_branch_contains_rep_version(const unsigned int replica_version, QList<QString>& nodes_id_list) {
        if(replica_child_nodes_desc.isEmpty()) return false;
        if(!nodes_id_list.isEmpty()) nodes_id_list.clear();
        QMapIterator<QString, SiblingNodeDesc> desc_iter(replica_child_nodes_desc);
        while(desc_iter.hasNext()) {
            desc_iter.next();
            if(!desc_iter.value().available_versions.contains(replica_version)) continue;
            nodes_id_list.append(desc_iter.key());
        }
        return true;
    }
    unsigned int count_rep_version(const unsigned int replica_version) {
        unsigned int total_count = 0;
        if(is_contains_replica_version(replica_version)) ++total_count;
        if(replica_child_nodes_desc.isEmpty()) return total_count;
        QMapIterator<QString, SiblingNodeDesc> desc_iter(replica_child_nodes_desc);
        while(desc_iter.hasNext()) {
            desc_iter.next();
            if(!desc_iter.value().available_versions.contains(replica_version)) continue;
            total_count += desc_iter.value().available_versions.value(replica_version);
        }
        return total_count;
    }
private:
    QMap<QString, SiblingNodeDesc> replica_child_nodes_desc;
};

struct RequestDesc {
    enum RequestType {
        ReadReplica,
        UpdateReplica,
        DeleteReplica
    };

    RequestType type;
    QString req_node_id;
    QUrl req_node_url;
    unsigned int req_rep_version;
};

class PrimaryReplicaHolder : public InternalReplicaHolder {
public:
    virtual ~PrimaryReplicaHolder() {
        reset();
    }

    virtual void reset() {
        InternalReplicaHolder::reset();
        latest_replica_version = recent_deleted_replica_version = 0;
        if(!request_queue.isEmpty()) request_queue.clear();
    }
    bool register_new_request(RequestDesc& new_req_desc) {
        switch(new_req_desc.type) {
        case RequestDesc::ReadReplica:
            new_req_desc.req_rep_version = latest_replica_version;
            request_queue.enqueue(new_req_desc);
            break;
        case RequestDesc::UpdateReplica:
            ++latest_replica_version;
            new_req_desc.req_rep_version = latest_replica_version;
            request_queue.enqueue(new_req_desc);
            break;
        case RequestDesc::DeleteReplica:
            if(count_rep_version(new_req_desc.req_rep_version) == 0) return false;
            request_queue.enqueue(new_req_desc);
            break;
        default:
            return false;
        }
        return true;
    }
    bool retrieve_next_request(RequestDesc& next_req_desc) {
        next_req_desc = request_queue.dequeue();
        if(auto_delete && next_req_desc.type == RequestDesc::ReadReplica && next_req_desc.req_rep_version > recent_deleted_replica_version) {
            RequestDesc new_req_desc;
            new_req_desc.req_node_id = get_node_holder_id();
            ++recent_deleted_replica_version;
            new_req_desc.req_rep_version = recent_deleted_replica_version;
            request_queue.enqueue(new_req_desc);
        }
        return true;
    }
    bool has_next_request() const {
        return request_queue.isEmpty() ? false : true;
    }
    void set_auto_delete(const bool auto_delete_) {
        auto_delete = auto_delete_;
    }
private:
    unsigned int latest_replica_version;
    unsigned int recent_deleted_replica_version;
    bool auto_delete;
    QQueue<RequestDesc> request_queue;
};

#endif // REPLICATIONMODEL_H
