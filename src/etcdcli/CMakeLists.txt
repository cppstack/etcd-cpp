add_executable(etcdcli etcdcli.cpp cluster.cpp kv.cpp)

target_compile_options(etcdcli PRIVATE -Wall -Wextra -Wpointer-arith)
target_link_libraries(etcdcli PRIVATE etcd cst_program_options)

install(TARGETS etcdcli DESTINATION bin)
