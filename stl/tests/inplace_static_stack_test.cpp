#include "stl/static_stack.h"

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

using InplaceStaticStack = rtw::stl::InplaceStaticStack<Struct, 10U>;

} // namespace

TEST(InplaceStaticStackTest, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceStaticStack>, "InplaceStaticStack should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceStaticStack>,
                "InplaceStaticStack should be trivially destructible.");

  InplaceStaticStack stack;
  EXPECT_EQ(stack.size(), 0U);
  EXPECT_EQ(stack.capacity(), 10U);
  EXPECT_TRUE(stack.empty());
}

TEST(InplaceStaticStackTest, push_pop)
{
  InplaceStaticStack stack;
  EXPECT_EQ(stack.size(), 0U);
  EXPECT_EQ(stack.capacity(), 10U);
  EXPECT_TRUE(stack.empty());

  for (std::size_t i = 0U; i < stack.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    stack.push(s);
  }

  EXPECT_EQ(stack.size(), stack.capacity());
  EXPECT_EQ(stack.top(), (Struct{10.0F, 10, 10U}));
  EXPECT_EQ(stack.bottom(), (Struct{1.0F, 1, 1U}));

  EXPECT_DEATH(stack.push(Struct{11.0F, 11, static_cast<uint8_t>(11)}), ".*");

  std::size_t i = stack.capacity() + 1U;
  while (!stack.empty())
  {
    Struct s;
    stack.pop(s);

    EXPECT_EQ(
        s, (Struct{static_cast<float>(i - 1U), static_cast<std::int32_t>(i - 1U), static_cast<std::uint8_t>(i - 1U)}));

    --i;
  }

  EXPECT_EQ(stack.size(), 0U);
  EXPECT_EQ(stack.capacity(), 10U);
  EXPECT_TRUE(stack.empty());

  EXPECT_DEATH(stack.pop(), ".*");
}

TEST(InplaceStaticStackTest, emplace_pop)
{
  InplaceStaticStack stack;
  EXPECT_EQ(stack.size(), 0U);
  EXPECT_EQ(stack.capacity(), 10U);
  EXPECT_TRUE(stack.empty());

  for (std::size_t i = 0U; i < stack.capacity(); ++i)
  {
    stack.emplace(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U));
  }

  EXPECT_EQ(stack.size(), stack.capacity());
  EXPECT_EQ(stack.top(), (Struct{10.0F, 10, 10U}));
  EXPECT_EQ(stack.bottom(), (Struct{1.0F, 1, 1U}));

  EXPECT_DEATH(stack.emplace(11.0F, 11, static_cast<uint8_t>(11)), ".*");

  std::size_t i = stack.capacity() + 1U;
  while (!stack.empty())
  {
    Struct s;
    stack.pop(s);

    EXPECT_EQ(
        s, (Struct{static_cast<float>(i - 1U), static_cast<std::int32_t>(i - 1U), static_cast<std::uint8_t>(i - 1U)}));

    --i;
  }

  EXPECT_EQ(stack.size(), 0U);
  EXPECT_EQ(stack.capacity(), 10U);
  EXPECT_TRUE(stack.empty());

  EXPECT_DEATH(stack.pop(), ".*");
}
