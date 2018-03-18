#include <grpc++/grpc++.h>
#include <etcd/etcd.hpp>
#include <etcd/proto/rpc.grpc.pb.h>

namespace etcdserverpb {

struct Stub {
    std::unique_ptr<Cluster::Stub> cluster;
    std::unique_ptr<KV::Stub> kv;
};

}

namespace etcd {

etcd::etcd(const std::string& endpoints)
    : endpoints_(endpoints),
      channel_(grpc::CreateChannel(endpoints, grpc::InsecureChannelCredentials())),
      stub_(new etcdserverpb::Stub())
{
    stub_->cluster = etcdserverpb::Cluster::NewStub(channel_);
    stub_->kv = etcdserverpb::KV::NewStub(channel_);
}

member_list_response etcd::member_list()
{
    etcdserverpb::MemberListResponse response;

    grpc::ClientContext context;

    grpc::Status status = stub_->cluster->MemberList(&context,
        etcdserverpb::MemberListRequest(), &response);

    if (!status.ok())
        throw grpc_error(status);

    return response;
}

void etcd::put(const std::string& key, const std::string& value)
{
    etcdserverpb::PutResponse response;

    grpc::ClientContext context;

    grpc::Status status = stub_->kv->Put(&context,
        (etcdserverpb::PutRequest) put_request(key, value), &response);

    if (!status.ok())
        throw grpc_error(status);
}

etcd::~etcd() = default;

}
