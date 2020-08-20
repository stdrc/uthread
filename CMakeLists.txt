cmake_minimum_required(VERSION 3.14)
project(user-level-thread)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

add_executable(main main.c thread.c)
