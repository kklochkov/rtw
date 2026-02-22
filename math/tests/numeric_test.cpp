#include "math/numeric.h"

#include <gtest/gtest.h>

#include <limits>

TEST(Numeric, is_near_zero_zero_value)
{
  EXPECT_TRUE(rtw::math::is_near_zero(0.0F));
  EXPECT_TRUE(rtw::math::is_near_zero(0.0));
  EXPECT_TRUE(rtw::math::is_near_zero(-0.0F));
  EXPECT_TRUE(rtw::math::is_near_zero(-0.0));
}

TEST(Numeric, is_near_zero_within_epsilon)
{
  constexpr auto EPS_F = std::numeric_limits<float>::epsilon();
  constexpr auto EPS_D = std::numeric_limits<double>::epsilon();

  EXPECT_TRUE(rtw::math::is_near_zero(EPS_F));
  EXPECT_TRUE(rtw::math::is_near_zero(EPS_D));
  EXPECT_TRUE(rtw::math::is_near_zero(-EPS_F));
  EXPECT_TRUE(rtw::math::is_near_zero(-EPS_D));
  EXPECT_TRUE(rtw::math::is_near_zero(EPS_F / 2.0F));
  EXPECT_TRUE(rtw::math::is_near_zero(EPS_D / 2.0));
}

TEST(Numeric, is_near_zero_outside_epsilon)
{
  constexpr auto EPS_F = std::numeric_limits<float>::epsilon();
  constexpr auto EPS_D = std::numeric_limits<double>::epsilon();

  EXPECT_FALSE(rtw::math::is_near_zero(EPS_F * 2.0F));
  EXPECT_FALSE(rtw::math::is_near_zero(EPS_D * 2.0));
  EXPECT_FALSE(rtw::math::is_near_zero(-EPS_F * 2.0F));
  EXPECT_FALSE(rtw::math::is_near_zero(-EPS_D * 2.0));
  EXPECT_FALSE(rtw::math::is_near_zero(1.0F));
  EXPECT_FALSE(rtw::math::is_near_zero(1.0));
}

TEST(Numeric, is_near_zero_with_custom_epsilon)
{
  EXPECT_TRUE(rtw::math::is_near_zero(0.5F, 1.0F));
  EXPECT_TRUE(rtw::math::is_near_zero(0.5, 1.0));
  EXPECT_TRUE(rtw::math::is_near_zero(-0.5F, 1.0F));
  EXPECT_TRUE(rtw::math::is_near_zero(-0.5, 1.0));
  EXPECT_FALSE(rtw::math::is_near_zero(1.5F, 1.0F));
  EXPECT_FALSE(rtw::math::is_near_zero(1.5, 1.0));
}

TEST(Numeric, is_near_zero_fixed_point)
{
  // For fixed-point, default epsilon is 0, so only exactly 0 is near zero
  EXPECT_TRUE(rtw::math::is_near_zero(rtw::multiprecision::FixedPoint16{0}));
  EXPECT_TRUE(rtw::math::is_near_zero(rtw::multiprecision::FixedPoint32{0}));

  // Non-zero values should not be near zero with default epsilon=0
  EXPECT_FALSE(rtw::math::is_near_zero(rtw::multiprecision::FixedPoint16{1}));
  EXPECT_FALSE(rtw::math::is_near_zero(rtw::multiprecision::FixedPoint32{1}));
}

TEST(Numeric, is_near_zero_fixed_point_with_custom_epsilon)
{
  using Q16 = rtw::multiprecision::FixedPoint16;
  using Q32 = rtw::multiprecision::FixedPoint32;

  EXPECT_TRUE(rtw::math::is_near_zero(Q16{0.5F}, Q16{1.0F}));
  EXPECT_TRUE(rtw::math::is_near_zero(Q32{0.5F}, Q32{1.0F}));
  EXPECT_FALSE(rtw::math::is_near_zero(Q16{1.5F}, Q16{1.0F}));
  EXPECT_FALSE(rtw::math::is_near_zero(Q32{1.5F}, Q32{1.0F}));
}

TEST(Numeric, is_near_equal_same_value)
{
  EXPECT_TRUE(rtw::math::is_near_equal(1.0F, 1.0F));
  EXPECT_TRUE(rtw::math::is_near_equal(1.0, 1.0));
  EXPECT_TRUE(rtw::math::is_near_equal(-1.0F, -1.0F));
  EXPECT_TRUE(rtw::math::is_near_equal(-1.0, -1.0));
  EXPECT_TRUE(rtw::math::is_near_equal(0.0F, 0.0F));
  EXPECT_TRUE(rtw::math::is_near_equal(0.0, 0.0));
}

TEST(Numeric, is_near_equal_within_tolerance)
{
  constexpr auto TOL_F = rtw::math::default_tolerance<float>();
  constexpr auto TOL_D = rtw::math::default_tolerance<double>();

  // Values differing by less than tolerance should be near equal
  EXPECT_TRUE(rtw::math::is_near_equal(1.0F, 1.0F + (TOL_F / 2.0F)));
  EXPECT_TRUE(rtw::math::is_near_equal(1.0, 1.0 + (TOL_D / 2.0)));
  EXPECT_TRUE(rtw::math::is_near_equal(1.0F, 1.0F - (TOL_F / 2.0F)));
  EXPECT_TRUE(rtw::math::is_near_equal(1.0, 1.0 - (TOL_D / 2.0)));
}

TEST(Numeric, is_near_equal_at_tolerance_boundary)
{
  constexpr auto TOL_F = rtw::math::default_tolerance<float>();
  constexpr auto TOL_D = rtw::math::default_tolerance<double>();

  // Values differing by exactly tolerance should be near equal (using <=)
  EXPECT_TRUE(rtw::math::is_near_equal(1.0F, 1.0F + TOL_F));
  EXPECT_TRUE(rtw::math::is_near_equal(1.0, 1.0 + TOL_D));
}

TEST(Numeric, is_near_equal_outside_tolerance)
{
  constexpr auto TOL_F = rtw::math::default_tolerance<float>();
  constexpr auto TOL_D = rtw::math::default_tolerance<double>();

  // Values differing by more than tolerance should not be near equal
  EXPECT_FALSE(rtw::math::is_near_equal(1.0F, 1.0F + (TOL_F * 2.0F)));
  EXPECT_FALSE(rtw::math::is_near_equal(1.0, 1.0 + (TOL_D * 2.0)));
  EXPECT_FALSE(rtw::math::is_near_equal(1.0F, 2.0F));
  EXPECT_FALSE(rtw::math::is_near_equal(1.0, 2.0));
}

TEST(Numeric, is_near_equal_with_custom_epsilon)
{
  EXPECT_TRUE(rtw::math::is_near_equal(1.0F, 1.5F, 1.0F));
  EXPECT_TRUE(rtw::math::is_near_equal(1.0, 1.5, 1.0));
  EXPECT_FALSE(rtw::math::is_near_equal(1.0F, 3.0F, 1.0F));
  EXPECT_FALSE(rtw::math::is_near_equal(1.0, 3.0, 1.0));
}

TEST(Numeric, is_near_equal_fixed_point)
{
  using Q16 = rtw::multiprecision::FixedPoint16;
  using Q32 = rtw::multiprecision::FixedPoint32;

  // Same values should be near equal
  EXPECT_TRUE(rtw::math::is_near_equal(Q16{1.0F}, Q16{1.0F}));
  EXPECT_TRUE(rtw::math::is_near_equal(Q32{1.0F}, Q32{1.0F}));

  // Values within tolerance (1 * epsilon for fixed-point)
  const auto tol_q16 = rtw::math::default_tolerance<Q16>();
  const auto tol_q32 = rtw::math::default_tolerance<Q32>();

  EXPECT_TRUE(rtw::math::is_near_equal(Q16{1.0F}, Q16{1.0F} + tol_q16));
  EXPECT_TRUE(rtw::math::is_near_equal(Q32{1.0F}, Q32{1.0F} + tol_q32));
}

TEST(Numeric, is_near_equal_fixed_point_with_custom_epsilon)
{
  using Q16 = rtw::multiprecision::FixedPoint16;
  using Q32 = rtw::multiprecision::FixedPoint32;

  EXPECT_TRUE(rtw::math::is_near_equal(Q16{1.0F}, Q16{1.5F}, Q16{1.0F}));
  EXPECT_TRUE(rtw::math::is_near_equal(Q32{1.0F}, Q32{1.5F}, Q32{1.0F}));
  EXPECT_FALSE(rtw::math::is_near_equal(Q16{1.0F}, Q16{3.0F}, Q16{1.0F}));
  EXPECT_FALSE(rtw::math::is_near_equal(Q32{1.0F}, Q32{3.0F}, Q32{1.0F}));
}
