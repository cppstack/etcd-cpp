add_executable(etcdcli etcdcli.cpp)

target_compile_options(etcdcli PRIVATE -Wall -Wextra -Wpointer-arith)
target_include_directories(etcdcli PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(etcdcli PRIVATE etcd)

install(TARGETS etcdcli DESTINATION bin)
