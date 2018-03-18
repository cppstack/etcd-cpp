#include <etcd/cluster/member_list_response.hpp>
#include <etcd/proto/rpc.pb.h>

namespace etcd {

member_list_response::member_list_response(etcdserverpb::MemberListResponse&& resp)
    : members_(resp.members_size())
{
    std::move(resp.mutable_members()->begin(),
              resp.mutable_members()->end(),
              members_.begin());
}

}
