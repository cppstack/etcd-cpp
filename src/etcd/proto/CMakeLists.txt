find_package(Protobuf REQUIRED)
find_package(Protobuf_gRPC REQUIRED)

set(RPCS rpc.proto)
set(PROTOS kv.proto ${RPCS})

set(Protobuf_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOS})
protobuf_generate_grpc_cpp(GRPC_SRCS GRPC_HDRS ${RPCS})

add_library(etcd-proto ${PROTO_SRCS} ${GRPC_SRCS})

target_include_directories(etcd-proto PUBLIC ${Protobuf_INCLUDE_DIRS})

target_link_libraries(etcd-proto PRIVATE ${Protobuf_LIBRARIES}
                                         ${gRPC_LIBRARIES})

install(TARGETS etcd-proto EXPORT etcd-targets DESTINATION lib)
