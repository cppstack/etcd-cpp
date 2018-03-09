#include <etcd/put_request.hpp>
#include <etcd/proto/rpc.pb.h>

namespace etcd {

put_request::operator etcdserverpb::PutRequest() const
{
    etcdserverpb::PutRequest req;
    req.set_key(key_);
    req.set_value(value_);
    req.set_lease(lease_);
    req.set_prev_kv(prev_kv_);
    req.set_ignore_value(ignore_value_);
    req.set_ignore_lease(ignore_lease_);
    return req;
}

}
