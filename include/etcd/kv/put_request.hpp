#ifndef _ETCD_PUT_REQUEST_HPP
#define _ETCD_PUT_REQUEST_HPP

#include <cstdint>
#include <string>

namespace etcdserverpb { class PutRequest; }

namespace etcd {

class put_request {
public:
    put_request(const std::string& key, const std::string& value)
        : key_(key), value_(value)
    { }

    operator etcdserverpb::PutRequest() const;

private:
    std::string key_;
    std::string value_;
    int64_t lease_ = 0;
    bool prev_kv_ = false;
    bool ignore_value_ = false;
    bool ignore_lease_ = false;
};

}

#endif
