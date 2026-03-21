#include "multiprecision/math.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

template <typename T>
class SignedFixedPointMathTest : public ::testing::Test
{
public:
  constexpr static double get_trigonometric_sin_cos_abs_error() noexcept
  {
    if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint8>)
    {
      return 0.03;
    }
    else if constexpr ((std::is_same_v<T, rtw::multiprecision::FixedPoint16>)
                       || (std::is_same_v<T, rtw::multiprecision::FixedPoint32>))
    {
      return 0.0003;
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  // asin/acos have higher error near domain boundaries (±1) due to steep slope
  constexpr static double get_trigonometric_asin_acos_abs_error(double value) noexcept
  {
    if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint8>)
    {
      // Near boundaries (|value| > 0.9), allow larger error for low-precision types
      return std::abs(value) > 0.9 ? 0.05 : 0.03;
    }
    else if constexpr ((std::is_same_v<T, rtw::multiprecision::FixedPoint16>)
                       || (std::is_same_v<T, rtw::multiprecision::FixedPoint32>))
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
    if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint8>)
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
    else if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint16>)
    {
      return std::abs(deg) <= 70 ? 0.0003 : 0.06;
    }
    else if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint32>)
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
    if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint8>)
    {
      return 80;
    }
    else if constexpr (std::is_same_v<T, rtw::multiprecision::FixedPoint16>
                       || std::is_same_v<T, rtw::multiprecision::FixedPoint32>)
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

using SignedFixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8, rtw::multiprecision::FixedPoint16,
                                               rtw::multiprecision::FixedPoint32>;
TYPED_TEST_SUITE(SignedFixedPointMathTest, SignedFixedPointTypes, );

TYPED_TEST(SignedFixedPointMathTest, abs)
{
  EXPECT_EQ(rtw::multiprecision::math::abs(TypeParam(-1.23)), 1.23);
  EXPECT_EQ(rtw::multiprecision::math::abs(TypeParam(1.23)), 1.23);
}

TYPED_TEST(SignedFixedPointMathTest, floor)
{
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(-1.23)), -2.0);
}

TYPED_TEST(SignedFixedPointMathTest, clamp)
{
  EXPECT_EQ(rtw::multiprecision::math::clamp(TypeParam(1.23), TypeParam(0.0), TypeParam(2.0)), 1.23);
  EXPECT_EQ(rtw::multiprecision::math::clamp(TypeParam(-1.23), TypeParam(0.0), TypeParam(2.0)), 0.0);
  EXPECT_EQ(rtw::multiprecision::math::clamp(TypeParam(3.23), TypeParam(0.0), TypeParam(2.0)), 2.0);
}

TYPED_TEST(SignedFixedPointMathTest, ceil)
{
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(1.23)), 2.0);
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(-1.23)), -1.0);
}

TYPED_TEST(SignedFixedPointMathTest, round)
{
  // Round half away from zero (matching std::round behavior)
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(1.5)), 2.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(1.6)), 2.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-1.23)), -1.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-1.5)), -2.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-1.6)), -2.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(2.5)), 3.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-2.5)), -3.0);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TYPED_TEST(SignedFixedPointMathTest, sqrt)
{
  const auto max = static_cast<double>(rtw::multiprecision::FixedPoint8::max());
  // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
  for (double i = 0.0; i < max; i += rtw::multiprecision::FixedPoint8::RESOLUTION)
  {
    const auto result = static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(i)));
    const auto expected = std::sqrt(i);
    EXPECT_NEAR(result, expected, TypeParam::RESOLUTION);
  }

  EXPECT_EQ(rtw::multiprecision::math::sqrt(TypeParam(4.0)), std::sqrt(4.0));
  EXPECT_EQ(rtw::multiprecision::math::sqrt(TypeParam(2.0)), std::sqrt(2.0));
  EXPECT_EQ(rtw::multiprecision::math::sqrt(TypeParam(42.0)), std::sqrt(42.0));
  EXPECT_EQ(rtw::multiprecision::math::sqrt(TypeParam(81.0)), std::sqrt(81.0));
  EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(72.0))), std::sqrt(72.0),
              TypeParam::RESOLUTION);
  EXPECT_EQ(rtw::multiprecision::math::sqrt(TypeParam(0.0)), std::sqrt(0.0));

  EXPECT_DEATH(rtw::multiprecision::math::sqrt(TypeParam(-1.0)), "");
}

TYPED_TEST(SignedFixedPointMathTest, sin_cos)
{
  EXPECT_EQ(TypeParam::pi(), 3.14159265358979323846);
  for (std::int16_t deg = -360; deg <= 360; ++deg)
  {
    const auto rad = deg * rtw::math_constants::DEG_TO_RAD<double>;
    const auto result_sin = rtw::multiprecision::math::sin(TypeParam(rad));
    const auto expected_sin = std::sin(rad);
    EXPECT_NEAR(static_cast<double>(result_sin), expected_sin, this->get_trigonometric_sin_cos_abs_error());
  }
  for (std::int16_t deg = -360; deg <= 360; ++deg)
  {
    const auto rad = deg * rtw::math_constants::DEG_TO_RAD<double>;
    const auto result_cos = rtw::multiprecision::math::cos(TypeParam(rad));
    const auto expected_cos = std::cos(rad);
    EXPECT_NEAR(static_cast<double>(result_cos), expected_cos, this->get_trigonometric_sin_cos_abs_error());
  }

  // Edge cases: exact multiples of PI and PI/2
  {
    // sin(0) = 0, cos(0) = 1
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sin(TypeParam(0))), 0.0,
                this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::cos(TypeParam(0))), 1.0,
                this->get_trigonometric_sin_cos_abs_error());

    // sin(PI/2) = 1, cos(PI/2) = 0
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sin(TypeParam::pi_2())), 1.0,
                this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::cos(TypeParam::pi_2())), 0.0,
                this->get_trigonometric_sin_cos_abs_error());

    // sin(PI) = 0, cos(PI) = -1
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sin(TypeParam::pi())), 0.0,
                this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::cos(TypeParam::pi())), -1.0,
                this->get_trigonometric_sin_cos_abs_error());

    // sin(2*PI) = 0, cos(2*PI) = 1
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sin(TypeParam::two_pi())), 0.0,
                this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::cos(TypeParam::two_pi())), 1.0,
                this->get_trigonometric_sin_cos_abs_error());

    // Negative angles
    // sin(-PI/2) = -1, cos(-PI/2) = 0
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sin(-TypeParam::pi_2())), -1.0,
                this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::cos(-TypeParam::pi_2())), 0.0,
                this->get_trigonometric_sin_cos_abs_error());
  }

  // Large angles (multiple rotations)
  {
    const auto large_angle = TypeParam(10.0) * TypeParam::two_pi(); // 10 full rotations
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sin(large_angle)), std::sin(10.0 * 2.0 * M_PI),
                this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::cos(large_angle)), std::cos(10.0 * 2.0 * M_PI),
                this->get_trigonometric_sin_cos_abs_error());
  }
}

TYPED_TEST(SignedFixedPointMathTest, asin_acos)
{
  for (double value = -1.0; value <= 1.0; value += 0.01)
  {
    const auto result_asin = rtw::multiprecision::math::asin(TypeParam(value));
    const auto expected_asin = std::asin(value);
    EXPECT_NEAR(static_cast<double>(result_asin), expected_asin, this->get_trigonometric_asin_acos_abs_error(value));

    const auto result_acos = rtw::multiprecision::math::acos(TypeParam(value));
    const auto expected_acos = std::acos(value);
    EXPECT_NEAR(static_cast<double>(result_acos), expected_acos, this->get_trigonometric_asin_acos_abs_error(value));
  }
  {
    EXPECT_DEATH(rtw::multiprecision::math::asin(TypeParam(1.1)), "");
    EXPECT_DEATH(rtw::multiprecision::math::asin(TypeParam(-1.1)), "");
    EXPECT_DEATH(rtw::multiprecision::math::acos(TypeParam(1.1)), "");
    EXPECT_DEATH(rtw::multiprecision::math::acos(TypeParam(-1.1)), "");
  }
}

TYPED_TEST(SignedFixedPointMathTest, tan)
{
  for (std::int16_t deg = -this->get_tan_angle_range_deg(); deg <= this->get_tan_angle_range_deg(); ++deg)
  {
    const auto rad = deg * rtw::math_constants::DEG_TO_RAD<double>;
    const auto result_tan = rtw::multiprecision::math::tan(TypeParam(rad));
    const auto expected_tan = std::tan(rad);
    EXPECT_NEAR(static_cast<double>(result_tan), expected_tan, this->get_trigonometric_tan_abs_error(deg));
  }
  {
    EXPECT_DEATH(rtw::multiprecision::math::tan(TypeParam::pi_2()), "");
    EXPECT_DEATH(rtw::multiprecision::math::tan(-TypeParam::pi_2()), "");
  }
}

TYPED_TEST(SignedFixedPointMathTest, atan)
{
  // Test |x| <= 1 (uses Taylor series directly or PI/4 reduction)
  for (double value = -1.0; value <= 1.0; value += 0.01)
  {
    const auto result_atan = rtw::multiprecision::math::atan(TypeParam(value));
    const auto expected_atan = std::atan(value);
    EXPECT_NEAR(static_cast<double>(result_atan), expected_atan, this->get_trigonometric_sin_cos_abs_error());
  }

  // Test |x| > 1 (uses PI/2 reduction: atan(x) = sign(x) * (PI/2 - atan(1/|x|)))
  // This exercises the range reduction code paths for large values
  for (double value = 1.1; value <= 10.0; value += 0.1)
  {
    // Positive values
    {
      const auto result_atan = rtw::multiprecision::math::atan(TypeParam(value));
      const auto expected_atan = std::atan(value);
      EXPECT_NEAR(static_cast<double>(result_atan), expected_atan, this->get_trigonometric_sin_cos_abs_error());
    }
    // Negative values
    {
      const auto result_atan = rtw::multiprecision::math::atan(TypeParam(-value));
      const auto expected_atan = std::atan(-value);
      EXPECT_NEAR(static_cast<double>(result_atan), expected_atan, this->get_trigonometric_sin_cos_abs_error());
    }
  }

  // Test specific boundary values where reciprocal 1/|x| falls in different ranges
  // When |x| = 2, 1/|x| = 0.5 (boundary between Taylor and PI/4 reduction for reciprocal)
  {
    const auto result_pos = rtw::multiprecision::math::atan(TypeParam(2.0));
    const auto result_neg = rtw::multiprecision::math::atan(TypeParam(-2.0));
    EXPECT_NEAR(static_cast<double>(result_pos), std::atan(2.0), this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(result_neg), std::atan(-2.0), this->get_trigonometric_sin_cos_abs_error());
  }

  // When |x| is very large, atan(x) approaches PI/2
  {
    const auto result_pos = rtw::multiprecision::math::atan(TypeParam(100.0));
    const auto result_neg = rtw::multiprecision::math::atan(TypeParam(-100.0));
    EXPECT_NEAR(static_cast<double>(result_pos), std::atan(100.0), this->get_trigonometric_sin_cos_abs_error());
    EXPECT_NEAR(static_cast<double>(result_neg), std::atan(-100.0), this->get_trigonometric_sin_cos_abs_error());
  }
}

TYPED_TEST(SignedFixedPointMathTest, atan2)
{
  for (double y = -1.0; y <= 1.0; y += 0.01)
  {
    for (double x = -1.0; x <= 1.0; x += 0.01)
    {
      const TypeParam fp_y(y);
      const TypeParam fp_x(x);
      const auto result_atan2 = rtw::multiprecision::math::atan2(fp_y, fp_x);
      // Compare against std::atan2 using the same (possibly quantized) values
      const auto expected_atan2 = std::atan2(static_cast<double>(fp_y), static_cast<double>(fp_x));
      EXPECT_NEAR(static_cast<double>(result_atan2), expected_atan2, this->get_trigonometric_sin_cos_abs_error());
    }
  }
}
//  -----------------------------------------------------------------------------------------------
template <typename T>
class UnsignedFixedPointMathTest : public ::testing::Test
{};

using UnsignedFixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8U, rtw::multiprecision::FixedPoint16U,
                                                 rtw::multiprecision::FixedPoint32U>;
TYPED_TEST_SUITE(UnsignedFixedPointMathTest, UnsignedFixedPointTypes, );

TYPED_TEST(UnsignedFixedPointMathTest, floor)
{
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(1.5)), 1.0);
}

TYPED_TEST(UnsignedFixedPointMathTest, ceil)
{
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(1.23)), 2.0);
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(1.5)), 2.0);
}

TYPED_TEST(UnsignedFixedPointMathTest, clamp)
{
  EXPECT_EQ(rtw::multiprecision::math::clamp(TypeParam(1.23), TypeParam(0.0), TypeParam(2.0)), 1.23);
  EXPECT_EQ(rtw::multiprecision::math::clamp(TypeParam(3.23), TypeParam(0.0), TypeParam(2.0)), 2.0);
}

TYPED_TEST(UnsignedFixedPointMathTest, round)
{
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(1.23)), 1.0);
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(1.5)), 2.0);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TYPED_TEST(UnsignedFixedPointMathTest, sqrt)
{
  const auto max = static_cast<double>(rtw::multiprecision::FixedPoint8U::max());
  // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
  for (double i = 0.0; i < max; i += rtw::multiprecision::FixedPoint8U::RESOLUTION)
  {
    const auto result = static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(i)));
    const auto expected = std::sqrt(i);
    EXPECT_NEAR(result, expected, TypeParam::RESOLUTION);
  }

  EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(4.0))), std::sqrt(4.0),
              TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(2.0))), std::sqrt(2.0),
              TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(42.0))), std::sqrt(42.0),
              TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(81.0))), std::sqrt(81.0),
              TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(rtw::multiprecision::math::sqrt(TypeParam(72.0))), std::sqrt(72.0),
              TypeParam::RESOLUTION);
  EXPECT_EQ(rtw::multiprecision::math::sqrt(TypeParam(0.0)), std::sqrt(0.0));
}
