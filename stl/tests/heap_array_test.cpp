#include "stl/heap_array.h"

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

using HeapArray = rtw::stl::HeapArray<Struct>;

} // namespace

TEST(HeapArrayTest, constructor)
{
  HeapArray heap_array{10U};
  EXPECT_EQ(heap_array.size(), 10U);

  EXPECT_DEATH(HeapArray{0U}, ".*");
}

TEST(HeapArrayTest, basic)
{
  HeapArray heap_array{10U};
  EXPECT_EQ(heap_array.size(), 10U);

  for (const auto& s : heap_array)
  {
    EXPECT_EQ(s, Struct{});
  }

  for (auto it = heap_array.begin(); it != heap_array.end(); ++it)
  {
    EXPECT_TRUE(it.is_constructed());
    EXPECT_EQ(*it, Struct{});
  }

  for (std::size_t i = 0U; i < heap_array.size(); ++i)
  {
    heap_array[i] =
        Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
  }

  for (std::size_t i = 0U; i < heap_array.size(); ++i)
  {
    EXPECT_EQ(heap_array[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                     static_cast<std::uint8_t>(i + 1U)}));
  }
}

TEST(HeapArrayTest, iterators)
{
  HeapArray heap_array{10U};
  EXPECT_EQ(heap_array.size(), 10U);

  for (std::size_t i = 0U; i < heap_array.size(); ++i)
  {
    heap_array[i] =
        Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
  }

  std::size_t index = 0U;
  for (const auto& value : heap_array)
  {
    EXPECT_EQ(&value, &heap_array[index]);
    EXPECT_EQ(value, heap_array[index]);
    ++index;
  }

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(heap_array.begin(), heap_array.end()));
}
