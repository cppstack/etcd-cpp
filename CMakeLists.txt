cmake_minimum_required(VERSION 3.8)
project(etcd-cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(proto)
add_subdirectory(src/etcd)
add_subdirectory(src/etcdcli)
