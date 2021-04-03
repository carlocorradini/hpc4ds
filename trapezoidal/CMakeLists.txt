cmake_minimum_required(VERSION 3.20)
project(trapezoidal C)

set(CMAKE_C_STANDARD 11)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)

set(SOURCE_FILES src/main.c)
add_executable(trapezoidal ${SOURCE_FILES})