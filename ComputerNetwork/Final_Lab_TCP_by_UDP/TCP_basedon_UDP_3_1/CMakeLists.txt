cmake_minimum_required(VERSION 3.23)
project(TCP_based_UDP_3_1)

set(CMAKE_CXX_STANDARD 14)

add_executable(Server Server.cpp Support.h  Support.cpp )
target_link_libraries(Server ws2_32)

add_executable(Client Client.cpp Support.h Support.cpp FileUtil.cpp FileUtil.h )
target_link_libraries(Client ws2_32)
