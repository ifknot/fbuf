cmake_minimum_required(VERSION 3.14)
project(fbuf)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "--debug -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

include_directories(tests)

add_executable(fbuf
        tests/catch2.h
        LICENSE
        main.cpp
        README.md frame_buffer.cpp frame_buffer.h tests/test_001_fbuf.cpp)
