cmake_minimum_required(VERSION 3.14)
project(uthread)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

include_directories(include)
include_directories(src)

add_library(uthread src/uthread.c)

add_executable(uthread_demo demo.c)
target_link_libraries(uthread_demo uthread)
