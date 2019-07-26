cmake_minimum_required(VERSION 3.14)
project(fbuf)

set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "--debug -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

include_directories(.)
include_directories(dev)

add_executable(fbuf
        dev/device.cpp
        dev/device.h
        canvas_factory.h
        main.cpp)
