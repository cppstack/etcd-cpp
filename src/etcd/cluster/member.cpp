#include <etcd/cluster/member.hpp>
#include <etcd/proto/rpc.pb.h>

namespace etcd {

member::member(etcdserverpb::Member&& m)
    : id_(m.id()),
      name_(std::move(*m.mutable_name())),
      peerurls_(m.peerurls_size()),
      clienturls_(m.clienturls_size())
{
    std::move(m.mutable_peerurls()->begin(),
              m.mutable_peerurls()->end(),
              peerurls_.begin());

    std::move(m.mutable_clienturls()->begin(),
              m.mutable_clienturls()->end(),
              clienturls_.begin());
}

member& member::operator=(etcdserverpb::Member&& m)
{
    id_ = m.id();
    name_ = std::move(*m.mutable_name());

    peerurls_.resize(m.peerurls_size());
    std::move(m.mutable_peerurls()->begin(),
              m.mutable_peerurls()->end(),
              peerurls_.begin());

    clienturls_.resize(m.clienturls_size());
    std::move(m.mutable_clienturls()->begin(),
              m.mutable_clienturls()->end(),
              clienturls_.begin());

    return *this;
}

}
