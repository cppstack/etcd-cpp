add_subdirectory(proto)

add_library(etcd
            etcd.cpp
            cluster/member.cpp
            cluster/member_list_response.cpp
            kv/put_request.cpp
            grpc_error.cpp
)

target_compile_options(etcd PRIVATE -Wall -Wextra -Wpointer-arith)

target_include_directories(etcd PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(etcd PUBLIC etcd-proto)

install(TARGETS etcd EXPORT etcd-targets DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/etcd/ DESTINATION include/etcd)
install(DIRECTORY ${PROJECT_BINARY_DIR}/src/etcd/proto/
        DESTINATION include/etcd/proto
        FILES_MATCHING PATTERN "*.pb.h" PATTERN "CMakeFiles" EXCLUDE)
install(EXPORT etcd-targets FILE etcd-config.cmake DESTINATION lib/cmake/etcd)
