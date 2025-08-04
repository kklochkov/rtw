#include "stl/packed_buffer.h"

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

using InplacePackedBuffer = rtw::stl::InplacePackedBuffer<Struct, 10U>;

} // namespace

TEST(InplacePackedBufferTest, constructor)
{
  InplacePackedBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(InplacePackedBufferTest, push_back)
{
  InplacePackedBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    buffer.push_back(s);
    EXPECT_EQ(buffer[i], s);
  }

  EXPECT_EQ(buffer.size(), buffer.capacity());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_DEATH(buffer.push_back(Struct{}), ".*");

  buffer.clear();
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(InplacePackedBufferTest, emplace_back)
{
  InplacePackedBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    const auto& s = buffer.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                        static_cast<std::uint8_t>(i + 1U));
    EXPECT_EQ(buffer[i], s);
    EXPECT_EQ(buffer[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_EQ(buffer.size(), buffer.capacity());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_DEATH(buffer.emplace_back(), ".*");

  buffer.clear();
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(InplacePackedBufferTest, remove)
{
  InplacePackedBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  // Remove from the head of the buffer.
  {
    for (std::size_t i = 0U; i < buffer.capacity(); ++i)
    {
      buffer.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                          static_cast<std::uint8_t>(i + 1U));
    }

    const auto size = buffer.size() - 1U;
    for (std::size_t i = 0U; i < size; ++i)
    {
      const std::size_t index = 0U;
      const auto* ptr = &buffer[index];

      buffer.remove(index);

      EXPECT_EQ(buffer.size(), buffer.capacity() - i - 1U);
      EXPECT_FALSE(buffer.empty());
      EXPECT_EQ(buffer[index],
                (Struct{static_cast<float>(buffer.size() + 1U), static_cast<std::int32_t>(buffer.size() + 1U),
                        static_cast<std::uint8_t>(buffer.size() + 1U)}));
      EXPECT_EQ(ptr, &buffer[index]);
    }

    EXPECT_EQ(buffer.size(), 1U);
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer[0U], (Struct{2.0F, 2, 2}));

    buffer.remove(0U);

    EXPECT_EQ(buffer.size(), 0U);
    EXPECT_TRUE(buffer.empty());
    EXPECT_DEATH(buffer.remove(0U), ".*");
  }

  // Remove from the tail of the buffer.
  {
    for (std::size_t i = 0U; i < buffer.capacity(); ++i)
    {
      buffer.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                          static_cast<std::uint8_t>(i + 1U));
    }

    const auto size = buffer.size() - 1U;
    for (std::size_t i = 0U; i < size; ++i)
    {
      const std::size_t index = buffer.size() - 2U;
      const auto* ptr = &buffer[index];

      buffer.remove(buffer.size() - 1U);

      EXPECT_EQ(buffer.size(), buffer.capacity() - i - 1U);
      EXPECT_FALSE(buffer.empty());
      EXPECT_EQ(buffer[index], (Struct{static_cast<float>(index + 1U), static_cast<std::int32_t>(index + 1U),
                                       static_cast<std::uint8_t>(index + 1U)}));
      EXPECT_EQ(ptr, &buffer[index]);
    }

    EXPECT_EQ(buffer.size(), 1U);
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer[0U], (Struct{1.0F, 1, 1}));

    buffer.remove(0U);

    EXPECT_EQ(buffer.size(), 0U);
    EXPECT_TRUE(buffer.empty());
    EXPECT_DEATH(buffer.remove(0U), ".*");
  }
}

TEST(InplacePackedBufferTest, iterators)
{
  InplacePackedBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    buffer.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
  }

  std::size_t index = 0U;
  for (const auto& value : buffer)
  {
    EXPECT_EQ(&*value, &buffer[index]);
    EXPECT_EQ(*value, buffer[index]);
    ++index;
  }

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(buffer.begin(), buffer.end()));

  buffer.remove(3U);
  buffer.remove(4U);
  buffer.remove(5U);

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(buffer.begin(), buffer.end()));
}
