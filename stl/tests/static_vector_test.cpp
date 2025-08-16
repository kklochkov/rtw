#include "stl/static_vector.h"

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
