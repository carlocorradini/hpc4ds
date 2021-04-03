cmake_minimum_required(VERSION 3.20)
project(pingpong C)

set(CMAKE_C_STANDARD 11)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)

set(SOURCE_FILES src/main.c)
add_executable(pingpong ${SOURCE_FILES})
target_link_libraries(pingpong m)