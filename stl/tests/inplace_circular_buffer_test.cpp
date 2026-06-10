#include "stl/circular_buffer.h"

#include <gtest/gtest.h>
#include <iterator>

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

using InplaceCircularBuffer = rtw::stl::InplaceCircularBuffer<Struct, 10U>;

} // namespace

TEST(InplaceCircularBufferTest, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceCircularBuffer>,
                "InplaceCircularBuffer should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceCircularBuffer>,
                "InplaceCircularBuffer should be trivially destructible.");

  InplaceCircularBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
  EXPECT_TRUE(buffer.begin() == buffer.end());

  EXPECT_DEATH(buffer.front(), ".*");
  EXPECT_DEATH(buffer.back(), ".*");
  EXPECT_DEATH(buffer[0U], ".*");
  EXPECT_DEATH(buffer.pop_front(), ".*");
  EXPECT_DEATH(buffer.pop_back(), ".*");
}

TEST(InplaceCircularBufferTest, push_front)
{
  InplaceCircularBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    buffer.push_front(s);
    EXPECT_EQ(buffer[buffer.size() - i - 1U], s);
  }

  EXPECT_EQ(buffer.front(), (Struct{10.0F, 10, 10U}));
  EXPECT_EQ(buffer.back(), (Struct{1.0F, 1, 1U}));
  EXPECT_EQ(buffer.size(), buffer.capacity());
  EXPECT_FALSE(buffer.cbegin() == buffer.cend());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[buffer.size() - i - 1U], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                                      static_cast<std::uint8_t>(i + 1U)}));
  }

  buffer.push_front(Struct{13.0F, 13, static_cast<uint8_t>(13)});
  buffer.push_front(Struct{14.0F, 14, static_cast<uint8_t>(14)});

  EXPECT_EQ(buffer.size(), buffer.capacity());

  EXPECT_EQ(buffer[0U], buffer.front());
  EXPECT_EQ(buffer[buffer.size() - 1U], buffer.back());
  EXPECT_EQ(buffer[0U], (Struct{14.0F, 14, 14U}));
  EXPECT_EQ(buffer[1U], (Struct{13.0F, 13, 13U}));

  buffer.clear();

  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
  EXPECT_TRUE(buffer.cbegin() == buffer.cend());
}

TEST(InplaceCircularBufferTest, emplace_front)
{
  InplaceCircularBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    const auto& s = buffer.emplace_front(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                         static_cast<std::uint8_t>(i + 1U));
    EXPECT_EQ(buffer[buffer.size() - i - 1U], s);
    EXPECT_EQ(buffer[buffer.size() - i - 1U], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                                      static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_EQ(buffer.front(), (Struct{10.0F, 10, 10U}));
  EXPECT_EQ(buffer.back(), (Struct{1.0F, 1, 1U}));
  EXPECT_EQ(buffer.size(), buffer.capacity());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[buffer.size() - i - 1U], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                                      static_cast<std::uint8_t>(i + 1U)}));
  }

  buffer.emplace_front(13.0F, 13, static_cast<uint8_t>(13));
  buffer.emplace_front(14.0F, 14, static_cast<uint8_t>(14));

  EXPECT_EQ(buffer.size(), buffer.capacity());

  EXPECT_EQ(buffer[0U], buffer.front());
  EXPECT_EQ(buffer[buffer.size() - 1U], buffer.back());
  EXPECT_EQ(buffer[0U], (Struct{14.0F, 14, 14U}));
  EXPECT_EQ(buffer[1U], (Struct{13.0F, 13, 13U}));

  buffer.clear();
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(InplaceCircularBufferTest, push_back)
{
  InplaceCircularBuffer buffer;
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());

  for (std::size_t i = 0U; i < buffer.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    buffer.push_back(s);
    EXPECT_EQ(buffer[i], s);
  }

  EXPECT_EQ(buffer.front(), (Struct{1.0F, 1, 1U}));
  EXPECT_EQ(buffer.back(), (Struct{10.0F, 10, 10U}));
  EXPECT_EQ(buffer.size(), buffer.capacity());
  EXPECT_FALSE(buffer.begin() == buffer.end());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  buffer.push_back(Struct{13.0F, 13, static_cast<uint8_t>(13)});
  buffer.push_back(Struct{14.0F, 14, static_cast<uint8_t>(14)});

  EXPECT_EQ(buffer.size(), buffer.capacity());

  EXPECT_EQ(buffer[0U], buffer.front());
  EXPECT_EQ(buffer[buffer.size() - 1U], buffer.back());
  EXPECT_EQ(buffer[0U], (Struct{3.0F, 3, 3U}));
  EXPECT_EQ(buffer[1U], (Struct{4.0F, 4, 4U}));

  buffer.clear();

  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
  EXPECT_TRUE(buffer.begin() == buffer.end());
}

TEST(InplaceCircularBufferTest, emplace_back)
{
  InplaceCircularBuffer buffer;
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

  EXPECT_EQ(buffer.front(), (Struct{1.0F, 1, 1U}));
  EXPECT_EQ(buffer.back(), (Struct{10.0F, 10, 10U}));
  EXPECT_EQ(buffer.size(), buffer.capacity());

  for (std::size_t i = 0U; i < buffer.size(); ++i)
  {
    EXPECT_EQ(buffer[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  buffer.emplace_back(13.0F, 13, static_cast<uint8_t>(13));
  buffer.emplace_back(14.0F, 14, static_cast<uint8_t>(14));

  EXPECT_EQ(buffer.size(), buffer.capacity());

  EXPECT_EQ(buffer[0U], buffer.front());
  EXPECT_EQ(buffer[buffer.size() - 1U], buffer.back());
  EXPECT_EQ(buffer[0U], (Struct{3.0F, 3, 3U}));
  EXPECT_EQ(buffer[1U], (Struct{4.0F, 4, 4U}));

  buffer.clear();
  EXPECT_EQ(buffer.size(), 0U);
  EXPECT_EQ(buffer.capacity(), 10U);
  EXPECT_TRUE(buffer.empty());
}

TEST(InplaceCircularBuffer, pop_back)
{
  InplaceCircularBuffer buffer;
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

TEST(InplaceCircularBufferTest, iterators)
{
  InplaceCircularBuffer buffer;
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

  EXPECT_EQ(index, buffer.size());
  EXPECT_EQ(std::distance(buffer.begin(), buffer.end()), static_cast<std::ptrdiff_t>(buffer.size()));

  const auto begin = buffer.begin();
  const auto end = buffer.end();
  EXPECT_EQ(end - begin, static_cast<std::ptrdiff_t>(buffer.size()));
  EXPECT_EQ(begin[3], buffer[3]);
  EXPECT_EQ(*(begin + 4), buffer[4]);

  std::size_t reverse_index = buffer.size();
  for (auto it = buffer.rbegin(); it != buffer.rend(); ++it)
  {
    --reverse_index;
    EXPECT_EQ(*it, buffer[reverse_index]);
  }

  buffer.pop_back();
  buffer.pop_back();
  buffer.pop_back();

  EXPECT_EQ(std::distance(buffer.begin(), buffer.end()), static_cast<std::ptrdiff_t>(buffer.size()));

  buffer.pop_front();
  buffer.pop_front();
  buffer.emplace_back(11.0F, 11, static_cast<std::uint8_t>(11));
  buffer.emplace_back(12.0F, 12, static_cast<std::uint8_t>(12));

  const Struct expected[] = {
      {3.0F, 3, 3U},
      {4.0F, 4, 4U},
      {5.0F, 5, 5U},
      {6.0F, 6, 6U},
      {7.0F, 7, 7U},
      {11.0F, 11, static_cast<std::uint8_t>(11)},
      {12.0F, 12, static_cast<std::uint8_t>(12)},
  };

  ASSERT_EQ(buffer.size(), std::size(expected));
  index = 0U;
  for (const auto& value : buffer)
  {
    EXPECT_EQ(value, expected[index]);
    ++index;
  }
}

TEST(InplaceCircularBufferTest, emplace_front_overwrite_at_full_capacity)
{
  rtw::stl::InplaceCircularBuffer<std::int32_t, 4U> buffer;
  buffer.emplace_back(1);
  buffer.emplace_back(2);
  buffer.emplace_back(3);
  buffer.emplace_back(4);
  ASSERT_TRUE(buffer.full());
  ASSERT_EQ(buffer.size(), 4U);

  // emplace_front on a full buffer: overwrites the back (newest) element.
  buffer.emplace_front(99);
  EXPECT_EQ(buffer.size(), 4U);
  EXPECT_TRUE(buffer.full());
  // Logical order: 99 (new front), 1, 2, 3. Old back (4) was overwritten.
  EXPECT_EQ(buffer[0U], 99);
  EXPECT_EQ(buffer[1U], 1);
  EXPECT_EQ(buffer[2U], 2);
  EXPECT_EQ(buffer[3U], 3);
  EXPECT_EQ(buffer.front(), 99);
  EXPECT_EQ(buffer.back(), 3);
}

TEST(InplaceCircularBufferTest, emplace_back_overwrite_at_full_capacity)
{
  rtw::stl::InplaceCircularBuffer<std::int32_t, 4U> buffer;
  buffer.emplace_back(1);
  buffer.emplace_back(2);
  buffer.emplace_back(3);
  buffer.emplace_back(4);
  ASSERT_TRUE(buffer.full());

  // emplace_back on a full buffer: overwrites the front (oldest) element.
  buffer.emplace_back(99);
  EXPECT_EQ(buffer.size(), 4U);
  EXPECT_TRUE(buffer.full());
  // Logical order: 2, 3, 4, 99. Old front (1) was overwritten.
  EXPECT_EQ(buffer[0U], 2);
  EXPECT_EQ(buffer[1U], 3);
  EXPECT_EQ(buffer[2U], 4);
  EXPECT_EQ(buffer[3U], 99);
  EXPECT_EQ(buffer.front(), 2);
  EXPECT_EQ(buffer.back(), 99);
}

TEST(InplaceCircularBufferTest, full_method)
{
  rtw::stl::InplaceCircularBuffer<std::int32_t, 3U> buffer;
  EXPECT_FALSE(buffer.full());

  buffer.emplace_back(1);
  EXPECT_FALSE(buffer.full());

  buffer.emplace_back(2);
  EXPECT_FALSE(buffer.full());

  buffer.emplace_back(3);
  EXPECT_TRUE(buffer.full());

  buffer.pop_front();
  EXPECT_FALSE(buffer.full());
}

// =============================================================================
// Constexpr evaluation tests (static_assert)
// =============================================================================
namespace
{

struct ConstexprBase
{
  std::int32_t tag{};
};

struct BufVal : ConstexprBase
{
  constexpr BufVal() noexcept = default;
  constexpr explicit BufVal(std::int32_t v) noexcept : ConstexprBase{0}, val{v} {}
  std::int32_t val{};
  constexpr bool operator==(const BufVal& other) const noexcept { return val == other.val; }
};

static_assert(!rtw::stl::details::IS_TRIVIAL_V<BufVal>,
              "BufVal must be non-trivial to use the constexpr storage path.");

constexpr auto make_constexpr_buffer()
{
  rtw::stl::InplaceCircularBuffer<BufVal, 4U> buf;
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  buf.pop_front();
  return buf;
}

static_assert(make_constexpr_buffer().size() == 2U, "constexpr buffer size");
static_assert(make_constexpr_buffer().front() == BufVal{2}, "constexpr buffer front");
static_assert(make_constexpr_buffer().back() == BufVal{3}, "constexpr buffer back");
static_assert(!make_constexpr_buffer().empty(), "constexpr buffer not empty");
static_assert(!make_constexpr_buffer().full(), "constexpr buffer not full");

constexpr auto make_constexpr_buffer_overwrite()
{
  rtw::stl::InplaceCircularBuffer<BufVal, 3U> buf;
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  // Full — next push_back overwrites front (oldest).
  buf.emplace_back(4);
  return buf;
}

static_assert(make_constexpr_buffer_overwrite().size() == 3U, "constexpr buffer overwrite size");
static_assert(make_constexpr_buffer_overwrite().front() == BufVal{2}, "constexpr buffer overwrite front");
static_assert(make_constexpr_buffer_overwrite().back() == BufVal{4}, "constexpr buffer overwrite back");
static_assert(make_constexpr_buffer_overwrite().full(), "constexpr buffer overwrite full");

constexpr auto make_constexpr_buffer_emplace_front()
{
  rtw::stl::InplaceCircularBuffer<BufVal, 4U> buf;
  buf.emplace_front(1);
  buf.emplace_front(2);
  buf.emplace_front(3);
  return buf;
}

static_assert(make_constexpr_buffer_emplace_front().size() == 3U, "constexpr buffer emplace_front size");
static_assert(make_constexpr_buffer_emplace_front().front() == BufVal{3}, "constexpr buffer emplace_front front");
static_assert(make_constexpr_buffer_emplace_front().back() == BufVal{1}, "constexpr buffer emplace_front back");

} // namespace
