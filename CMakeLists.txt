cmake_minimum_required(VERSION 3.10)
project(MemAlloc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(mem_alloc
    src/main.c
)
