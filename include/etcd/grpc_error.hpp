#ifndef _ETCD_GRPC_ERROR_HPP
#define _ETCD_GRPC_ERROR_HPP

#include <exception>

namespace grpc { class Status; }

namespace etcd {

class grpc_error : public std::exception {
public:
    grpc_error(const grpc::Status&);

    int code() const noexcept
    { return code_; }

    const char* what() const noexcept override
    { return what_.c_str(); }

private:
    int code_;
    std::string what_;
};

}

#endif
