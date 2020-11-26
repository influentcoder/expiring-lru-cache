cmake_minimum_required(VERSION 3.16)

set(This Example)

set(Sources
    Example.cpp
)

# Needed as the library provides only a ".hpp" file which is not automatically
# recognized by cmake.
set_target_properties(ExpiringLRUCache PROPERTIES LINKER_LANGUAGE CXX)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC
    ExpiringLRUCache
)
