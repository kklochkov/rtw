#include "multiprecision/format.h" // IWYU pragma: keep
#include "multiprecision/math.h"
#include "multiprecision/rational.h"

#include <gtest/gtest.h>

template <typename T>
class RationalTest : public ::testing::Test
{};

using RationalTypes = ::testing::Types<rtw::multiprecision::Rational32, rtw::multiprecision::Rational64>;
TYPED_TEST_SUITE(RationalTest, RationalTypes, );

TYPED_TEST(RationalTest, construct)
{
  {
    const TypeParam r(3, 4);
    EXPECT_EQ(r.numerator(), 3);
    EXPECT_EQ(r.denominator(), 4);
  }
  {
    const TypeParam r(-6, -8);
    EXPECT_EQ(r.numerator(), 3);
    EXPECT_EQ(r.denominator(), 4);
  }
  {
    const TypeParam r(6, -8);
    EXPECT_EQ(r.numerator(), -3);
    EXPECT_EQ(r.denominator(), 4);
  }
  {
    const TypeParam r(0, 5);
    EXPECT_EQ(r.numerator(), 0);
    EXPECT_EQ(r.denominator(), 1);
  }
  {
    const TypeParam r(5);
    EXPECT_EQ(r.numerator(), 5);
    EXPECT_EQ(r.denominator(), 1);
  }
  {
    const TypeParam r(-5);
    EXPECT_EQ(r.numerator(), -5);
    EXPECT_EQ(r.denominator(), 1);
  }
  {
    const TypeParam r(0.75F);
    EXPECT_EQ(r.numerator(), 3);
    EXPECT_EQ(r.denominator(), 4);
  }
  {
    const TypeParam r(-0.75F);
    EXPECT_EQ(r.numerator(), -3);
    EXPECT_EQ(r.denominator(), 4);
  }
}

TYPED_TEST(RationalTest, to_floating_point)
{
  {
    const TypeParam r(3, 4);
    const auto value = static_cast<float>(r);
    EXPECT_NEAR(value, 0.75F, 1e-6F);
  }
  {
    const TypeParam r(-3, 4);
    const auto value = static_cast<double>(r);
    EXPECT_NEAR(value, -0.75, 1e-12);
  }
  {
    const TypeParam r(3, -4);
    const auto value = static_cast<long double>(r);
    EXPECT_NEAR(value, -0.75L, 1e-12L);
  }
}

TYPED_TEST(RationalTest, add)
{
  {
    TypeParam r1(1, 2);
    const TypeParam r2(1, 3);
    r1 += r2;
    EXPECT_EQ(r1.numerator(), 5);
    EXPECT_EQ(r1.denominator(), 6);
  }
  {
    TypeParam r1(-1, 2);
    const TypeParam r2(1, 3);
    r1 += r2;
    EXPECT_EQ(r1.numerator(), -1);
    EXPECT_EQ(r1.denominator(), 6);
  }
  {
    TypeParam r1(1, 2);
    const TypeParam r2(-1, 3);
    r1 += r2;
    EXPECT_EQ(r1.numerator(), 1);
    EXPECT_EQ(r1.denominator(), 6);
  }
  {
    TypeParam r1(-1, 2);
    const TypeParam r2(-1, 3);
    r1 += r2;
    EXPECT_EQ(r1.numerator(), -5);
    EXPECT_EQ(r1.denominator(), 6);
  }
}

TYPED_TEST(RationalTest, subtract)
{
  {
    TypeParam r1(1, 2);
    const TypeParam r2(1, 3);
    r1 -= r2;
    EXPECT_EQ(r1.numerator(), 1);
    EXPECT_EQ(r1.denominator(), 6);
  }
  {
    TypeParam r1(-1, 2);
    const TypeParam r2(1, 3);
    r1 -= r2;
    EXPECT_EQ(r1.numerator(), -5);
    EXPECT_EQ(r1.denominator(), 6);
  }
  {
    TypeParam r1(1, 2);
    const TypeParam r2(-1, 3);
    r1 -= r2;
    EXPECT_EQ(r1.numerator(), 5);
    EXPECT_EQ(r1.denominator(), 6);
  }
  {
    TypeParam r1(-1, 2);
    const TypeParam r2(-1, 3);
    r1 -= r2;
    EXPECT_EQ(r1.numerator(), -1);
    EXPECT_EQ(r1.denominator(), 6);
  }
}

TYPED_TEST(RationalTest, multiply)
{
  {
    TypeParam r1(2, 3);
    const TypeParam r2(3, 4);
    r1 *= r2;
    EXPECT_EQ(r1.numerator(), 1);
    EXPECT_EQ(r1.denominator(), 2);
  }
  {
    TypeParam r1(-2, 3);
    const TypeParam r2(3, 4);
    r1 *= r2;
    EXPECT_EQ(r1.numerator(), -1);
    EXPECT_EQ(r1.denominator(), 2);
  }
  {
    TypeParam r1(2, 3);
    const TypeParam r2(-3, 4);
    r1 *= r2;
    EXPECT_EQ(r1.numerator(), -1);
    EXPECT_EQ(r1.denominator(), 2);
  }
  {
    TypeParam r1(-2, 3);
    const TypeParam r2(-3, 4);
    r1 *= r2;
    EXPECT_EQ(r1.numerator(), 1);
    EXPECT_EQ(r1.denominator(), 2);
  }
}

TYPED_TEST(RationalTest, divide)
{
  {
    TypeParam r1(2, 3);
    const TypeParam r2(3, 4);
    r1 /= r2;
    EXPECT_EQ(r1.numerator(), 8);
    EXPECT_EQ(r1.denominator(), 9);
  }
  {
    TypeParam r1(-2, 3);
    const TypeParam r2(3, 4);
    r1 /= r2;
    EXPECT_EQ(r1.numerator(), -8);
    EXPECT_EQ(r1.denominator(), 9);
  }
  {
    TypeParam r1(2, 3);
    const TypeParam r2(-3, 4);
    r1 /= r2;
    EXPECT_EQ(r1.numerator(), -8);
    EXPECT_EQ(r1.denominator(), 9);
  }
  {
    TypeParam r1(-2, 3);
    const TypeParam r2(-3, 4);
    r1 /= r2;
    EXPECT_EQ(r1.numerator(), 8);
    EXPECT_EQ(r1.denominator(), 9);
  }
}

TYPED_TEST(RationalTest, compare)
{
  {
    const TypeParam r1(1, 2);
    const TypeParam r2(2, 4);
    EXPECT_TRUE(r1 == r2);
    EXPECT_FALSE(r1 != r2);
    EXPECT_FALSE(r1 < r2);
    EXPECT_FALSE(r1 > r2);
    EXPECT_TRUE(r1 <= r2);
    EXPECT_TRUE(r1 >= r2);
  }
  {
    const TypeParam r1(1, 3);
    const TypeParam r2(1, 2);
    EXPECT_FALSE(r1 == r2);
    EXPECT_TRUE(r1 != r2);
    EXPECT_TRUE(r1 < r2);
    EXPECT_FALSE(r1 > r2);
    EXPECT_TRUE(r1 <= r2);
    EXPECT_FALSE(r1 >= r2);
  }
  {
    const TypeParam r1(3, 4);
    const TypeParam r2(2, 4);
    EXPECT_FALSE(r1 == r2);
    EXPECT_TRUE(r1 != r2);
    EXPECT_FALSE(r1 < r2);
    EXPECT_TRUE(r1 > r2);
    EXPECT_FALSE(r1 <= r2);
    EXPECT_TRUE(r1 >= r2);
  }
}

TYPED_TEST(RationalTest, format)
{
  {
    const TypeParam r(3, 4);
    const auto str = fmt::format("{}", r);
    EXPECT_EQ(str, "3/4");
  }
  {
    const TypeParam r(-3, 4);
    const auto str = fmt::format("{}", r);
    EXPECT_EQ(str, "-3/4");
  }
  {
    const TypeParam r(3, -4);
    const auto str = fmt::format("{}", r);
    EXPECT_EQ(str, "-3/4");
  }
  {
    const TypeParam r(5);
    const auto str = fmt::format("{}", r);
    EXPECT_EQ(str, "5");
  }
  {
    const TypeParam r(0);
    const auto str = fmt::format("{}", r);
    EXPECT_EQ(str, "0");
  }
}

TYPED_TEST(RationalTest, abs)
{
  EXPECT_EQ(rtw::multiprecision::math::abs(TypeParam(3, 4)), TypeParam(3, 4));
  EXPECT_EQ(rtw::multiprecision::math::abs(TypeParam(-3, 4)), TypeParam(3, 4));
  EXPECT_EQ(rtw::multiprecision::math::abs(TypeParam(3, -4)), TypeParam(3, 4));
  EXPECT_EQ(rtw::multiprecision::math::abs(TypeParam(0)), TypeParam(0));
}

TYPED_TEST(RationalTest, floor)
{
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(7, 4)), TypeParam(1)); // 1.75 -> 1
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(8, 4)), TypeParam(2)); // 2.0 -> 2
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(1, 4)), TypeParam(0)); // 0.25 -> 0

  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(-7, 4)), TypeParam(-2)); // -1.75 -> -2
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(-8, 4)), TypeParam(-2)); // -2.0 -> -2
  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(-1, 4)), TypeParam(-1)); // -0.25 -> -1

  EXPECT_EQ(rtw::multiprecision::math::floor(TypeParam(0)), TypeParam(0));
}

TYPED_TEST(RationalTest, ceil)
{
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(7, 4)), TypeParam(2)); // 1.75 -> 2
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(8, 4)), TypeParam(2)); // 2.0 -> 2
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(1, 4)), TypeParam(1)); // 0.25 -> 1

  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(-7, 4)), TypeParam(-1)); // -1.75 -> -1
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(-8, 4)), TypeParam(-2)); // -2.0 -> -2
  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(-1, 4)), TypeParam(0));  // -0.25 -> 0

  EXPECT_EQ(rtw::multiprecision::math::ceil(TypeParam(0)), TypeParam(0));
}

TYPED_TEST(RationalTest, round)
{
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(7, 4)), TypeParam(2)); // 1.75 -> 2
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(5, 4)), TypeParam(1)); // 1.25 -> 1
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(3, 2)), TypeParam(2)); // 1.5 -> 2 (half away from zero)
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(1, 4)), TypeParam(0)); // 0.25 -> 0

  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-7, 4)), TypeParam(-2)); // -1.75 -> -2
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-5, 4)), TypeParam(-1)); // -1.25 -> -1
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-3, 2)), TypeParam(-2)); // -1.5 -> -2 (half away from zero)
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-1, 4)), TypeParam(0));  // -0.25 -> 0

  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(5)), TypeParam(5));
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(-5)), TypeParam(-5));
  EXPECT_EQ(rtw::multiprecision::math::round(TypeParam(0)), TypeParam(0));
}

TYPED_TEST(RationalTest, trunc)
{
  EXPECT_EQ(rtw::multiprecision::math::trunc(TypeParam(7, 4)), TypeParam(1)); // 1.75 -> 1
  EXPECT_EQ(rtw::multiprecision::math::trunc(TypeParam(8, 4)), TypeParam(2)); // 2.0 -> 2

  EXPECT_EQ(rtw::multiprecision::math::trunc(TypeParam(-7, 4)), TypeParam(-1)); // -1.75 -> -1
  EXPECT_EQ(rtw::multiprecision::math::trunc(TypeParam(-8, 4)), TypeParam(-2)); // -2.0 -> -2

  EXPECT_EQ(rtw::multiprecision::math::trunc(TypeParam(0)), TypeParam(0));
}

TYPED_TEST(RationalTest, pow)
{
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2, 3), 0), TypeParam(1));
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(0), 0), TypeParam(1)); // 0^0 = 1 by convention

  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2, 3), 1), TypeParam(2, 3));

  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2, 3), 2), TypeParam(4, 9));  // (2/3)^2 = 4/9
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2, 3), 3), TypeParam(8, 27)); // (2/3)^3 = 8/27
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(3), 3), TypeParam(27));       // 3^3 = 27

  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2, 3), -1), TypeParam(3, 2)); // (2/3)^-1 = 3/2
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2, 3), -2), TypeParam(9, 4)); // (2/3)^-2 = 9/4
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(2), -3), TypeParam(1, 8));    // 2^-3 = 1/8

  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(-2, 3), 2), TypeParam(4, 9));   // (-2/3)^2 = 4/9
  EXPECT_EQ(rtw::multiprecision::math::pow(TypeParam(-2, 3), 3), TypeParam(-8, 27)); // (-2/3)^3 = -8/27
}
