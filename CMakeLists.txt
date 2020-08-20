cmake_minimum_required(VERSION 3.14)
project(user-level-thread)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

include_directories(include)
include_directories(src)

add_library(uthread src/uthread.c)

add_executable(main main.c)
target_link_libraries(main uthread)
