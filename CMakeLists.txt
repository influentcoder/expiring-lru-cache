cmake_minimum_required(VERSION 3.16)

set(This ExpiringLRUCache)

project(${This} C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_INSTALL_PREFIX /usr/include)

add_subdirectory(googletest EXCLUDE_FROM_ALL)

enable_testing()

set(Headers
    gh-lru/expiring_lru_cache.hpp
)

add_library(${This} INTERFACE)

target_include_directories(${This} INTERFACE include/)

add_subdirectory(examples)
add_subdirectory(tests)

install(DIRECTORY include/
        DESTINATION /)