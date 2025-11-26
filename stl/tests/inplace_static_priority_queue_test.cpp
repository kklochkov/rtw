#include "stl/static_priority_queue.h"

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
  bool operator<(const Struct& other) const { return std::tie(a, b, c) < std::tie(other.a, other.b, other.c); }
  bool operator>(const Struct& other) const { return std::tie(a, b, c) > std::tie(other.a, other.b, other.c); }
};

using InplaceStaticPriorityQueue = rtw::stl::InplaceStaticPriorityQueue<Struct, 10U>;

} // namespace

TEST(InplaceStaticPriorityQueueTest, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceStaticPriorityQueue>,
                "InplaceStaticPriorityQueue should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceStaticPriorityQueue>,
                "InplaceStaticPriorityQueue should be trivially destructible.");

  InplaceStaticPriorityQueue queue;
  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  EXPECT_DEATH(queue.top(), ".*");
  EXPECT_DEATH(queue.pop(), ".*");
}

TEST(InplaceStaticPriorityQueueTest, push_pop)
{
  InplaceStaticPriorityQueue queue;
  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  for (std::size_t i = 0U; i < queue.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    queue.push(s);
  }

  EXPECT_EQ(queue.size(), queue.capacity());
  EXPECT_EQ(queue.top(), (Struct{10.0F, 10, 10U}));

  EXPECT_DEATH(queue.push(Struct{11.0F, 11, static_cast<uint8_t>(11)}), ".*");

  std::size_t i = queue.size();
  while (!queue.empty())
  {
    Struct s;
    queue.pop(s);

    EXPECT_EQ(s, (Struct{static_cast<float>(i), static_cast<std::int32_t>(i), static_cast<std::uint8_t>(i)}));

    --i;
  }

  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  EXPECT_DEATH(queue.pop(), ".*");
}

TEST(InplaceStaticPriorityQueueTest, emplace_pop)
{
  InplaceStaticPriorityQueue queue;
  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  for (std::size_t i = 0U; i < queue.capacity(); ++i)
  {
    queue.emplace(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U));
  }

  EXPECT_EQ(queue.size(), queue.capacity());
  EXPECT_EQ(queue.top(), (Struct{10.0F, 10, 10U}));

  EXPECT_DEATH(queue.emplace(11.0F, 11, static_cast<uint8_t>(11)), ".*");

  std::size_t i = queue.size();
  while (!queue.empty())
  {
    Struct s;
    queue.pop(s);

    EXPECT_EQ(s, (Struct{static_cast<float>(i), static_cast<std::int32_t>(i), static_cast<std::uint8_t>(i)}));

    --i;
  }

  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  EXPECT_DEATH(queue.pop(), ".*");
}

TEST(InplaceStaticPriorityQueueTest, max_queue_additional_tests)
{
  rtw::stl::InplaceStaticPriorityQueue<std::int32_t, 10U> queue;
  {
    queue.push(42);
    EXPECT_EQ(queue.top(), 42);
  }

  queue.clear();

  {
    for (const auto v : {1, 2, 3, 4, 5})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 5);
  }

  queue.clear();

  {
    for (const auto v : {5, 4, 3, 2, 1})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 5);
  }

  queue.clear();

  {
    for (const auto v : {7, 3, 9, 1, 5, 2, 8, 4, 6})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 9);

    std::vector<std::int32_t> result;
    while (!queue.empty())
    {
      auto& value = result.emplace_back();
      queue.pop(value);
    }

    EXPECT_THAT(result, testing::ElementsAreArray({9, 8, 7, 6, 5, 4, 3, 2, 1}));
  }

  queue.clear();

  {
    for (const auto v : {3, 3, 3, 7, 7, 7, 7})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 7);

    std::vector<std::int32_t> result;
    while (!queue.empty())
    {
      auto& value = result.emplace_back();
      queue.pop(value);
    }

    EXPECT_THAT(result, testing::ElementsAreArray({7, 7, 7, 7, 3, 3, 3}));
  }

  queue.clear();

  {
    for (const auto v : {7, 7, 7, 7, 7, 7, 7})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 7);

    std::vector<std::int32_t> result;
    while (!queue.empty())
    {
      auto& value = result.emplace_back();
      queue.pop(value);
    }

    EXPECT_THAT(result, testing::ElementsAreArray({7, 7, 7, 7, 7, 7, 7}));
  }
}

TEST(InplaceStaticPriorityQueueTest, min_queue_additional_tests)
{
  rtw::stl::InplaceStaticPriorityQueue<std::int32_t, 10U, std::greater<std::int32_t>> queue;
  {
    queue.push(42);
    EXPECT_EQ(queue.top(), 42);
  }

  queue.clear();

  {
    for (const auto v : {1, 2, 3, 4, 5})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 1);
  }

  queue.clear();

  {
    for (const auto v : {5, 4, 3, 2, 1})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 1);
  }

  queue.clear();

  {
    for (const auto v : {7, 3, 9, 1, 5, 2, 8, 4, 6})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 1);

    std::vector<std::int32_t> result;
    while (!queue.empty())
    {
      auto& value = result.emplace_back();
      queue.pop(value);
    }

    EXPECT_THAT(result, testing::ElementsAreArray({1, 2, 3, 4, 5, 6, 7, 8, 9}));
  }

  queue.clear();

  {
    for (const auto v : {3, 3, 3, 7, 7, 7, 7})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 3);

    std::vector<std::int32_t> result;
    while (!queue.empty())
    {
      auto& value = result.emplace_back();
      queue.pop(value);
    }

    EXPECT_THAT(result, testing::ElementsAreArray({3, 3, 3, 7, 7, 7, 7}));
  }

  queue.clear();

  {
    for (const auto v : {7, 7, 7, 7, 7, 7, 7})
    {
      queue.push(v);
    }

    EXPECT_EQ(queue.top(), 7);

    std::vector<std::int32_t> result;
    while (!queue.empty())
    {
      auto& value = result.emplace_back();
      queue.pop(value);
    }

    EXPECT_THAT(result, testing::ElementsAreArray({7, 7, 7, 7, 7, 7, 7}));
  }
}
