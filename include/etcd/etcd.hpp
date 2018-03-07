#ifndef _ETCD_ETCD_HPP
#define _ETCD_ETCD_HPP

#include <memory>
#include <etcd/member.hpp>

#include "rpc.grpc.pb.h"

namespace grpc {
class Channel;
}

namespace etcd {

class etcd {
public:
    etcd(const std::string& endpoints);

    std::vector<member> members() const;

private:
    std::string endpoints_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<etcdserverpb::Cluster::Stub> cluster_stub_;
};

}

#endif
