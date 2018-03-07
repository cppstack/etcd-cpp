add_executable(etcdcli etcdcli.cpp)

target_compile_options(etcdcli PRIVATE -Wall -Wextra -Wpointer-arith)
target_link_libraries(etcdcli PRIVATE etcd)
