#include "stl/flat_unordered_set.h"

#include <gtest/gtest.h>

namespace
{

using InplaceFlatUnorderedSet = rtw::stl::InplaceFlatUnorderedSet<std::size_t, 10U>;

} // namespace

TEST(InplaceFlatUnorderedSet, constructor)
{
  InplaceFlatUnorderedSet set;
  EXPECT_EQ(set.size(), 0U);
  EXPECT_EQ(set.capacity(), 10U);
  EXPECT_TRUE(set.empty());
}

TEST(InplaceFlatUnorderedSet, emplace_and_operator_brackets)
{
  {
    InplaceFlatUnorderedSet set;

    EXPECT_TRUE(set.emplace(1U));
    EXPECT_EQ(set.size(), 1U);
    EXPECT_FALSE(set.empty());

    EXPECT_TRUE(set.emplace(2U));
    EXPECT_EQ(set.size(), 2U);

    EXPECT_TRUE(set.contains(1U));
    EXPECT_TRUE(set.contains(2U));

    set.clear();
    EXPECT_EQ(set.size(), 0U);
    EXPECT_TRUE(set.empty());

    EXPECT_TRUE(rtw::stl::is_memory_contiguous(set.begin(), set.end()));
  }
  {
    rtw::stl::InplaceFlatUnorderedSet<std::size_t, 2U> set;

    EXPECT_TRUE(set.emplace(1U));
    EXPECT_TRUE(set.emplace(2U));
    EXPECT_FALSE(set.emplace(3U));
    EXPECT_EQ(set.size(), 2U);
  }
}

TEST(InplaceFlatUnorderedSet, insert_and_operator_brackets)
{
  {
    InplaceFlatUnorderedSet set;

    EXPECT_TRUE(set.insert(1U));
    EXPECT_EQ(set.size(), 1U);
    EXPECT_FALSE(set.empty());

    EXPECT_TRUE(set.insert(2U));
    EXPECT_EQ(set.size(), 2U);

    EXPECT_TRUE(set.contains(1U));
    EXPECT_TRUE(set.contains(2U));

    set.clear();
    EXPECT_EQ(set.size(), 0U);
    EXPECT_TRUE(set.empty());

    EXPECT_TRUE(rtw::stl::is_memory_contiguous(set.begin(), set.end()));
  }
  {
    rtw::stl::InplaceFlatUnorderedSet<std::size_t, 2U> set;

    EXPECT_TRUE(set.insert(1U));
    EXPECT_TRUE(set.insert(2U));
    EXPECT_FALSE(set.insert(3U));
    EXPECT_EQ(set.size(), 2U);
  }
}

TEST(InplaceFlatUnorderedSet, erase)
{
  InplaceFlatUnorderedSet set;

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

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(set.begin(), set.end()));
}

TEST(InplaceFlatUnorderedSet, find)
{
  InplaceFlatUnorderedSet set;

  set.emplace(1U);
  set.emplace(2U);

  auto* it = set.find(1U);
  EXPECT_NE(it, set.end());
  EXPECT_EQ(it->get_reference(), 1U);

  it = set.find(2U);
  EXPECT_NE(it, set.end());
  EXPECT_EQ(it->get_reference(), 2U);

  it = set.find(3U);
  EXPECT_EQ(it, set.end()); // Non-existing key

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(set.begin(), set.end()));
}
