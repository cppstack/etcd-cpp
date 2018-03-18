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

    const std::string& name() const noexcept
    { return name_; }

    const std::vector<std::string>& peerurls() const noexcept
    { return peerurls_; }

    const std::vector<std::string>& clienturls() const noexcept
    { return clienturls_; }

private:
    uint64_t id_;
    std::string name_;
    std::vector<std::string> peerurls_;
    std::vector<std::string> clienturls_;
};

}

#endif
