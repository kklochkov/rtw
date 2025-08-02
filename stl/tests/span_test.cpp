#include "stl/span.h"

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

using Span = rtw::stl::Span<Struct>;

} // namespace

TEST(SpanTest, constructors)
{
  {
    // Default constructor
    Span span;
    EXPECT_EQ(span.size(), 0U);
    EXPECT_TRUE(span.empty());
    EXPECT_EQ(span.data(), nullptr);
  }

  {
    // Constructor with pointer and size
    Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
    Span span{arr, 3U}; // NOLINT
    EXPECT_EQ(span.size(), 3U);
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(span.data(), arr);
  }

  {
    // Constructor with C-style array
    Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
    Span span{arr}; // NOLINT
    EXPECT_EQ(span.size(), 3U);
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(span.data(), arr);
  }

  {
    // Constructor with begin and end iterators
    Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
    Span span{arr, arr + 3U}; // NOLINT
    EXPECT_EQ(span.size(), 3U);
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(span.data(), arr);
  }

  {
    // Constructor with another span
    Struct arr[3] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
    Span original{arr, 3U}; // NOLINT
    Span span{original};
    EXPECT_EQ(span.size(), original.size());
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(span.data(), original.data());
  }

  {
    // Constructor with a span of different type
    Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
    Span original{arr, 3U}; // NOLINT
    rtw::stl::Span<const Struct> const_span(original);
    EXPECT_EQ(const_span.size(), original.size());
    EXPECT_FALSE(const_span.empty());
    EXPECT_EQ(const_span.data(), original.data());
  }

  {
    // Constructor with a vector
    std::vector<Struct> vec = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
    Span span{vec};
    EXPECT_EQ(span.size(), vec.size());
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(span.data(), vec.data());
  }
}

TEST(SpanTest, modifying_operations)
{
  Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
  Span span{arr, 3U}; // NOLINT

  // Modify elements
  span[0] = Struct{10.0F, 20, 30};
  EXPECT_EQ(span[0], (Struct{10.0F, 20, 30}));

  // Modify using front and back
  span.front() = Struct{11.0F, 21, 31};
  EXPECT_EQ(span.front(), (Struct{11.0F, 21, 31}));

  span.back() = Struct{12.0F, 22, 32};
  EXPECT_EQ(span.back(), (Struct{12.0F, 22, 32}));
}

TEST(SpanTest, subspan)
{
  Struct arr[5U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}, Struct{10.0F, 11, 12},
                    Struct{13.0F, 14, 15}};
  Span span{arr, 5U}; // NOLINT

  // Create a subspan
  auto subspan = span.subspan(1U, 3U);
  EXPECT_EQ(subspan.size(), 3U);
  EXPECT_EQ(subspan[0], (Struct{4.0F, 5, 6}));
  EXPECT_EQ(subspan[1], (Struct{7.0F, 8, 9}));
  EXPECT_EQ(subspan[2], (Struct{10.0F, 11, 12}));
}

TEST(SpanTest, first_and_last)
{
  Struct arr[5U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}, Struct{10.0F, 11, 12},
                    Struct{13.0F, 14, 15}};
  Span span{arr, 5U}; // NOLINT

  // Get first and last elements
  auto first = span.first(3U);
  EXPECT_EQ(first.size(), 3U);
  EXPECT_EQ(first[0], (Struct{1.0F, 2, 3}));
  EXPECT_EQ(first[1], (Struct{4.0F, 5, 6}));
  EXPECT_EQ(first[2], (Struct{7.0F, 8, 9}));

  auto last = span.last(2U);
  EXPECT_EQ(last.size(), 2U);
  EXPECT_EQ(last[0], (Struct{10.0F, 11, 12}));
  EXPECT_EQ(last[1], (Struct{13.0F, 14, 15}));
}

TEST(SpanTest, make_span)
{
  Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};

  // Using make_span with C-style array
  auto span = rtw::stl::make_span(arr);
  EXPECT_EQ(span.size(), 3U);
  EXPECT_EQ(span[0], (Struct{1.0F, 2, 3}));

  // Using make_span with std::vector
  std::vector<Struct> vec = {Struct{10.0F, 20, 30}, Struct{40.0F, 50, 60}};
  auto vec_span = rtw::stl::make_span(vec);
  EXPECT_EQ(vec_span.size(), vec.size());
  EXPECT_EQ(vec_span[0], (Struct{10.0F, 20, 30}));
}

TEST(SpanTest, as_bytes)
{
  Struct arr[3U] = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
  Span span{arr, 3U}; // NOLINT

  // Convert to bytes
  auto byte_span = rtw::stl::as_bytes(span);
  EXPECT_EQ(byte_span.size(), sizeof(Struct) * span.size());
}

TEST(SpanTest, check_types)
{
  // Check if Span<Struct> is a span of Struct
  static_assert(std::is_same_v<rtw::stl::Span<Struct>::element_type, Struct>,
                "Span<Struct> should be a span of Struct");

  // Check if Span<const Struct> is a span of const Struct
  static_assert(std::is_same_v<rtw::stl::Span<const Struct>::element_type, const Struct>,
                "Span<const Struct> should be a span of const Struct");

  std::vector<Struct> vec = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};
  const std::vector<Struct> const_vec = {Struct{1.0F, 2, 3}, Struct{4.0F, 5, 6}, Struct{7.0F, 8, 9}};

  Span span_from_vec{vec};
  rtw::stl::Span<const Struct> const_span_from_vec{const_vec};

  static_assert(std::is_same_v<decltype(span_from_vec)::element_type, Struct>,
                "Span from vector should be a span of Struct");
  static_assert(std::is_same_v<decltype(const_span_from_vec)::element_type, const Struct>,
                "Span from const vector should be a span of const Struct");
}
