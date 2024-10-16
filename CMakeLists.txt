cmake_minimum_required(VERSION 3.0)
project(MeowPi_protocol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_FIND_ROOT_PATH ~/toolchain-sunxi-musl-gcc-830/toolchain)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

find_package(OpenSSL REQUIRED)

add_library(cat_protocol_udp SHARED cat_net.cpp)

target_link_libraries(cat_protocol_udp PRIVATE ${OPENSSL_LIBRARIES})