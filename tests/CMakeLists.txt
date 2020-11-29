cmake_minimum_required(VERSION 3.16)

set(This ExpiringLRUCacheTests)

set(Sources
    expiring_lru_cache_tests.cpp
)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC
    gtest_main
    ExpiringLRUCache
)

add_test(
    NAME ${This}
    COMMAND ${This}
)
