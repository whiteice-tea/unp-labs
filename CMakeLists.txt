cmake_minimum_required(VERSION 3.16)
project(unp_labs CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(common common/common.cpp)

add_executable(tcp_server src/00_bootstrap/server.cpp)
target_link_libraries(tcp_server common)

add_executable(tcp_client src/00_bootstrap/client.cpp)
target_link_libraries(tcp_client common)
