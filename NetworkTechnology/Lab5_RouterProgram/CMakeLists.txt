cmake_minimum_required(VERSION 3.23)
project(Lab5RouterProgram)

set(CMAKE_CXX_STANDARD 11)
set(INC_DIR C:\\npcap-sdk\\Include)
set(LINK_DIR C:\\npcap-sdk\\Lib)

#set(LINK_DIR C:\\npcap-sdk\\Lib\\x86)
#set(CMAKE_CXX_FLAGS -m32)


include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(Packet wpcap)

add_executable(Lab5RouterProgram main.cpp Protocol.h)
target_link_libraries(Lab5RouterProgram Packet wpcap ws2_32)

add_executable(test test.cpp)

