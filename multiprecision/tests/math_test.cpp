#include "multiprecision/math.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

// =============================================================================
// Constexpr static_assert tests for math utilities
// =============================================================================
namespace
{

namespace math = rtw::multiprecision::math;

// abs: basic correctness
static_assert(math::abs(std::int32_t{5}) == 5, "abs(5) == 5");
static_assert(math::abs(std::int32_t{-5}) == 5, "abs(-5) == 5");
static_assert(math::abs(std::int32_t{0}) == 0, "abs(0) == 0");
static_assert(math::abs(std::int64_t{-1}) == 1, "abs(-1LL) == 1");

// abs: MIN saturates to MAX (avoids signed overflow UB)
static_assert(math::abs(std::numeric_limits<std::int8_t>::min()) == std::numeric_limits<std::int8_t>::max(),
              "abs(INT8_MIN) == INT8_MAX");
static_assert(math::abs(std::numeric_limits<std::int16_t>::min()) == std::numeric_limits<std::int16_t>::max(),
              "abs(INT16_MIN) == INT16_MAX");
static_assert(math::abs(std::numeric_limits<std::int32_t>::min()) == std::numeric_limits<std::int32_t>::max(),
              "abs(INT32_MIN) == INT32_MAX");
static_assert(math::abs(std::numeric_limits<std::int64_t>::min()) == std::numeric_limits<std::int64_t>::max(),
              "abs(INT64_MIN) == INT64_MAX");

// abs: MAX is identity
static_assert(math::abs(std::numeric_limits<std::int32_t>::max()) == std::numeric_limits<std::int32_t>::max(),
              "abs(INT32_MAX) == INT32_MAX");

// round_to_nearest_integer: positive values
static_assert(math::round_to_nearest_integer(2.5) == 3.0, "round(2.5) == 3.0");
static_assert(math::round_to_nearest_integer(2.3) > 2.0, "round(2.3) > 2 (adds 0.5)");
static_assert(static_cast<std::int32_t>(math::round_to_nearest_integer(2.3)) == 2, "int(round(2.3)) == 2");
static_assert(static_cast<std::int32_t>(math::round_to_nearest_integer(2.5)) == 3, "int(round(2.5)) == 3");
static_assert(static_cast<std::int32_t>(math::round_to_nearest_integer(0.0)) == 0, "int(round(0.0)) == 0");

// round_to_nearest_integer: negative values
static_assert(math::round_to_nearest_integer(-2.5) == -3.0, "round(-2.5) == -3.0");
static_assert(static_cast<std::int32_t>(math::round_to_nearest_integer(-2.3)) == -2, "int(round(-2.3)) == -2");
static_assert(static_cast<std::int32_t>(math::round_to_nearest_integer(-2.5)) == -3, "int(round(-2.5)) == -3");

// fmod: basic correctness
static_assert(math::fmod(5.5, 2.0) > 1.49 && math::fmod(5.5, 2.0) < 1.51, "fmod(5.5, 2.0) ~= 1.5");
static_assert(math::fmod(-5.5, 2.0) > -1.51 && math::fmod(-5.5, 2.0) < -1.49, "fmod(-5.5, 2.0) ~= -1.5");
static_assert(math::fmod(5.5, -2.0) > 1.49 && math::fmod(5.5, -2.0) < 1.51, "fmod(5.5, -2.0) ~= 1.5");
static_assert(math::fmod(0.0, 2.0) == 0.0, "fmod(0, 2) == 0");
static_assert(math::fmod(5.0, 2.0) > 0.99 && math::fmod(5.0, 2.0) < 1.01, "fmod(5.0, 2.0) ~= 1.0");

// signbit: constexpr validation
static_assert(math::signbit(std::int32_t{-1}) == true, "signbit(-1) == true");
static_assert(math::signbit(std::int32_t{0}) == false, "signbit(0) == false");
static_assert(math::signbit(std::int32_t{1}) == false, "signbit(1) == false");
static_assert(math::signbit(std::uint32_t{0}) == false, "signbit(uint 0) == false");
static_assert(math::signbit(std::uint32_t{1}) == false, "signbit(uint 1) == false");

} // namespace

TEST(math, signbit)
{
  {
    constexpr std::int8_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::int8_t A = -1;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 1);
  }
  {
    constexpr std::int16_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::int16_t A = -1;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 1);
  }
  {
    constexpr std::int32_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::int32_t A = -1;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 1);
  }
  {
    constexpr std::int64_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::int64_t A = -1;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 1);
  }
  {
    constexpr std::uint8_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::uint16_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::uint32_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
  {
    constexpr std::uint64_t A = 0;
    EXPECT_EQ(rtw::multiprecision::math::signbit(A), 0);
  }
}

TEST(math, count_leading_zero)
{
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint8_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint8_t a = 0xFFU >> i;
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint16_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint16_t a = 0xFFFFU >> i;
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint32_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint32_t a = 0xFF'FF'FF'FF >> i;
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint64_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint64_t a = 0xFF'FF'FF'FF'FF'FF'FF'FFULL >> i;
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
    }
  }
}

TEST(math, abs_runtime)
{
  using rtw::multiprecision::math::abs;

  EXPECT_EQ(abs(std::int32_t{42}), 42);
  EXPECT_EQ(abs(std::int32_t{-42}), 42);
  EXPECT_EQ(abs(std::int32_t{0}), 0);

  // MIN saturates to MAX
  EXPECT_EQ(abs(std::numeric_limits<std::int8_t>::min()), std::numeric_limits<std::int8_t>::max());
  EXPECT_EQ(abs(std::numeric_limits<std::int16_t>::min()), std::numeric_limits<std::int16_t>::max());
  EXPECT_EQ(abs(std::numeric_limits<std::int32_t>::min()), std::numeric_limits<std::int32_t>::max());
  EXPECT_EQ(abs(std::numeric_limits<std::int64_t>::min()), std::numeric_limits<std::int64_t>::max());
}

TEST(math, round_to_nearest_integer_runtime)
{
  using rtw::multiprecision::math::round_to_nearest_integer;

  // Positive half rounds up
  EXPECT_DOUBLE_EQ(round_to_nearest_integer(2.5), 3.0);
  EXPECT_EQ(static_cast<std::int32_t>(round_to_nearest_integer(2.3)), 2);
  EXPECT_EQ(static_cast<std::int32_t>(round_to_nearest_integer(2.7)), 3);

  // Negative half rounds down (away from zero)
  EXPECT_DOUBLE_EQ(round_to_nearest_integer(-2.5), -3.0);
  EXPECT_EQ(static_cast<std::int32_t>(round_to_nearest_integer(-2.3)), -2);
  EXPECT_EQ(static_cast<std::int32_t>(round_to_nearest_integer(-2.7)), -3);

  // Zero
  EXPECT_EQ(static_cast<std::int32_t>(round_to_nearest_integer(0.0)), 0);

  // Float variant
  EXPECT_FLOAT_EQ(round_to_nearest_integer(2.5F), 3.0F);
  EXPECT_FLOAT_EQ(round_to_nearest_integer(-2.5F), -3.0F);
}

TEST(math, fmod_runtime)
{
  using rtw::multiprecision::math::fmod;

  EXPECT_NEAR(fmod(5.5, 2.0), 1.5, 1e-10);
  EXPECT_NEAR(fmod(5.0, 2.0), 1.0, 1e-10);
  EXPECT_NEAR(fmod(-5.5, 2.0), -1.5, 1e-10);
  EXPECT_NEAR(fmod(5.5, -2.0), 1.5, 1e-10);
  EXPECT_DOUBLE_EQ(fmod(0.0, 2.0), 0.0);
  EXPECT_NEAR(fmod(10.0, 3.0), 1.0, 1e-10);
}

TEST(math, fmod_zero_divisor_death)
{
  EXPECT_DEATH(rtw::multiprecision::math::fmod(1.0, 0.0), ".*");
  EXPECT_DEATH(rtw::multiprecision::math::fmod(0.0, 0.0), ".*");
}
