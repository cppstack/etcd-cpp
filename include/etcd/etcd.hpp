#ifndef _ETCD_ETCD_HPP
#define _ETCD_ETCD_HPP

#include <memory>
#include <etcd/cluster/member_list_response.hpp>
#include <etcd/kv/put_request.hpp>
#include <etcd/grpc_error.hpp>

namespace grpc { class Channel; }

namespace etcdserverpb { struct Stub; }

namespace etcd {

class etcd {
public:
    explicit etcd(const std::string& endpoints);

    member_list_response member_list();

    void put(const std::string& key, const std::string& value);

    ~etcd();

private:
    std::string endpoints_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<etcdserverpb::Stub> stub_;
};

}

#endif
