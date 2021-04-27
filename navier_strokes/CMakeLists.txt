cmake_minimum_required(VERSION 3.20)
project(navierstokes C)

# Configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER mpicc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat -Wformat-security -Wundef -Wconversion -fno-common -pedantic")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# Headers
file(GLOB HEADER_FILES src/*.h)
# Sources
file(GLOB SOURCE_FILES src/*.c)

# Executable
add_executable(navierstokes ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(navierstokes m)