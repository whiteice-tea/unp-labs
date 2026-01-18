cmake_minimum_required(VERSION 3.16)
project(unp_labs CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(common common/common.cpp)

# 针对 Windows 的客户端和服务端
if (WIN32)
    add_executable(tcp_client src/00_bootstrap/tcp_client_win.cpp)
    add_executable(tcp_server src/00_bootstrap/tcp_server_win.cpp)
    add_executable(test_learn src/00_bootstrap/test_learn.cpp) 
    

 
    target_link_libraries(tcp_client common Ws2_32)
    target_link_libraries(tcp_server common Ws2_32)
    target_link_libraries(test_learn common Ws2_32)

else()
    add_executable(tcp_client src/00_bootstrap/tcp_client.cpp)
    add_executable(tcp_server src/00_bootstrap/tcp_server.cpp)
    add_executable(test_learn src/00_bootstrap/test_learn.cpp)
    target_link_libraries(tcp_client common)
    target_link_libraries(tcp_server common)
    target_link_libraries(test_learn common)  
endif()
