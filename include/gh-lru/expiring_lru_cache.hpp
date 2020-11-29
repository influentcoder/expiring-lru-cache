#ifndef EXPIRING_LRU_CACHE_HPP
#define EXPIRING_LRU_CACHE_HPP 1

#include <list>
#include <unordered_map>
#include <chrono>

/**
 * @brief A container built on top of unordered_map and list to implement a LRU cache.
 * After a certain time, the elements of the cache expire and are lazily evicted from
 * the cache.
 * 
 * @tparam K Type of key objects.
 * @tparam V Type of mapped values.
 */
template <class K, class V>
class ExpiringLruCache
{
    using clock_t = std::chrono::steady_clock;
    using time_point_t = clock_t::time_point;

    using listk_t = typename std::list<K>;
    using list_it_t = typename listk_t::iterator;
    using tuple_t = typename std::tuple<V, list_it_t, time_point_t>;
    using mapkpairkv_t = typename std::unordered_map<K, typename std::tuple<V, list_it_t, time_point_t>>;

public:

    using iterator = typename mapkpairkv_t::iterator;
    using const_iterator = typename mapkpairkv_t::const_iterator;
    
    /**
     * Build a cache with a given capacity and TTL (time-to-live).
     * @param capacity The capacity of the cache. When the cache size reaches capicity,
     * and a new element is inserted, the least used element will be evicted from the
     * cache.
     * @param ttl Time-to-live for the elements in the cache, in seconds.
     */
    ExpiringLruCache(size_t capacity, unsigned int ttl) : m_capacity(capacity) {
        m_ttl = std::chrono::seconds(ttl);
    }

    /**
     * Get an iterator, where the first element points to a key and the second element
     * points to a tuple. The tuple is composed of three elements:
     * 0. The value associated to the key.
     * 1. The underlying list iterator.
     * 2. The time point which represents the time the element was created.
     */
    iterator find(const K &key)
    {
        using std::chrono::duration_cast;
        using std::chrono::seconds;

        const auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            if (duration_cast<seconds>(clock_t::now() - std::get<2>(it->second)).count() > m_ttl.count())
            {
                m_used.erase(std::get<1>(it->second));
                m_cache.erase(it);
                return m_cache.end();
            }

            touch(it);
        }
        return it;
    }

    /**
     * Returns a reference to the mapped value of the element with key k in the LRU
     * cache.
     * 
     * If k does not match the key of any element in the container, the function throws
     * an out_of_range exception.
     */
    V& at(const K &key)
    {
        const auto it = find(key);
        if (it == m_cache.end())
        {
            throw std::out_of_range("Out of range");
        }

        return std::get<0>(it->second);
    }

    /**
     * Returns an iterator pointing to the past-the-end element in the container.
     */
    const_iterator end() const
    {
        return m_cache.end();
    }

    /**
     * Insert a new elements in the LRU cache. Unlike the unordered_map emplace(), if
     * the key already exists, the value will be replaced with the provided value.
     */
    void emplace(const K &key, const V &value)
    {
        const auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            touch(it);
        }
        else
        {
            if (m_cache.size() == m_capacity)
            {
                m_cache.erase(m_used.back());
                m_used.pop_back();
            }
            m_used.push_front(key);
        }
        m_cache[key] = {value, m_used.begin(), clock_t::now()};
    }

    /**
     * Returns the number of elements in the LRU cache container.
     */
    size_t size()
    {
        return m_cache.size();
    }

    /**
     * Size of the underlying list, use for testing purposes only.
     */
    size_t _listSize()
    {
        return m_used.size();
    }

private:
    void touch(const iterator &it)
    {
        // We use splice instead of erase()/push_front() to avoid an unnecessary copy.
        m_used.splice(m_used.begin(), m_used, std::get<1>(it->second));
        std::get<1>(it->second) = m_used.begin();
        std::get<2>(it->second) = clock_t::now();
    }

    mapkpairkv_t m_cache;
    listk_t m_used;
    size_t m_capacity;
    std::chrono::duration<int64_t> m_ttl;
};

#endif /* EXPIRING_LRU_CACHE_HPP */