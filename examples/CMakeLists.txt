cmake_minimum_required(VERSION 3.16)

set(This Example)

set(Sources
    example.cpp
)

add_executable(${This} ${Sources})
target_link_libraries(${This}
    ExpiringLRUCache
)
