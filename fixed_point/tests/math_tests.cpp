#include "fixed_point/math.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

template <typename T>
class SignedFixedPointMathTest : public ::testing::Test
{
public:
  constexpr static double get_trigonometric_sin_cos_abs_error() noexcept
  {
    if constexpr (std::is_same_v<T, rtw::fixed_point::FixedPoint8>)
    {
      return 0.03;
    }
    else if constexpr ((std::is_same_v<T, rtw::fixed_point::FixedPoint16>)
                       || (std::is_same_v<T, rtw::fixed_point::FixedPoint32>))
    {
      return 0.0003;
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr static double get_trigonometric_tan_abs_error(const std::int16_t deg) noexcept
  {
    if constexpr (std::is_same_v<T, rtw::fixed_point::FixedPoint8>)
    {
      if (std::abs(deg) <= 70)
      {
        return 0.04;
      }
      if (std::abs(deg) <= 80)
      {
        return 0.2;
      }
      return 0.03;
    }
    else if constexpr (std::is_same_v<T, rtw::fixed_point::FixedPoint16>)
    {
      return std::abs(deg) <= 70 ? 0.0003 : 0.06;
    }
    else if constexpr (std::is_same_v<T, rtw::fixed_point::FixedPoint32>)
    {
      return 0.0003;
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr std::int16_t get_tan_angle_range_deg() noexcept
  {
    if constexpr (std::is_same_v<T, rtw::fixed_point::FixedPoint8>)
    {
      return 80;
    }
    else if constexpr (std::is_same_v<T, rtw::fixed_point::FixedPoint16>
                       || std::is_same_v<T, rtw::fixed_point::FixedPoint32>)
    {
      return 89;
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }
};

using SignedFixedPointTypes =
    ::testing::Types<rtw::fixed_point::FixedPoint8, rtw::fixed_point::FixedPoint16, rtw::fixed_point::FixedPoint32>;
TYPED_TEST_SUITE(SignedFixedPointMathTest, SignedFixedPointTypes, );

TYPED_TEST(SignedFixedPointMathTest, abs)
{
  EXPECT_EQ(rtw::fixed_point::math::abs(TypeParam(-1.23)), 1.23);
  EXPECT_EQ(rtw::fixed_point::math::abs(TypeParam(1.23)), 1.23);
}

TYPED_TEST(SignedFixedPointMathTest, floor)
{
  EXPECT_EQ(rtw::fixed_point::math::floor(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::fixed_point::math::floor(TypeParam(-1.23)), -2.0);
}

TYPED_TEST(SignedFixedPointMathTest, clamp)
{
  EXPECT_EQ(rtw::fixed_point::math::clamp(TypeParam(1.23), TypeParam(0.0), TypeParam(2.0)), 1.23);
  EXPECT_EQ(rtw::fixed_point::math::clamp(TypeParam(-1.23), TypeParam(0.0), TypeParam(2.0)), 0.0);
  EXPECT_EQ(rtw::fixed_point::math::clamp(TypeParam(3.23), TypeParam(0.0), TypeParam(2.0)), 2.0);
}

TYPED_TEST(SignedFixedPointMathTest, ceil)
{
  EXPECT_EQ(rtw::fixed_point::math::ceil(TypeParam(1.23)), 2.0);
  EXPECT_EQ(rtw::fixed_point::math::ceil(TypeParam(-1.23)), -1.0);
}

TYPED_TEST(SignedFixedPointMathTest, round)
{
  EXPECT_EQ(rtw::fixed_point::math::round(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::fixed_point::math::round(TypeParam(1.5)), 2.0);
  EXPECT_EQ(rtw::fixed_point::math::round(TypeParam(-1.23)), -1.0);
  EXPECT_EQ(rtw::fixed_point::math::round(TypeParam(-1.5)), -1.0);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TYPED_TEST(SignedFixedPointMathTest, sqrt)
{
  const auto max = static_cast<double>(rtw::fixed_point::FixedPoint8::max());
  // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
  for (double i = 0.0; i < max; i += rtw::fixed_point::FixedPoint8::RESOLUTION)
  {
    const auto result = static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(i)));
    const auto expected = std::sqrt(i);
    EXPECT_NEAR(result, expected, TypeParam::RESOLUTION);
  }

  EXPECT_EQ(rtw::fixed_point::math::sqrt(TypeParam(4.0)), std::sqrt(4.0));
  EXPECT_EQ(rtw::fixed_point::math::sqrt(TypeParam(2.0)), std::sqrt(2.0));
  EXPECT_EQ(rtw::fixed_point::math::sqrt(TypeParam(42.0)), std::sqrt(42.0));
  EXPECT_EQ(rtw::fixed_point::math::sqrt(TypeParam(81.0)), std::sqrt(81.0));
  EXPECT_NEAR(static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(72.0))), std::sqrt(72.0),
              TypeParam::RESOLUTION);
  EXPECT_EQ(rtw::fixed_point::math::sqrt(TypeParam(0.0)), std::sqrt(0.0));

  EXPECT_DEATH(rtw::fixed_point::math::sqrt(TypeParam(-1.0)), "");
}

TYPED_TEST(SignedFixedPointMathTest, sin_cos)
{
  EXPECT_EQ(TypeParam::pi(), 3.14159265358979323846);
  for (std::int16_t deg = -360; deg <= 360; ++deg)
  {
    const auto rad = deg * rtw::math_constants::DEG_TO_RAD<double>;
    const auto result_sin = rtw::fixed_point::math::sin(TypeParam(rad));
    const auto expected_sin = std::sin(rad);
    EXPECT_NEAR(static_cast<double>(result_sin), expected_sin, this->get_trigonometric_sin_cos_abs_error());
  }
  for (std::int16_t deg = -360; deg <= 360; ++deg)
  {
    const auto rad = deg * rtw::math_constants::DEG_TO_RAD<double>;
    const auto result_cos = rtw::fixed_point::math::cos(TypeParam(rad));
    const auto expected_cos = std::cos(rad);
    EXPECT_NEAR(static_cast<double>(result_cos), expected_cos, this->get_trigonometric_sin_cos_abs_error());
  }
}

TYPED_TEST(SignedFixedPointMathTest, tan)
{
  for (std::int16_t deg = -this->get_tan_angle_range_deg(); deg <= this->get_tan_angle_range_deg(); ++deg)
  {
    const auto rad = deg * rtw::math_constants::DEG_TO_RAD<double>;
    const auto result_tan = rtw::fixed_point::math::tan(TypeParam(rad));
    const auto expected_tan = std::tan(rad);
    EXPECT_NEAR(static_cast<double>(result_tan), expected_tan, this->get_trigonometric_tan_abs_error(deg));
  }
  {
    EXPECT_DEATH(rtw::fixed_point::math::tan(TypeParam::pi_2()), "");
    EXPECT_DEATH(rtw::fixed_point::math::tan(-TypeParam::pi_2()), "");
  }
}
//  -----------------------------------------------------------------------------------------------
template <typename T>
class UnsignedFixedPointMathTest : public ::testing::Test
{};

using UnsignedFixedPointTypes =
    ::testing::Types<rtw::fixed_point::FixedPoint8U, rtw::fixed_point::FixedPoint16U, rtw::fixed_point::FixedPoint32U>;
TYPED_TEST_SUITE(UnsignedFixedPointMathTest, UnsignedFixedPointTypes, );

TYPED_TEST(UnsignedFixedPointMathTest, floor)
{
  EXPECT_EQ(rtw::fixed_point::math::floor(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::fixed_point::math::floor(TypeParam(1.5)), 1.0);
}

TYPED_TEST(UnsignedFixedPointMathTest, ceil)
{
  EXPECT_EQ(rtw::fixed_point::math::ceil(TypeParam(1.23)), 2.0);
  EXPECT_EQ(rtw::fixed_point::math::ceil(TypeParam(1.5)), 2.0);
}

TYPED_TEST(UnsignedFixedPointMathTest, clamp)
{
  EXPECT_EQ(rtw::fixed_point::math::clamp(TypeParam(1.23), TypeParam(0.0), TypeParam(2.0)), 1.23);
  EXPECT_EQ(rtw::fixed_point::math::clamp(TypeParam(3.23), TypeParam(0.0), TypeParam(2.0)), 2.0);
}

TYPED_TEST(UnsignedFixedPointMathTest, round)
{
  EXPECT_EQ(rtw::fixed_point::math::round(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::fixed_point::math::round(TypeParam(1.5)), 2.0);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TYPED_TEST(UnsignedFixedPointMathTest, sqrt)
{
  const auto max = static_cast<double>(rtw::fixed_point::FixedPoint8U::max());
  // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
  for (double i = 0.0; i < max; i += rtw::fixed_point::FixedPoint8U::RESOLUTION)
  {
    const auto result = static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(i)));
    const auto expected = std::sqrt(i);
    EXPECT_NEAR(result, expected, TypeParam::RESOLUTION);
  }

  EXPECT_NEAR(static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(4.0))), std::sqrt(4.0), TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(2.0))), std::sqrt(2.0), TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(42.0))), std::sqrt(42.0),
              TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(81.0))), std::sqrt(81.0),
              TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::fixed_point::math::sqrt(TypeParam(72.0))), std::sqrt(72.0),
              TypeParam::RESOLUTION);
  EXPECT_EQ(rtw::fixed_point::math::sqrt(TypeParam(0.0)), std::sqrt(0.0));
}
