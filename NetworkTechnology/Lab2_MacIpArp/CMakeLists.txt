cmake_minimum_required(VERSION 3.23)
project(Lab2MacIpArp)

set(CMAKE_CXX_STANDARD 14)

set(INC_DIR C:\\npcap-sdk\\Include)
set(LINK_DIR C:\\npcap-sdk\\Lib)


include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(Packet wpcap)

add_executable(Lab2MacIpArp main.cpp Protocol.h)
target_link_libraries(Lab2MacIpArp Packet wpcap ws2_32)