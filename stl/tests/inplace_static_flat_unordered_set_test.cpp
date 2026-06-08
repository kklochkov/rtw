#include "stl/static_flat_unordered_set.h"

#include <gtest/gtest.h>

namespace
{

using InplaceStaticFlatUnorderedSet = rtw::stl::InplaceStaticFlatUnorderedSet<std::size_t, 10U>;

} // namespace

TEST(InplaceStaticFlatUnorderedSet, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceStaticFlatUnorderedSet>,
                "InplaceStaticFlatUnorderedSet should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceStaticFlatUnorderedSet>,
                "InplaceStaticFlatUnorderedSet should be trivially destructible.");

  InplaceStaticFlatUnorderedSet set;
  EXPECT_EQ(set.size(), 0U);
  EXPECT_EQ(set.capacity(), 10U);
  EXPECT_TRUE(set.empty());
  EXPECT_TRUE(set.begin() == set.end());
}

TEST(InplaceStaticFlatUnorderedSet, emplace_and_operator_brackets)
{
  {
    InplaceStaticFlatUnorderedSet set;

    EXPECT_TRUE(set.emplace(1U));
    EXPECT_EQ(set.size(), 1U);
    EXPECT_FALSE(set.empty());

    EXPECT_TRUE(set.emplace(2U));
    EXPECT_EQ(set.size(), 2U);

    EXPECT_TRUE(set.contains(1U));
    EXPECT_TRUE(set.contains(2U));
    EXPECT_FALSE(set.begin() == set.end());
    EXPECT_EQ(std::distance(set.begin(), set.end()), 2);

    {
      // Check number of iterations
      std::size_t iterations = 0U;
      for (const auto& key : set)
      {
        ++iterations;
        EXPECT_TRUE(key == 1U || key == 2U);
      }
      EXPECT_EQ(iterations, 2U);
    }

    set.clear();

    EXPECT_EQ(set.size(), 0U);
    EXPECT_TRUE(set.empty());
    EXPECT_TRUE(set.begin() == set.end());
    EXPECT_EQ(std::distance(set.begin(), set.end()), 0);
  }
  {
    rtw::stl::InplaceStaticFlatUnorderedSet<std::size_t, 2U> set;

    EXPECT_TRUE(set.emplace(1U));
    EXPECT_TRUE(set.emplace(2U));
    EXPECT_FALSE(set.emplace(3U));
    EXPECT_EQ(set.size(), 2U);
    EXPECT_FALSE(set.begin() == set.end());
  }
}

TEST(InplaceStaticFlatUnorderedSet, insert_and_operator_brackets)
{
  {
    InplaceStaticFlatUnorderedSet set;

    EXPECT_TRUE(set.insert(1U));
    EXPECT_EQ(set.size(), 1U);
    EXPECT_FALSE(set.empty());

    EXPECT_TRUE(set.insert(2U));
    EXPECT_EQ(set.size(), 2U);

    EXPECT_TRUE(set.contains(1U));
    EXPECT_TRUE(set.contains(2U));
    EXPECT_FALSE(set.begin() == set.end());

    set.clear();

    EXPECT_EQ(set.size(), 0U);
    EXPECT_TRUE(set.empty());
    EXPECT_TRUE(set.begin() == set.end());
  }
  {
    rtw::stl::InplaceStaticFlatUnorderedSet<std::size_t, 2U> set;

    EXPECT_TRUE(set.insert(1U));
    EXPECT_TRUE(set.insert(2U));
    EXPECT_FALSE(set.insert(3U));
    EXPECT_EQ(set.size(), 2U);
  }
}

TEST(InplaceStaticFlatUnorderedSet, erase)
{
  InplaceStaticFlatUnorderedSet set;

  set.emplace(1U);
  set.emplace(2U);
  EXPECT_EQ(set.size(), 2U);

  EXPECT_TRUE(set.erase(1U));
  EXPECT_EQ(set.size(), 1U);
  EXPECT_FALSE(set.contains(1U));
  EXPECT_TRUE(set.contains(2U));

  EXPECT_TRUE(set.erase(2U));
  EXPECT_EQ(set.size(), 0U);
  EXPECT_FALSE(set.contains(2U));

  EXPECT_FALSE(set.erase(3U)); // Erasing non-existing key
}

TEST(InplaceStaticFlatUnorderedSet, find)
{
  InplaceStaticFlatUnorderedSet set;

  set.emplace(1U);
  set.emplace(2U);

  auto it = set.find(1U);
  EXPECT_NE(it, set.end());
  EXPECT_EQ(*it, 1U);

  it = set.find(2U);
  EXPECT_NE(it, set.end());
  EXPECT_EQ(*it, 2U);

  it = set.find(3U);
  EXPECT_EQ(it, set.end()); // Non-existing key
}

TEST(InplaceStaticFlatUnorderedSet, tombstone_reuse)
{
  // Use a small capacity to make collision/tombstone behavior observable.
  rtw::stl::InplaceStaticFlatUnorderedSet<std::size_t, 7U> set;

  // Fill several slots.
  EXPECT_TRUE(set.emplace(1U));
  EXPECT_TRUE(set.emplace(2U));
  EXPECT_TRUE(set.emplace(3U));
  EXPECT_TRUE(set.emplace(4U));
  EXPECT_TRUE(set.emplace(5U));
  EXPECT_EQ(set.size(), 5U);

  // Erase some elements — creates tombstones.
  EXPECT_TRUE(set.erase(2U));
  EXPECT_TRUE(set.erase(4U));
  EXPECT_EQ(set.size(), 3U);
  EXPECT_FALSE(set.contains(2U));
  EXPECT_FALSE(set.contains(4U));

  // Insert new elements — should reuse tombstone slots.
  EXPECT_TRUE(set.emplace(10U));
  EXPECT_TRUE(set.emplace(20U));
  EXPECT_EQ(set.size(), 5U);
  EXPECT_TRUE(set.contains(10U));
  EXPECT_TRUE(set.contains(20U));

  // Original non-erased elements still findable.
  EXPECT_TRUE(set.contains(1U));
  EXPECT_TRUE(set.contains(3U));
  EXPECT_TRUE(set.contains(5U));

  // Fill remaining slots.
  EXPECT_TRUE(set.emplace(30U));
  EXPECT_TRUE(set.emplace(40U));
  EXPECT_EQ(set.size(), 7U);

  // Container is full — further insertions fail.
  EXPECT_FALSE(set.emplace(50U));
  EXPECT_EQ(set.size(), 7U);

  // Erase and re-insert to verify tombstone reuse still works after full-fill.
  EXPECT_TRUE(set.erase(30U));
  EXPECT_EQ(set.size(), 6U);
  EXPECT_TRUE(set.emplace(99U));
  EXPECT_EQ(set.size(), 7U);
  EXPECT_TRUE(set.contains(99U));
  EXPECT_FALSE(set.contains(30U));
}

TEST(InplaceStaticFlatUnorderedSet, find_skips_tombstones)
{
  // Verify that find correctly skips over tombstones to reach elements further in the probe chain.
  rtw::stl::InplaceStaticFlatUnorderedSet<std::size_t, 7U> set;

  // Insert elements that may collide (hash % 7 may map multiple keys to same slot).
  set.emplace(0U);
  set.emplace(7U);  // hash(7) % 7 == 0 on many implementations, same probe chain as 0
  set.emplace(14U); // hash(14) % 7 == 0, further along the chain

  // Erase the middle element.
  EXPECT_TRUE(set.erase(7U));
  EXPECT_FALSE(set.contains(7U));

  // Elements before and after the tombstone are still findable.
  EXPECT_TRUE(set.contains(0U));
  EXPECT_TRUE(set.contains(14U));
}
