#include <etcd/member.hpp>
#include <etcd/proto/rpc.pb.h>

namespace etcd {

member::member(etcdserverpb::Member&& m)
    : id_(m.id()),
      name_(std::move(*m.mutable_name())),
      peer_urls_(m.peerurls_size()),
      client_urls_(m.clienturls_size())
{
    std::move(m.mutable_peerurls()->begin(),
              m.mutable_peerurls()->end(),
              peer_urls_.begin());

    std::move(m.mutable_clienturls()->begin(),
              m.mutable_clienturls()->end(),
              client_urls_.begin());
}

member& member::operator=(etcdserverpb::Member&& m)
{
    id_ = m.id();
    name_ = std::move(*m.mutable_name());

    peer_urls_.resize(m.peerurls_size());
    std::move(m.mutable_peerurls()->begin(),
              m.mutable_peerurls()->end(),
              peer_urls_.begin());

    client_urls_.resize(m.clienturls_size());
    std::move(m.mutable_clienturls()->begin(),
              m.mutable_clienturls()->end(),
              client_urls_.begin());

    return *this;
}

}
