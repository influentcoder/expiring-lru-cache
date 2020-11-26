#include "../ExpiringLRUCache.hpp"
#include <boost/functional/hash.hpp>
#include <gtest/gtest.h>

size_t actualKeycopies = 0;
size_t expectedKeyCopies = 0;

size_t actualValueCopies = 0;
size_t expectedValueCopies = 0;

size_t actualValueEmpty = 0;
size_t expectedValueEmpty = 0;

class MyKey
{
public:
    MyKey(int aInt, std::string aString) : m_int(aInt), m_string(aString) {}

    MyKey(const MyKey &other)
    {
        m_int = other.m_int;
        m_string = other.m_string;
        ++actualKeycopies;
    }

    bool operator==(const MyKey &rhs) const
    {
        return m_int == rhs.m_int && m_string == rhs.m_string;
    };

private:
    int m_int;
    std::string m_string;

    friend std::hash<MyKey>;
};

namespace std
{
    template <>
    struct hash<MyKey>
    {
        std::size_t operator()(const MyKey &k) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, boost::hash_value(k.m_int));
            boost::hash_combine(seed, boost::hash_value(k.m_string));
            return seed;
        }
    };
} // namespace std

class MyVal
{

public:
    MyVal()
    {
        ++actualValueEmpty;
    }
    MyVal(int aInt) : m_int(aInt) {}
    MyVal(const MyVal &other)
    {
        ++actualValueCopies;
        m_int = other.m_int;
    }

    bool operator==(const MyVal &rhs) const
    {
        return m_int == rhs.m_int;
    }

private:
    int m_int;
};

struct ExpiringLRUCacheTests
    : public ::testing::Test
{

    ExpiringLRUCacheTests() : m_cache(3, 1) {}

    virtual void SetUp() override
    {
        // Use the -V flag to display the standard output with ctest.
        std::cout << "SETUP" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "TEAR DOWN" << std::endl;
    }

    void emplaceNew(const MyKey &key, const MyVal &val)
    {
        m_cache.emplace(key, val);

        // The key is copied twice: once in the unordered_map, once in the list.
        expectedKeyCopies += 2;
        ++expectedValueCopies;
        ++expectedValueEmpty;
    }

    void emplaceExisting(const MyKey &key, const MyVal &val)
    {
        m_cache.emplace(key, val);
        ++expectedValueCopies;
    }

    void assertSize(size_t expectedSize)
    {
        ASSERT_EQ(m_cache.size(), expectedSize);
        ASSERT_EQ(m_cache._listSize(), expectedSize);
    }

    void assertGetFromCache(const MyKey &k, const MyVal &expectedVal)
    {
        ASSERT_EQ(std::get<0>(m_cache.find(k)->second), expectedVal);
        ASSERT_EQ(m_cache.at(k), expectedVal);
    }

    ExpiringLruCache<MyKey, MyVal> m_cache;
};

TEST_F(ExpiringLRUCacheTests, TypicalTest)
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::seconds;
    using std::chrono::steady_clock;

    assertSize(0);

    emplaceNew(MyKey(1, "yo"), 1);
    emplaceNew(MyKey(2, "yo"), 2);
    emplaceNew(MyKey(3, "yo"), 3);
    emplaceExisting(MyKey(3, "yo"), 3);

    assertGetFromCache(MyKey(1, "yo"), MyVal(1));
    assertGetFromCache(MyKey(2, "yo"), MyVal(2));
    assertGetFromCache(MyKey(3, "yo"), MyVal(3));

    emplaceExisting(MyKey(3, "yo"), 33);
    assertGetFromCache(MyKey(3, "yo"), MyVal(33));

    ASSERT_EQ(m_cache.find(MyKey(10, "yo")), m_cache.end());

    emplaceNew(MyKey(4, "yo"), 4);
    assertGetFromCache(MyKey(2, "yo"), MyVal(2));
    assertGetFromCache(MyKey(3, "yo"), MyVal(33));
    assertGetFromCache(MyKey(4, "yo"), MyVal(4));

    // Element 1 was evicted from the cache.
    ASSERT_EQ(m_cache.find(MyKey(1, "yo")), m_cache.end());

    // Order is now 2 -> 4 -> 3 ->
    m_cache.find(MyKey(2, "yo"));
    // Element 3 is now out.
    emplaceNew(MyKey(5, "yo"), 5);
    assertGetFromCache(MyKey(5, "yo"), MyVal(5));
    assertGetFromCache(MyKey(4, "yo"), MyVal(4));
    assertGetFromCache(MyKey(2, "yo"), MyVal(2));
    assertSize(3);

    std::get<2>(m_cache.find(MyKey(5, "yo"))->second) -= seconds(2);
    std::get<2>(m_cache.find(MyKey(4, "yo"))->second) -= seconds(2);
    std::get<2>(m_cache.find(MyKey(2, "yo"))->second) -= seconds(2);

    // We are doing lazy eviction - elements will be evicted when we try to access them.
    assertSize(3);
    ASSERT_EQ(m_cache.find(MyKey(5, "yo")), m_cache.end());
    ASSERT_EQ(m_cache.find(MyKey(4, "yo")), m_cache.end());
    ASSERT_EQ(m_cache.find(MyKey(2, "yo")), m_cache.end());
    assertSize(0);

    emplaceNew(MyKey(1, "yo"), 1);
    emplaceNew(MyKey(2, "yo"), 2);
    emplaceNew(MyKey(3, "yo"), 3);

    std::get<2>(m_cache.find(MyKey(1, "yo"))->second) -= seconds(2);
    std::get<2>(m_cache.find(MyKey(2, "yo"))->second) -= milliseconds(100);
    std::get<2>(m_cache.find(MyKey(3, "yo"))->second) -= milliseconds(100);

    assertGetFromCache(MyKey(2, "yo"), MyVal(2));
    assertGetFromCache(MyKey(3, "yo"), MyVal(3));
    ASSERT_EQ(m_cache.find(MyKey(1, "yo")), m_cache.end());
    assertSize(2);

    std::get<2>(m_cache.find(MyKey(2, "yo"))->second) -= milliseconds(500);
    // Finding an element should reset its timestamp.
    ASSERT_LT(duration_cast<milliseconds>(
                  steady_clock::now() - std::get<2>(m_cache.find(MyKey(2, "yo"))->second))
                  .count(),
              100);

    ASSERT_EQ(expectedKeyCopies, actualKeycopies);
    ASSERT_EQ(expectedValueCopies, actualValueCopies);
    ASSERT_EQ(expectedValueEmpty, actualValueEmpty);
}