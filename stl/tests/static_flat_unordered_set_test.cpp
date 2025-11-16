#include "stl/static_flat_unordered_set.h"

#include <gtest/gtest.h>

namespace
{

using StaticFlatUnorderedSet = rtw::stl::StaticFlatUnorderedSet<std::size_t>;

} // namespace

TEST(StaticFlatUnorderedSet, constructor)
{
  StaticFlatUnorderedSet set{10U};
  EXPECT_EQ(set.size(), 0U);
  EXPECT_EQ(set.capacity(), 10U);
  EXPECT_TRUE(set.empty());
  EXPECT_TRUE(set.begin() == set.end());

  EXPECT_DEATH(StaticFlatUnorderedSet{0U}, ".*");
}

TEST(StaticFlatUnorderedSet, emplace_and_operator_brackets)
{
  {
    StaticFlatUnorderedSet set{10U};

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
    StaticFlatUnorderedSet set{2U};

    EXPECT_TRUE(set.emplace(1U));
    EXPECT_TRUE(set.emplace(2U));
    EXPECT_FALSE(set.emplace(3U));
    EXPECT_EQ(set.size(), 2U);
    EXPECT_FALSE(set.begin() == set.end());
  }
}

TEST(StaticFlatUnorderedSet, insert_and_operator_brackets)
{
  {
    StaticFlatUnorderedSet set{10U};

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
    StaticFlatUnorderedSet set{2U};

    EXPECT_TRUE(set.insert(1U));
    EXPECT_TRUE(set.insert(2U));
    EXPECT_FALSE(set.insert(3U));
    EXPECT_EQ(set.size(), 2U);
  }
}

TEST(StaticFlatUnorderedSet, erase)
{
  StaticFlatUnorderedSet set{10U};

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

TEST(StaticFlatUnorderedSet, find)
{
  StaticFlatUnorderedSet set{10U};

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
