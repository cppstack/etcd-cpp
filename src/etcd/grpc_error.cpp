#include <grpc++/grpc++.h>
#include <etcd/grpc_error.hpp>

namespace etcd {

grpc_error::grpc_error(const grpc::Status& status)
    : code_(status.error_code())
{
    what_.append("code: ")
         .append(std::to_string(code_))
         .append(", message: ")
         .append(status.error_message());
}

}
