add_library(etcd
            etcd.cpp
            member.cpp
            grpc_error.cpp
)

target_compile_options(etcd PRIVATE -Wall -Wextra -Wpointer-arith)
target_include_directories(etcd PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(etcd PUBLIC etcd-rpc)
