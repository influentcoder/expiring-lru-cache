#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <gh-lru/expiring_lru_cache.hpp>

int main()
{
    using Cache = ExpiringLruCache<int, std::string>;

    size_t capacity = 2;
    unsigned int timeToLiveInSeconds = 3;

    Cache cache(capacity, timeToLiveInSeconds);

    cache.emplace(1, "a");
    cache.emplace(2, "b");

    std::cout << cache.at(1) << std::endl; // prints "a"
    std::cout << cache.at(2) << std::endl; // prints "b"

    // The find() method returns an iterator, on which the first element is the key and
    // the second element is a tuple of three elements:
    // 0. The value
    // 1. A list iterator on the keys
    // 2. A chrono time point which represents the time when the element was created or
    //    last accessed.
    std::cout << std::get<0>(cache.find(1)->second) << std::endl; // prints "a"
    std::cout << std::get<0>(cache.find(2)->second) << std::endl; // prints "b"

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // Refresh the timestamp.
    cache.at(1);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << cache.at(1) << std::endl; // prints "a"
    // prints 1 (true), as the element was evicted due to being outdated
    std::cout << (cache.find(2) == cache.end()) << std::endl;

    return 0;
}
