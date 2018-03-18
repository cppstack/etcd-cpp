#ifndef _ETCD_MEMBER_LIST_RESPONSE_HPP
#define _ETCD_MEMBER_LIST_RESPONSE_HPP

#include <etcd/cluster/member.hpp>

namespace etcdserverpb { class MemberListResponse; }

namespace etcd {

class member_list_response {
public:
    member_list_response(etcdserverpb::MemberListResponse&& resp);

    const std::vector<member>& members() const noexcept
    { return members_; }

private:
    std::vector<member> members_;
};

}

#endif
