cmake_minimum_required(VERSION 3.20)
project(navierstokes C)

# === Option
option(NO_OPEN_MP "Disable OpenMP" OFF)

# === Include
include(FetchContent)

# === Dependencies
# cJSON
set(cJSON_WITH_TESTS OFF)
set(cJSON_WITH_POST_BUILD_UNITTEST OFF)
FetchContent_Declare(
        cJSON
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG v1.7.14
)
FetchContent_MakeAvailable(cJSON)
include_directories("${cJSON_SOURCE_DIR}")
# argparse
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/cofyc/argparse.git
        GIT_TAG master
)
FetchContent_MakeAvailable(argparse)
include_directories("${argparse_SOURCE_DIR}")
# MPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
# Open MP
if (NOT NO_OPEN_MP)
    find_package(OpenMP REQUIRED)
endif ()

# === Configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER mpicc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat -Wformat-security -Wundef -Wconversion -fno-common -pedantic")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# Headers
file(GLOB_RECURSE HEADER_FILES src/*.h)
# Sources
file(GLOB_RECURSE SOURCE_FILES src/*.c)

# Executable
add_executable(navierstokes ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(navierstokes PRIVATE cjson argparse)
if (NOT NO_OPEN_MP)
    target_link_libraries(navierstokes PRIVATE OpenMP::OpenMP_C)
endif ()