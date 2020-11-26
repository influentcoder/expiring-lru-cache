cmake_minimum_required(VERSION 3.16)

set(This ExpiringLRUCache)

project(${This} C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(googletest)

enable_testing()

set(Headers
    ExpiringLRUCache.hpp
)

add_library(${This} STATIC ${Headers})

add_subdirectory(examples)
add_subdirectory(tests)