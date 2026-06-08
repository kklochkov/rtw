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
  static_assert(std::is_trivially_copyable_v<InplacePackedBuffer>, "InplacePackedBuffer should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplacePackedBuffer>,
                "InplacePackedBuffer should be trivially destructible.");

  InplacePackedBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
  EXPECT_TRUE(buffer.begin() == buffer.end());
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
  EXPECT_FALSE(buffer.begin() == buffer.end());

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
  EXPECT_TRUE(buffer.begin() == buffer.end());
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
    EXPECT_EQ(&value, &buffer[index]);
    EXPECT_EQ(value, buffer[index]);
    ++index;
  }

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(buffer.begin(), buffer.end()));

  buffer.remove(3U);
  buffer.remove(4U);
  buffer.remove(5U);

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(buffer.begin(), buffer.end()));
}

// =============================================================================
// Constexpr evaluation tests (static_assert)
// =============================================================================
namespace
{

struct ConstexprBase
{
  int tag{};
};

struct PBConstexprVal : ConstexprBase
{
  constexpr PBConstexprVal() noexcept = default;
  constexpr explicit PBConstexprVal(int v) noexcept : ConstexprBase{0}, val{v} {}
  int val{};
  constexpr bool operator==(const PBConstexprVal& other) const noexcept { return val == other.val; }
};

static_assert(!rtw::stl::details::IS_TRIVIAL_V<PBConstexprVal>,
              "PBConstexprVal must be non-trivial to use the constexpr storage path.");

constexpr auto make_constexpr_packed_buffer()
{
  rtw::stl::InplacePackedBuffer<PBConstexprVal, 8U> buf;
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  buf.emplace_back(4);
  return buf;
}

static_assert(make_constexpr_packed_buffer().size() == 4U, "constexpr packed_buffer size");
static_assert(make_constexpr_packed_buffer()[0U] == PBConstexprVal{1}, "constexpr packed_buffer [0]");
static_assert(make_constexpr_packed_buffer()[3U] == PBConstexprVal{4}, "constexpr packed_buffer [3]");
static_assert(!make_constexpr_packed_buffer().empty(), "constexpr packed_buffer not empty");
static_assert(!make_constexpr_packed_buffer().full(), "constexpr packed_buffer not full");
static_assert(make_constexpr_packed_buffer().capacity() == 8U, "constexpr packed_buffer capacity");

constexpr auto make_constexpr_packed_buffer_with_pop()
{
  rtw::stl::InplacePackedBuffer<PBConstexprVal, 8U> buf;
  buf.emplace_back(10);
  buf.emplace_back(20);
  buf.emplace_back(30);
  buf.pop_back();
  return buf;
}

static_assert(make_constexpr_packed_buffer_with_pop().size() == 2U, "constexpr packed_buffer pop size");
static_assert(make_constexpr_packed_buffer_with_pop()[1U] == PBConstexprVal{20}, "constexpr packed_buffer pop [1]");

constexpr auto make_constexpr_packed_buffer_with_remove()
{
  rtw::stl::InplacePackedBuffer<PBConstexprVal, 8U> buf;
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  buf.emplace_back(4);
  buf.remove(1U); // removes 2, moves 4 into position 1
  return buf;
}

static_assert(make_constexpr_packed_buffer_with_remove().size() == 3U, "constexpr packed_buffer remove size");
static_assert(make_constexpr_packed_buffer_with_remove()[0U] == PBConstexprVal{1},
              "constexpr packed_buffer remove [0]");
static_assert(make_constexpr_packed_buffer_with_remove()[1U] == PBConstexprVal{4},
              "constexpr packed_buffer remove [1] (swapped)");
static_assert(make_constexpr_packed_buffer_with_remove()[2U] == PBConstexprVal{3},
              "constexpr packed_buffer remove [2]");

constexpr auto make_constexpr_packed_buffer_front_back()
{
  rtw::stl::InplacePackedBuffer<PBConstexprVal, 8U> buf;
  buf.emplace_back(100);
  buf.emplace_back(200);
  buf.emplace_back(300);
  return buf;
}

static_assert(make_constexpr_packed_buffer_front_back().front() == PBConstexprVal{100},
              "constexpr packed_buffer front");
static_assert(make_constexpr_packed_buffer_front_back().back() == PBConstexprVal{300}, "constexpr packed_buffer back");

constexpr auto make_constexpr_packed_buffer_cleared()
{
  rtw::stl::InplacePackedBuffer<PBConstexprVal, 4U> buf;
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.clear();
  return buf;
}

static_assert(make_constexpr_packed_buffer_cleared().size() == 0U, "constexpr packed_buffer clear");
static_assert(make_constexpr_packed_buffer_cleared().empty(), "constexpr packed_buffer empty after clear");

} // namespace
