find_package(Protobuf REQUIRED)
find_package(Protobuf_gRPC REQUIRED)

set(RPCS etcdserverpb/rpc.proto)
set(PROTOS ${RPCS})
set(Protobuf_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOS})
protobuf_generate_grpc_cpp(GRPC_SRCS GRPC_HDRS ${RPCS})

add_library(etcd-rpc ${PROTO_SRCS} ${GRPC_SRCS})

target_include_directories(etcd-rpc PUBLIC ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(etcd-rpc PRIVATE ${Protobuf_LIBRARIES} ${gRPC_LIBRARIES})
