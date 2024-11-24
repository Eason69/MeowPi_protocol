cmake_minimum_required(VERSION 3.16.3)
project(MeowPi_protocol)

set(CMAKE_CXX_STANDARD 17)

set(T113_LIB_DIR ~/t113_libs)
set(CMAKE_FIND_ROOT_PATH ${T113_LIB_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
include_directories(${T113_LIB_DIR}/include)
link_directories(${T113_LIB_DIR}/lib)

find_package(OpenSSL REQUIRED)

add_library(cat_protocol_v1.1_udp SHARED cat_net.cpp)

target_link_libraries(cat_protocol_v1.1_udp PRIVATE ${OPENSSL_LIBRARIES})