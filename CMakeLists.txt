cmake_minimum_required(VERSION 3.8)
project(etcd-cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS "Checking out dependent libraries...")
execute_process(COMMAND git submodule update --init --recursive
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_subdirectory(src/etcd)
add_subdirectory(src/cst/program_options)
add_subdirectory(src/etcdcli)
