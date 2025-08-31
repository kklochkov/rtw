#include "multiprecision/format.h" // IWYU pragma: keep
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
