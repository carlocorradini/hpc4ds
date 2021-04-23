cmake_minimum_required(VERSION 3.20)
project(navierstrokes C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER mpicc)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# Header
file(GLOB HEADER_FILES src/*.h)
# Sources
file(GLOB SOURCE_FILES src/*.c)

# Executable
add_executable(navierstrokes ${SOURCE_FILES} ${HEADER_FILES})