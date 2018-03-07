#ifndef _ETCD_MEMBER_HPP
#define _ETCD_MEMBER_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace etcdserverpb { class Member; }

namespace etcd {

class member {
public:
    member() = default;
    member(etcdserverpb::Member&&);

    member& operator=(etcdserverpb::Member&&);

    uint64_t id() const noexcept
    { return id_; }

    std::string name() const noexcept
    { return name_; }

    std::vector<std::string> peer_urls() const noexcept
    { return peer_urls_; }

    std::vector<std::string> client_urls() const noexcept
    { return client_urls_; }

private:
    uint64_t id_;
    std::string name_;
    std::vector<std::string> peer_urls_;
    std::vector<std::string> client_urls_;
};

}

#endif
