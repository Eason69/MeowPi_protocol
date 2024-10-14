cmake_minimum_required(VERSION 3.0)
project(MeowPi_protocol)

set(CMAKE_CXX_STANDARD 17)

add_library(MeowPi_protocol SHARED cat_net.cpp)