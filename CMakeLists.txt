cmake_minimum_required(VERSION 3.16)
project(unp_labs CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(include)

# common 库
add_library(common common/common.cpp)

# tcp_server
add_executable(tcp_server src/00_bootstrap/server.cpp)
target_link_libraries(tcp_server PRIVATE common)

# tcp_client
add_executable(tcp_client src/00_bootstrap/client.cpp)
target_link_libraries(tcp_client PRIVATE common)

# Windows: 需要链接 Winsock 库 Ws2_32
if (WIN32)
  target_link_libraries(tcp_client PRIVATE Ws2_32)
  target_link_libraries(tcp_server PRIVATE Ws2_32)
endif()
