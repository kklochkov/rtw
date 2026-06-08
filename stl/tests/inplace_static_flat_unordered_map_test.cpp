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

using InplaceStaticFlatUnorderedMap = rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, Struct, 10U>;

} // namespace

TEST(InplaceStaticFlatUnorderedMap, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceStaticFlatUnorderedMap>,
                "InplaceStaticFlatUnorderedMap should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceStaticFlatUnorderedMap>,
                "InplaceStaticFlatUnorderedMap should be trivially destructible.");

  InplaceStaticFlatUnorderedMap map;
  EXPECT_EQ(map.size(), 0U);
  EXPECT_EQ(map.capacity(), 10U);
  EXPECT_TRUE(map.empty());
  EXPECT_TRUE(map.begin() == map.end());
}

TEST(InplaceStaticFlatUnorderedMap, emplace_and_operator_brackets)
{
  {
    InplaceStaticFlatUnorderedMap map;

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
    rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, Struct, 2U> map;

    EXPECT_TRUE(map.emplace(1U, Struct{1.0F, 2, 3}));
    EXPECT_TRUE(map.emplace(2U, Struct{4.0F, 5, 6}));
    EXPECT_TRUE(map.emplace(2U, Struct{7.0F, 8, 9}));
    EXPECT_TRUE(map[2U] == (Struct{7.0F, 8, 9}));
    EXPECT_FALSE(map.emplace(3U, Struct{7.0F, 8, 9}));
    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(InplaceStaticFlatUnorderedMap, insert_and_operator_brackets)
{
  {
    InplaceStaticFlatUnorderedMap map;

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
    rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, Struct, 2U> map;

    EXPECT_TRUE(map.insert({1U, Struct{1.0F, 2, 3}}));
    EXPECT_TRUE(map.insert({2U, Struct{4.0F, 5, 6}}));
    EXPECT_TRUE(map.insert({2U, Struct{7.0F, 8, 9}}));
    EXPECT_TRUE(map[2U] == (Struct{7.0F, 8, 9}));
    EXPECT_FALSE(map.insert({3U, Struct{7.0F, 8, 9}}));
    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(InplaceStaticFlatUnorderedMap, operator_brackets)
{
  {
    InplaceStaticFlatUnorderedMap map;

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
    rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, Struct, 2U> map;

    map[1U] = Struct{1.0F, 2, 3};
    map[2U] = Struct{4.0F, 5, 6};

    EXPECT_DEATH(map[3U], ".*");

    EXPECT_EQ(map.size(), 2U);
  }
}

TEST(InplaceStaticFlatUnorderedMap, erase)
{
  InplaceStaticFlatUnorderedMap map;

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

TEST(InplaceStaticFlatUnorderedMap, find)
{
  InplaceStaticFlatUnorderedMap map;

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

TEST(InplaceStaticFlatUnorderedMap, tombstone_reuse)
{
  // Use a small capacity to increase collision probability and test tombstone traversal.
  rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, int, 7U> map;

  // Fill the map partially.
  map.emplace(1U, 10);
  map.emplace(2U, 20);
  map.emplace(3U, 30);
  map.emplace(4U, 40);
  EXPECT_EQ(map.size(), 4U);

  // Erase middle keys (creates tombstones).
  EXPECT_TRUE(map.erase(2U));
  EXPECT_TRUE(map.erase(3U));
  EXPECT_EQ(map.size(), 2U);

  // Verify erased keys are not found.
  EXPECT_FALSE(map.contains(2U));
  EXPECT_FALSE(map.contains(3U));

  // Verify remaining keys are still found (find must skip tombstones).
  EXPECT_TRUE(map.contains(1U));
  EXPECT_TRUE(map.contains(4U));
  EXPECT_EQ(map[1U], 10);
  EXPECT_EQ(map[4U], 40);

  // Insert new keys — should reuse tombstone slots.
  map.emplace(5U, 50);
  map.emplace(6U, 60);
  EXPECT_EQ(map.size(), 4U);

  // Verify all current keys are accessible.
  EXPECT_EQ(map[1U], 10);
  EXPECT_EQ(map[4U], 40);
  EXPECT_EQ(map[5U], 50);
  EXPECT_EQ(map[6U], 60);

  // Erase all, then refill — ensures tombstones are cleared on reuse.
  EXPECT_TRUE(map.erase(1U));
  EXPECT_TRUE(map.erase(4U));
  EXPECT_TRUE(map.erase(5U));
  EXPECT_TRUE(map.erase(6U));
  EXPECT_EQ(map.size(), 0U);
  EXPECT_TRUE(map.empty());

  // Refill after all-erased state.
  map.emplace(10U, 100);
  map.emplace(20U, 200);
  EXPECT_EQ(map.size(), 2U);
  EXPECT_EQ(map[10U], 100);
  EXPECT_EQ(map[20U], 200);
}

TEST(InplaceStaticFlatUnorderedMap, find_skips_tombstones)
{
  // Keys that are likely to collide in a capacity-7 map (same hash % 7).
  // We insert keys, erase one that's probed through, then verify the later one is still findable.
  rtw::stl::InplaceStaticFlatUnorderedMap<std::size_t, int, 7U> map;

  // Insert keys 0, 7, 14 — all hash to index 0 in capacity-7 (assuming identity hash for size_t).
  map.emplace(0U, 0);
  map.emplace(7U, 7);
  map.emplace(14U, 14);
  EXPECT_EQ(map.size(), 3U);

  // Erase the middle one (key 7) — creates a tombstone in the probe chain.
  EXPECT_TRUE(map.erase(7U));
  EXPECT_EQ(map.size(), 2U);

  // Find must still locate key 14 by probing past the tombstone.
  EXPECT_TRUE(map.contains(14U));
  EXPECT_EQ(map[14U], 14);
  EXPECT_TRUE(map.contains(0U));
  EXPECT_EQ(map[0U], 0);

  // Re-insert key 7 — should reuse the tombstone slot.
  map.emplace(7U, 77);
  EXPECT_EQ(map.size(), 3U);
  EXPECT_EQ(map[7U], 77);

  // full() check
  map.emplace(1U, 1);
  map.emplace(2U, 2);
  map.emplace(3U, 3);
  map.emplace(4U, 4);
  EXPECT_TRUE(map.full());
}
