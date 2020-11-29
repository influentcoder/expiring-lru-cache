[![Build](https://github.com/guillaume-humbert/expiring-lru-cache/workflows/CMake/badge.svg)](https://github.com/guillaume-humbert/expiring-lru-cache/actions)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![GitHub version](https://badge.fury.io/gh/guillaume-humbert%2Fexpiring-lru-cache.svg)](https://badge.fury.io/gh/guillaume-humbert%2Fexpiring-lru-cache)
![Active](http://img.shields.io/badge/Status-Active-green.svg)

# High Performance LRU Cache With Expiring Elements

This is very similar to a traditional LRU cache, with the additional feature that
elements expire after a certain determined time.

If we try to get an element which has passed its time-to-live (TTL), it won't be
considered found and will be evicted from the cache.

# Sample Usage

See the examples folder:

```cpp
#include <iostream>
#include <string>
#include <unistd.h>
#include "../ExpiringLRUCache.hpp"

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

    sleep(2);
    // Refresh the timestamp.
    cache.at(1);

    sleep(2);
    std::cout << cache.at(1) << std::endl; // prints "a"
    // prints 1 (true), as the element was evicted due to being outdated
    std::cout << (cache.find(2) == cache.end()) << std::endl;

    std::unordered_map<int, int> map;
    map.emplace(1, 10);
    map.emplace(1, 11);
    std::cout << map.at(1) << std::endl;

    return 0;
}
```

# Build the example

```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . --target Example
```

To run it:

```bash
$ ./examples/Example
```

# Build the tests

```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . --target ExpiringLRUCacheTests
```

Run the tests:

```
$ ctest -T test --output-on-failure
```