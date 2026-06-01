#include "stl/static_flat_unordered_map.h"

#include <gtest/gtest.h>

namespace
{

struct Struct
{
  Struct() = default;
  Struct(const float a, const std::int32_t b, const std::uint8_t c) : a{a}, b{b}, c{c} {}

  float a{};
  std::int32_t b{};
  std::uint8_t c{};

  bool operator==(const Struct& other) const { return std::tie(a, b, c) == std::tie(other.a, other.b, other.c); }
};

using StaticFlatUnorderedMap = rtw::stl::StaticFlatUnorderedMap<std::size_t, Struct>;

} // namespace

TEST(StaticFlatUnorderedMap, constructor)
{
  StaticFlatUnorderedMap map{10U};
  EXPECT_EQ(map.size(), 0U);
  EXPECT_EQ(map.capacity(), 10U);
  EXPECT_TRUE(map.empty());
  EXPECT_TRUE(map.begin() == map.end());

  EXPECT_DEATH(StaticFlatUnorderedMap{0U}, ".*");
}

TEST(StaticFlatUnorderedMap, emplace_and_operator_brackets)
{
  {
    StaticFlatUnorderedMap map{10U};

    EXPECT_TRUE(map.emplace(1U, Struct{1.0F, 2, 3}));
    EXPECT_EQ(map.size(), 1U);
    EXPECT_FALSE(map.empty());

    EXPECT_TRUE(map.emplace(2U, Struct{4.0F, 5, 6}));
    EXPECT_EQ(map.size(), 2U);

    EXPECT_EQ(map[1U], (Struct{1.0F, 2, 3}));
    EXPECT_EQ(map[2U], (Struct{4.0F, 5, 6}));

    map[1U] = Struct{7.0F, 8, 9};
    EXPECT_EQ(map[1U], (Struct{7.0F, 8, 9}));

    EXPECT_TRUE(map.contains(1U));
    EXPECT_TRUE(map.contains(2U));
    EXPECT_FALSE(map.begin() == map.end());
    EXPECT_EQ(std::distance(map.begin(), map.end()), 2);

    {
      // Check number of iterations
      std::size_t iterations = 0U;
      for (const auto& [key, _] : map)
      {
        ++iterations;
        EXPECT_TRUE(key == 1U || key == 2U);
      }
      EXPECT_EQ(iterations, 2U);
    }

    map.clear();

    EXPECT_EQ(map.size(), 0U);
    EXPECT_TRUE(map.empty());
    EXPECT_TRUE(map.begin() == map.end());
    EXPECT_EQ(std::distance(map.begin(), map.end()), 0);
  }
  {
    StaticFlatUnorderedMap map{2U};

    EXPECT_TRUE(map.emplace(1U, Struct{1.0F, 2, 3}));
    EXPECT_TRUE(map.emplace(2U, Struct{4.0F, 5, 6}));
    EXPECT_TRUE(map.emplace(2U, Struct{7.0F, 8, 9}));
    EXPECT_TRUE(map[2U] == (Struct{7.0F, 8, 9}));
    EXPECT_FALSE(map.emplace(3U, Struct{7.0F, 8, 9}));
    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(StaticFlatUnorderedMap, insert_and_operator_brackets)
{
  {
    StaticFlatUnorderedMap map{10U};

    EXPECT_TRUE(map.insert({1U, Struct{1.0F, 2, 3}}));
    EXPECT_EQ(map.size(), 1U);
    EXPECT_FALSE(map.empty());

    EXPECT_TRUE(map.insert({2U, Struct{4.0F, 5, 6}}));
    EXPECT_EQ(map.size(), 2U);

    EXPECT_EQ(map[1U], (Struct{1.0F, 2, 3}));
    EXPECT_EQ(map[2U], (Struct{4.0F, 5, 6}));

    map[1U] = Struct{7.0F, 8, 9};
    EXPECT_EQ(map[1U], (Struct{7.0F, 8, 9}));

    EXPECT_TRUE(map.contains(1U));
    EXPECT_TRUE(map.contains(2U));
    EXPECT_FALSE(map.begin() == map.end());

    map.clear();

    EXPECT_EQ(map.size(), 0U);
    EXPECT_TRUE(map.empty());
    EXPECT_TRUE(map.begin() == map.end());
  }
  {
    StaticFlatUnorderedMap map{2U};

    EXPECT_TRUE(map.insert({1U, Struct{1.0F, 2, 3}}));
    EXPECT_TRUE(map.insert({2U, Struct{4.0F, 5, 6}}));
    EXPECT_TRUE(map.insert({2U, Struct{7.0F, 8, 9}}));
    EXPECT_TRUE(map[2U] == (Struct{7.0F, 8, 9}));
    EXPECT_FALSE(map.insert({3U, Struct{7.0F, 8, 9}}));
    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(StaticFlatUnorderedMap, operator_brackets)
{
  {
    StaticFlatUnorderedMap map{10U};

    map[1U] = Struct{1.0F, 2, 3};
    EXPECT_EQ(map.size(), 1U);
    EXPECT_FALSE(map.empty());

    map[2U] = Struct{4.0F, 5, 6};
    EXPECT_EQ(map.size(), 2U);

    EXPECT_EQ(map[1U], (Struct{1.0F, 2, 3}));
    EXPECT_EQ(map[2U], (Struct{4.0F, 5, 6}));

    map[1U] = Struct{7.0F, 8, 9};
    EXPECT_EQ(map[1U], (Struct{7.0F, 8, 9}));

    EXPECT_TRUE(map.contains(1U));
    EXPECT_TRUE(map.contains(2U));

    map.clear();
    EXPECT_EQ(map.size(), 0U);
    EXPECT_TRUE(map.empty());
  }
  {
    StaticFlatUnorderedMap map{2U};

    map[1U] = Struct{1.0F, 2, 3};
    map[2U] = Struct{4.0F, 5, 6};

    EXPECT_DEATH(map[3U], ".*");

    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(StaticFlatUnorderedMap, erase)
{
  StaticFlatUnorderedMap map{10U};

  map.emplace(1U, Struct{1.0F, 2, 3});
  map.emplace(2U, Struct{4.0F, 5, 6});
  EXPECT_EQ(map.size(), 2U);

  EXPECT_TRUE(map.erase(1U));
  EXPECT_EQ(map.size(), 1U);
  EXPECT_FALSE(map.contains(1U));
  EXPECT_TRUE(map.contains(2U));

  EXPECT_TRUE(map.erase(2U));
  EXPECT_EQ(map.size(), 0U);
  EXPECT_FALSE(map.contains(2U));

  EXPECT_FALSE(map.erase(3U)); // Erasing non-existing key
}

TEST(StaticFlatUnorderedMap, find)
{
  StaticFlatUnorderedMap map{10U};

  map.emplace(1U, Struct{1.0F, 2, 3});
  map.emplace(2U, Struct{4.0F, 5, 6});

  auto it = map.find(1U);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->first, 1U);
  EXPECT_EQ(it->second, (Struct{1.0F, 2, 3}));

  it = map.find(2U);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->first, 2U);
  EXPECT_EQ(it->second, (Struct{4.0F, 5, 6}));

  it->second = Struct{7.0F, 8, 9};
  EXPECT_EQ(it->second, (Struct{7.0F, 8, 9}));

  it = map.find(3U);
  EXPECT_EQ(it, map.end()); // Non-existing key
}

TEST(InplaceStaticFlatUnorderedMap, erase_and_find_with_tombstone)
{
  // Use a small capacity to increase collision likelihood with quadratic probing.
  rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, Struct, 8U> map;

  // Insert several keys that collide (hash % 8U == 0U)
  map.emplace(0U, Struct{1.0F, 1, 1});
  map.emplace(8U, Struct{2.0F, 2, 2});
  map.emplace(16U, Struct{3.0F, 3, 3});

  EXPECT_EQ(map.size(), 3U);
  EXPECT_TRUE(map.contains(0U));
  EXPECT_TRUE(map.contains(8U));
  EXPECT_TRUE(map.contains(16U));

  // Erase the first key in the probe chain - create a tombstone.
  EXPECT_TRUE(map.erase(0U));
  EXPECT_EQ(map.size(), 2U);
  EXPECT_FALSE(map.contains(0U));

  // Keys past the tombstone must still be findable.
  EXPECT_TRUE(map.contains(8U));
  EXPECT_TRUE(map.contains(16U));
  EXPECT_EQ(map[8U], (Struct{2.0F, 2, 2}));
  EXPECT_EQ(map[16U], (Struct{3.0F, 3, 3}));

  // Re-insert the erased key - should reuse the tombstone slot.
  EXPECT_TRUE(map.emplace(0U, Struct{10.0F, 10, 10}));
  EXPECT_EQ(map.size(), 3U);
  EXPECT_EQ(map[0U], (Struct{10.0F, 10, 10}));
  EXPECT_TRUE(map.contains(8U));
  EXPECT_TRUE(map.contains(16U));

  // Emplace with existing key should overwrite value.
  EXPECT_TRUE(map.emplace(8U, Struct{20.0F, 20, 20}));
  EXPECT_EQ(map.size(), 3U);
  EXPECT_EQ(map[8U], (Struct{20.0F, 20, 20}));

  // Erase middle element and verify the est is still reachable.
  EXPECT_TRUE(map.erase(8U));
  EXPECT_EQ(map.size(), 2U);
  EXPECT_TRUE(map.contains(0U));
  EXPECT_FALSE(map.contains(8U));
  EXPECT_TRUE(map.contains(16U));

  // operator[] should insert into tombstone slot.
  map[8U] = Struct{30.0F, 30, 30};
  EXPECT_EQ(map.size(), 3U);
  EXPECT_EQ(map[8U], (Struct{30.0F, 30, 30}));
}
