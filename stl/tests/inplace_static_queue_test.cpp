#include "stl/static_queue.h"

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

using InplaceStaticQueue = rtw::stl::InplaceStaticQueue<Struct, 10U>;

} // namespace

TEST(InplaceStaticQueueTest, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceStaticQueue>, "InplaceStaticQueue should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceStaticQueue>,
                "InplaceStaticQueue should be trivially destructible.");

  InplaceStaticQueue queue;
  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());
}

TEST(InplaceStaticQueueTest, push_pop)
{
  InplaceStaticQueue queue;
  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  for (std::size_t i = 0U; i < queue.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    queue.push(s);
  }

  EXPECT_EQ(queue.size(), queue.capacity());
  EXPECT_EQ(queue.front(), (Struct{1.0F, 1, 1U}));
  EXPECT_EQ(queue.back(), (Struct{10.0F, 10, 10U}));

  EXPECT_DEATH(queue.push(Struct{11.0F, 11, static_cast<uint8_t>(11)}), ".*");

  std::size_t i = 0U;
  while (!queue.empty())
  {
    Struct s;
    queue.pop(s);

    EXPECT_EQ(
        s, (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)}));

    ++i;
  }

  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  EXPECT_DEATH(queue.pop(), ".*");
}

TEST(InplaceStaticQueueTest, emplace_pop)
{
  InplaceStaticQueue queue;
  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  for (std::size_t i = 0U; i < queue.capacity(); ++i)
  {
    queue.emplace(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U));
  }

  EXPECT_EQ(queue.size(), queue.capacity());
  EXPECT_EQ(queue.front(), (Struct{1.0F, 1, 1U}));
  EXPECT_EQ(queue.back(), (Struct{10.0F, 10, 10U}));

  EXPECT_DEATH(queue.emplace(11.0F, 11, static_cast<uint8_t>(11)), ".*");

  std::size_t i = 0U;
  while (!queue.empty())
  {
    Struct s;
    queue.pop(s);

    EXPECT_EQ(
        s, (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)}));

    ++i;
  }

  EXPECT_EQ(queue.size(), 0U);
  EXPECT_EQ(queue.capacity(), 10U);
  EXPECT_TRUE(queue.empty());

  EXPECT_DEATH(queue.pop(), ".*");
}
