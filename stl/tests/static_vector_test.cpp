#include "stl/static_vector.h"

#include <gmock/gmock.h>
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

using StaticVector = rtw::stl::StaticVector<Struct>;

} // namespace

TEST(StaticVectorTest, constructor)
{
  StaticVector vector{10U};
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());
  EXPECT_TRUE(vector.begin() == vector.end());

  EXPECT_DEATH(StaticVector{0U}, ".*");
}

TEST(StaticVectorTest, push_back)
{
  StaticVector vector{10U};
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    vector.push_back(s);
    EXPECT_EQ(vector[i], s);
  }

  EXPECT_EQ(vector.size(), vector.capacity());
  EXPECT_FALSE(vector.begin() == vector.end());

  for (std::size_t i = 0U; i < vector.size(); ++i)
  {
    EXPECT_EQ(vector[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_DEATH(vector.push_back(Struct{}), ".*");

  vector.clear();

  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());
  EXPECT_TRUE(vector.begin() == vector.end());
}

TEST(StaticVectorTest, emplace_back)
{
  StaticVector vector{10U};
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    const auto& s = vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                        static_cast<std::uint8_t>(i + 1U));
    EXPECT_EQ(vector[i], s);
    EXPECT_EQ(vector[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_EQ(vector.size(), vector.capacity());

  for (std::size_t i = 0U; i < vector.size(); ++i)
  {
    EXPECT_EQ(vector[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_DEATH(vector.emplace_back(), ".*");

  vector.clear();
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());
}

TEST(StaticVectorTest, pop_back)
{
  StaticVector vector{10U};
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
  }

  const auto size = vector.size() - 1U;
  for (std::size_t i = 0U; i < size; ++i)
  {
    const std::size_t index = vector.size() - 2U;
    const auto* ptr = &vector[index];

    vector.pop_back();

    EXPECT_EQ(vector.size(), vector.capacity() - i - 1U);
    EXPECT_FALSE(vector.empty());
    EXPECT_EQ(vector[index], (Struct{static_cast<float>(index + 1U), static_cast<std::int32_t>(index + 1U),
                                     static_cast<std::uint8_t>(index + 1U)}));
    EXPECT_EQ(ptr, &vector[index]);
  }

  EXPECT_EQ(vector.size(), 1U);
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(vector[0U], (Struct{1.0F, 1, 1}));

  vector.pop_back();

  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
  EXPECT_DEATH(vector.pop_back(), ".*");
}

TEST(StaticVectorTest, iterators)
{
  StaticVector vector{10U};
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
  }

  std::size_t index = 0U;
  for (const auto& value : vector)
  {
    EXPECT_EQ(&value, &vector[index]);
    EXPECT_EQ(value, vector[index]);
    ++index;
  }

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(vector.begin(), vector.end()));

  vector.pop_back();
  vector.pop_back();
  vector.pop_back();

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(vector.begin(), vector.end()));
}

TEST(StaticVectorTest, reverse_iterator)
{
  StaticVector vector{10U};
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  std::vector<Struct> expected;
  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
    expected.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                          static_cast<std::uint8_t>(i + 1U));
  }

  std::reverse(expected.begin(), expected.end());
  std::reverse(vector.begin(), vector.end());

  EXPECT_THAT(expected, ::testing::ElementsAreArray(vector));

  std::reverse(expected.rbegin(), expected.rend());
  std::reverse(vector.rbegin(), vector.rend());

  EXPECT_THAT(expected, ::testing::ElementsAreArray(vector));
}

TEST(StaticVectorTest, resize)
{
  StaticVector vector{10U};

  vector.resize(5U);
  EXPECT_EQ(vector.size(), 5U);
  for (std::size_t i = 0U; i < 5U; ++i)
  {
    EXPECT_EQ(vector[i], (Struct{}));
  }

  // Resize down.
  vector.resize(2U);
  EXPECT_EQ(vector.size(), 2U);

  // Resize back up.
  vector.resize(4U);
  EXPECT_EQ(vector.size(), 4U);

  // Resize to zero.
  vector.resize(0U);
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
}

TEST(StaticVectorTest, insert_by_index)
{
  StaticVector vector{10U};

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});

  // Insert at the beginning.
  vector.insert(0U, Struct{0.0F, 0, 0});
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(vector[0U], (Struct{0.0F, 0, 0}));
  EXPECT_EQ(vector[1U], (Struct{1.0F, 1, 1}));
  EXPECT_EQ(vector[2U], (Struct{2.0F, 2, 2}));
  EXPECT_EQ(vector[3U], (Struct{3.0F, 3, 3}));

  // Insert in the middle.
  vector.insert(2U, Struct{1.5F, 15, 15});
  EXPECT_EQ(vector.size(), 5U);
  EXPECT_EQ(vector[2U], (Struct{1.5F, 15, 15}));
  EXPECT_EQ(vector[3U], (Struct{2.0F, 2, 2}));

  // Insert at the end.
  vector.insert(vector.size(), Struct{9.0F, 9, 9});
  EXPECT_EQ(vector.size(), 6U);
  EXPECT_EQ(vector[5U], (Struct{9.0F, 9, 9}));

  // Insert with move.
  Struct moved{7.0F, 7, 7};
  vector.insert(1U, std::move(moved));
  EXPECT_EQ(vector.size(), 7U);
  EXPECT_EQ(vector[1U], (Struct{7.0F, 7, 7}));
}

TEST(StaticVectorTest, insert_by_iterator)
{
  StaticVector vector{10U};

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});

  // Insert at the beginning via iterator.
  auto it = vector.insert(vector.cbegin(), Struct{0.0F, 0, 0});
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(*it, (Struct{0.0F, 0, 0}));
  EXPECT_EQ(vector[0U], (Struct{0.0F, 0, 0}));
  EXPECT_EQ(vector[1U], (Struct{1.0F, 1, 1}));

  // Insert in the middle via iterator.
  it = vector.insert(vector.cbegin() + 2, Struct{1.5F, 15, 15});
  EXPECT_EQ(vector.size(), 5U);
  EXPECT_EQ(*it, (Struct{1.5F, 15, 15}));
  EXPECT_EQ(vector[2U], (Struct{1.5F, 15, 15}));

  // Insert at the end via iterator.
  it = vector.insert(vector.cend(), Struct{9.0F, 9, 9});
  EXPECT_EQ(vector.size(), 6U);
  EXPECT_EQ(*it, (Struct{9.0F, 9, 9}));
  EXPECT_EQ(vector[5U], (Struct{9.0F, 9, 9}));

  // Insert with move via iterator.
  Struct moved{7.0F, 7, 7};
  it = vector.insert(vector.cbegin() + 1, std::move(moved));
  EXPECT_EQ(vector.size(), 7U);
  EXPECT_EQ(*it, (Struct{7.0F, 7, 7}));
  EXPECT_EQ(vector[1U], (Struct{7.0F, 7, 7}));
}

TEST(StaticVectorTest, erase_by_index)
{
  StaticVector vector{10U};

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});
  vector.push_back(Struct{4.0F, 4, 4});
  vector.push_back(Struct{5.0F, 5, 5});

  // Erase from the middle.
  auto it = vector.erase(2U);
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(*it, (Struct{4.0F, 4, 4}));
  EXPECT_EQ(vector[0U], (Struct{1.0F, 1, 1}));
  EXPECT_EQ(vector[1U], (Struct{2.0F, 2, 2}));
  EXPECT_EQ(vector[2U], (Struct{4.0F, 4, 4}));
  EXPECT_EQ(vector[3U], (Struct{5.0F, 5, 5}));

  // Erase from the beginning.
  it = vector.erase(0U);
  EXPECT_EQ(vector.size(), 3U);
  EXPECT_EQ(*it, (Struct{2.0F, 2, 2}));

  // Erase last element.
  it = vector.erase(vector.size() - 1U);
  EXPECT_EQ(vector.size(), 2U);
  EXPECT_EQ(it, vector.end());

  // Erase all.
  vector.erase(0U);
  vector.erase(0U);
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
}

TEST(StaticVectorTest, erase_by_iterator)
{
  StaticVector vector{10U};

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});
  vector.push_back(Struct{4.0F, 4, 4});
  vector.push_back(Struct{5.0F, 5, 5});

  // Erase from the middle via iterator.
  auto it = vector.erase(vector.cbegin() + 2);
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(*it, (Struct{4.0F, 4, 4}));

  // Erase from the beginning via iterator.
  it = vector.erase(vector.cbegin());
  EXPECT_EQ(vector.size(), 3U);
  EXPECT_EQ(*it, (Struct{2.0F, 2, 2}));

  // Erase last element via iterator.
  it = vector.erase(vector.cbegin() + 2);
  EXPECT_EQ(vector.size(), 2U);
  EXPECT_EQ(it, vector.end());

  // Erase all via iterator.
  vector.erase(vector.cbegin());
  vector.erase(vector.cbegin());
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
}

TEST(StaticVectorTest, front_back_data)
{
  StaticVector vector{10U};

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});

  EXPECT_EQ(vector.front(), (Struct{1.0F, 1, 1}));
  EXPECT_EQ(vector.back(), (Struct{3.0F, 3, 3}));

  // Modify via front/back.
  vector.front() = Struct{10.0F, 10, 10};
  vector.back() = Struct{30.0F, 30, 30};
  EXPECT_EQ(vector[0U], (Struct{10.0F, 10, 10}));
  EXPECT_EQ(vector[2U], (Struct{30.0F, 30, 30}));

  // data() points to first element.
  EXPECT_EQ(vector.data(), &vector[0U]);
}
