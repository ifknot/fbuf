cmake_minimum_required(VERSION 3.14)
project(fbuf)

set(CMAKE_CXX_STANDARD 17)

include_directories(tests)

add_executable(fbuf
        tests/catch2.h
        LICENSE
        main.cpp
        README.md frame_buffer.cpp frame_buffer.h)
