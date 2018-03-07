#include <grpc++/grpc++.h>
#include <etcd/etcd.hpp>
#include <etcd/grpc_error.hpp>

namespace etcd {

etcd::etcd(const std::string& endpoints)
    : endpoints_(endpoints),
      channel_(grpc::CreateChannel(endpoints, grpc::InsecureChannelCredentials())),
      cluster_stub_(etcdserverpb::Cluster::NewStub(channel_))
{ }

std::vector<member> etcd::members() const
{
    etcdserverpb::MemberListResponse response;

    grpc::ClientContext context;

    grpc::Status status = cluster_stub_->MemberList(&context,
        etcdserverpb::MemberListRequest(), &response);

    if (!status.ok())
        throw grpc_error(status);

    std::vector<member> members(response.members_size());

    std::move(response.mutable_members()->begin(),
              response.mutable_members()->end(),
              members.begin());

    return members;
}

}
