#include "stl/flat_unordered_map.h"

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

using InplaceFlatUnorderedMap = rtw::stl::InplaceFlatUnorderedMap<std::size_t, Struct, 10U>;

} // namespace

TEST(InplaceFlatUnorderedMap, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceFlatUnorderedMap>,
                "InplaceFlatUnorderedMap should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceFlatUnorderedMap>,
                "InplaceFlatUnorderedMap should be trivially destructible.");

  InplaceFlatUnorderedMap map;
  EXPECT_EQ(map.size(), 0U);
  EXPECT_EQ(map.capacity(), 10U);
  EXPECT_TRUE(map.empty());
}

TEST(InplaceFlatUnorderedMap, emplace_and_operator_brackets)
{
  {
    InplaceFlatUnorderedMap map;

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

    map.clear();
    EXPECT_EQ(map.size(), 0U);
    EXPECT_TRUE(map.empty());
  }
  {
    rtw::stl::InplaceFlatUnorderedMap<std::size_t, Struct, 2U> map;

    EXPECT_TRUE(map.emplace(1U, Struct{1.0F, 2, 3}));
    EXPECT_TRUE(map.emplace(2U, Struct{4.0F, 5, 6}));
    EXPECT_TRUE(map.emplace(2U, Struct{7.0F, 8, 9}));
    EXPECT_TRUE(map[2U] == (Struct{7.0F, 8, 9}));
    EXPECT_FALSE(map.emplace(3U, Struct{7.0F, 8, 9}));
    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(InplaceFlatUnorderedMap, insert_and_operator_brackets)
{
  {
    InplaceFlatUnorderedMap map;

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

    map.clear();
    EXPECT_EQ(map.size(), 0U);
    EXPECT_TRUE(map.empty());
  }
  {
    rtw::stl::InplaceFlatUnorderedMap<std::size_t, Struct, 2U> map;

    EXPECT_TRUE(map.insert({1U, Struct{1.0F, 2, 3}}));
    EXPECT_TRUE(map.insert({2U, Struct{4.0F, 5, 6}}));
    EXPECT_TRUE(map.insert({2U, Struct{7.0F, 8, 9}}));
    EXPECT_TRUE(map[2U] == (Struct{7.0F, 8, 9}));
    EXPECT_FALSE(map.insert({3U, Struct{7.0F, 8, 9}}));
    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(InplaceFlatUnorderedMap, operator_brackets)
{
  {
    InplaceFlatUnorderedMap map;

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
    rtw::stl::InplaceFlatUnorderedMap<std::size_t, Struct, 2U> map;

    map[1U] = Struct{1.0F, 2, 3};
    map[2U] = Struct{4.0F, 5, 6};

    EXPECT_DEATH(map[3U], ".*");

    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(InplaceFlatUnorderedMap, erase)
{
  InplaceFlatUnorderedMap map;

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

TEST(InplaceFlatUnorderedMap, find)
{
  InplaceFlatUnorderedMap map;

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
