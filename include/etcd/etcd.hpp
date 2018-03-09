#ifndef _ETCD_ETCD_HPP
#define _ETCD_ETCD_HPP

#include <etcd/member.hpp>
#include <etcd/put_request.hpp>
#include <etcd/grpc_error.hpp>
#include <etcd/proto/rpc.grpc.pb.h>

namespace grpc {
class Channel;
}

namespace etcd {

class etcd {
public:
    etcd(const std::string& endpoints);

    std::vector<member> members() const;

    void put(const std::string& key, const std::string& value);

private:
    std::string endpoints_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<etcdserverpb::Cluster::Stub> cluster_stub_;
    std::unique_ptr<etcdserverpb::KV::Stub> kv_stub_;
};

}

#endif
