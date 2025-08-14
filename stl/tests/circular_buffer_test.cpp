#include "stl/circular_buffer.h"

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

using CircularBuffer = rtw::stl::CircularBuffer<Struct>;

} // namespace

TEST(CircularBufferTest, constructor)
{
  CircularBuffer buffer{10U};
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  EXPECT_DEATH(CircularBuffer{0U}, ".*");
}

TEST(CircularBufferTest, push_back)
{
  CircularBuffer buffer{10U};
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

  buffer.push_back(Struct{13.0F, 13, static_cast<uint8_t>(13)});
  buffer.push_back(Struct{14.0F, 14, static_cast<uint8_t>(14)});

  EXPECT_EQ(buffer.size(), buffer.capacity());

  EXPECT_EQ(buffer[0U], (Struct{13.0F, 13, 13U}));
  EXPECT_EQ(buffer[1U], (Struct{14.0F, 14, 14U}));

  buffer.clear();
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(CircularBufferTest, emplace_back)
{
  CircularBuffer buffer{10U};
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

  buffer.emplace_back(13.0F, 13, static_cast<uint8_t>(13));
  buffer.emplace_back(14.0F, 14, static_cast<uint8_t>(14));

  EXPECT_EQ(buffer.size(), buffer.capacity());

  EXPECT_EQ(buffer[0U], (Struct{13.0F, 13, 13U}));
  EXPECT_EQ(buffer[1U], (Struct{14.0F, 14, 14U}));

  buffer.clear();
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(CircularBuffer, pop_back)
{
  CircularBuffer buffer{10U};
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    buffer.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
  }

  EXPECT_EQ(buffer.size(), buffer.capacity());
  EXPECT_FALSE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  buffer.pop_back();
  buffer.pop_back();

  EXPECT_EQ(buffer.size(), buffer.capacity() - 2U);

  EXPECT_EQ(buffer[buffer.size() - 1U], (Struct{8.0F, 8, 8U}));

  while (!buffer.empty())
  {
    buffer.pop_back();
  }

  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(CircularBufferTest, iterators)
{
  CircularBuffer buffer{10U};
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
    EXPECT_EQ(&value, &buffer[index]);
    EXPECT_EQ(value, buffer[index]);
    ++index;
  }

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(buffer.begin(), buffer.end()));

  buffer.pop_back();
  buffer.pop_back();
  buffer.pop_back();

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(buffer.begin(), buffer.end()));
}
