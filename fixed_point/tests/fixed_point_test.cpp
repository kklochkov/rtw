#include "fixed_point/fixed_point.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(FixedPoint8, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint8::type>::min()
                    == rtw::fixed_point::FixedPoint8::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint8::type>::max()
                    == rtw::fixed_point::FixedPoint8::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::BITS, 16);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::FRACTIONAL_BITS, 8);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::INTEGER_BITS, 7);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::ONE, 256);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::RESOLUTION, 0.00390625);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::min(), -128.0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::max(), 127.99609375);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::MIN, -128);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8::MAX, 127);
}

TEST(FixedPoint8u, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint8u::type>::min()
                    == rtw::fixed_point::FixedPoint8u::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint8u::type>::max()
                    == rtw::fixed_point::FixedPoint8u::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::BITS, 16);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::FRACTIONAL_BITS, 8);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::INTEGER_BITS, 8);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::ONE, 256);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::RESOLUTION, 0.00390625);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::min(), 0.0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::max(), 255.99609375);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::MIN, 0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint8u::MAX, 255);
}

TEST(FixedPoint16, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint16::type>::min()
                    == rtw::fixed_point::FixedPoint16::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint16::type>::max()
                    == rtw::fixed_point::FixedPoint16::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::BITS, 32);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::FRACTIONAL_BITS, 16);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::INTEGER_BITS, 15);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::ONE, 65'536);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::RESOLUTION, 0.0000152587890625);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::min(), -32768.0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::max(), 32767.999984741211);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::MIN, -32'768);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16::MAX, 32'767);
}

TEST(FixedPoint16u, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint16u::type>::min()
                    == rtw::fixed_point::FixedPoint16u::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint16u::type>::max()
                    == rtw::fixed_point::FixedPoint16u::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::BITS, 32);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::FRACTIONAL_BITS, 16);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::INTEGER_BITS, 16);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::ONE, 65'536);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::RESOLUTION, 0.0000152587890625);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::min(), 0.0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::max(), 65535.999984741211);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::MIN, 0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint16u::MAX, 65'535);
}

TEST(FixedPoint32, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint32::type>::min()
                    == rtw::fixed_point::FixedPoint32::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint32::type>::max()
                    == rtw::fixed_point::FixedPoint32::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::BITS, 64);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::FRACTIONAL_BITS, 32);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::INTEGER_BITS, 31);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::ONE, 4'294'967'296);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::RESOLUTION, 0.00000000023283064365386962890625);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::min(), -2147483648);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::max(), 2147483648);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::MIN, -2'147'483'648);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32::MAX, 2'147'483'647);
}

TEST(FixedPoint32u, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint32u::type>::min()
                    == rtw::fixed_point::FixedPoint32u::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::FixedPoint32u::type>::max()
                    == rtw::fixed_point::FixedPoint32u::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::BITS, 64);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::FRACTIONAL_BITS, 32);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::INTEGER_BITS, 32);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::ONE, 4'294'967'296);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::RESOLUTION, 0.00000000023283064365386962890625);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::min(), 0.0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::max(), 4294967296.0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::MIN, 0);
  EXPECT_EQ(rtw::fixed_point::FixedPoint32u::MAX, 4'294'967'295);
}
// -----------------------------------------------------------------------------------------------
template <typename T>
class FixedPointTest : public ::testing::Test
{};

using FixedPointTypes =
    ::testing::Types<rtw::fixed_point::FixedPoint8, rtw::fixed_point::FixedPoint8u, rtw::fixed_point::FixedPoint16,
                     rtw::fixed_point::FixedPoint16u, rtw::fixed_point::FixedPoint32, rtw::fixed_point::FixedPoint32u>;
TYPED_TEST_SUITE(FixedPointTest, FixedPointTypes, );

TYPED_TEST(FixedPointTest, traits)
{
  static_assert(std::is_trivially_default_constructible_v<TypeParam>);
  static_assert(std::is_nothrow_default_constructible_v<TypeParam>);
  static_assert(std::is_trivially_copy_constructible_v<TypeParam>);
  static_assert(std::is_trivially_copy_assignable_v<TypeParam>);
  static_assert(std::is_trivially_destructible_v<TypeParam>);
  static_assert(std::is_nothrow_move_constructible_v<TypeParam>);
  static_assert(std::is_nothrow_move_assignable_v<TypeParam>);
  static_assert(std::is_nothrow_swappable_v<TypeParam>);
}

TYPED_TEST(FixedPointTest, ctor_default)
{
  constexpr TypeParam FP{};
  EXPECT_EQ(static_cast<float>(FP), 0.0F);
  EXPECT_EQ(static_cast<double>(FP), 0.0);
  EXPECT_EQ(static_cast<std::int32_t>(FP), 0);
}

TYPED_TEST(FixedPointTest, comparison)
{
  {
    const TypeParam a(1.23);
    const TypeParam b(2.34);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
  }
  {
    const TypeParam a(1.23);
    const TypeParam b(1.23);
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a == b);
  }
  {
    const TypeParam a(2.34);
    const TypeParam b(1.23);
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a <= b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
  }
  {
    const TypeParam a(1.23);
    const double b(2.34);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
  }
  {
    const TypeParam a(1.23);
    const double b(1.23);
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a == b);
  }
  {
    const TypeParam a(2.34);
    const double b(1.23);
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a <= b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
  }
  {
    const double a(1.23);
    const TypeParam b(2.34);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
  }
  {
    const double a(1.23);
    const TypeParam b(1.23);
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a == b);
  }
  {
    const double a(2.34);
    const TypeParam b(1.23);
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a <= b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
  }
  {
    const double a(0.0);
    const TypeParam b(0.0);
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a == b);
  }
  {
    const TypeParam a(29);
    const TypeParam b(19);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a <= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
    EXPECT_EQ(static_cast<std::int32_t>(a), 29);
    EXPECT_EQ(static_cast<std::uint32_t>(b), 19);
  }
}
// -----------------------------------------------------------------------------------------------
template <typename T>
class SignedFixedPointTest : public ::testing::Test
{
public:
  constexpr static std::array<float, 10> CTOR_TEST_EXPECTED_FLOAT = {-128.0F, -35.56F, -7.23F, -1.0F,  0.0F,
                                                                     1.0F,    7.23F,   35.56F, 127.0F, 0.0F};
  constexpr static std::array<double, 10> CTOR_TEST_EXPECTED_DOUBLE = {-128.0, -35.56, -7.23, -1.0,  0.0,
                                                                       1.0,    7.23,   35.56, 127.0, 0.0};
  constexpr static std::array<std::int32_t, 10> CTOR_TEST_EXPECTED_INT32 = {-128, -36, -7, -1, 0, 1, 7, 29, 36, 127};
  constexpr static std::array<std::uint32_t, 10> CTOR_TEST_EXPECTED_UINT32 = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
};

using SignedFixedPointTypes =
    ::testing::Types<rtw::fixed_point::FixedPoint8, rtw::fixed_point::FixedPoint16, rtw::fixed_point::FixedPoint32>;
TYPED_TEST_SUITE(SignedFixedPointTest, SignedFixedPointTypes, );

TYPED_TEST(SignedFixedPointTest, ctor)
{
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_FLOAT)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_DOUBLE)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_INT32)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<std::int32_t>(fp), e);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_UINT32)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<std::uint32_t>(fp), e);
  }
}

TYPED_TEST(SignedFixedPointTest, arithmetic)
{
  {
    const TypeParam a(0.1);
    const TypeParam b(0.2);
    const TypeParam c = a + b;
    EXPECT_NEAR(static_cast<float>(c), 0.1F + 0.2F, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(c), 0.1 + 0.2, TypeParam::RESOLUTION);
  }
  {
    const TypeParam a(1.5);
    const TypeParam b(2.3);
    const TypeParam c = a - b;
    EXPECT_EQ(c, -0.8);
    EXPECT_NEAR(static_cast<double>(c), 1.5 - 2.3, TypeParam::RESOLUTION);
  }
  {
    const TypeParam a(1.5);
    const TypeParam b(2.25);
    const TypeParam c = a * b;
    EXPECT_NEAR(static_cast<float>(c), 1.5F * 2.25F, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(c), 1.5 * 2.25, TypeParam::RESOLUTION);
  }
  {
    const TypeParam a(1.0);
    const TypeParam b(2.0);
    const TypeParam c = a / b;
    EXPECT_NEAR(static_cast<float>(c), 1.0F / 2.0F, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(c), 1.0 / 2.0, TypeParam::RESOLUTION);
  }
}

TYPED_TEST(SignedFixedPointTest, arithmetic_saturate)
{
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(1.0);
    const TypeParam c = a + b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(TypeParam::min());
    const TypeParam b(-1.0);
    const TypeParam c = a + b;
    EXPECT_EQ(c, TypeParam::min());
  }
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(-1.0);
    const TypeParam c = a - b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(TypeParam::min());
    const TypeParam b(1.0);
    const TypeParam c = a - b;
    EXPECT_EQ(c, TypeParam::min());
  }
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(2.0);
    const TypeParam c = a * b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(TypeParam::min());
    const TypeParam b(2.0);
    const TypeParam c = a * b;
    EXPECT_EQ(c, TypeParam::min());
  }
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(TypeParam::RESOLUTION);
    const TypeParam c = a / b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(TypeParam::min());
    const TypeParam b(TypeParam::RESOLUTION);
    const TypeParam c = a / b;
    EXPECT_EQ(c, TypeParam::min());
  }
}

TYPED_TEST(SignedFixedPointTest, unary_minus)
{
  {
    const TypeParam a(1.23);
    const TypeParam b = -a;
    EXPECT_EQ(b, -1.23);
  }
  {
    const TypeParam a(-1.23);
    const TypeParam b = -a;
    EXPECT_EQ(b, 1.23);
  }
}

TYPED_TEST(SignedFixedPointTest, assignment)
{
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a += b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 + 2.34, TypeParam::RESOLUTION);
  }
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a -= b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 - 2.34, TypeParam::RESOLUTION);
  }
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a *= b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 * 2.34, TypeParam::RESOLUTION);
  }
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a /= b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 / 2.34, TypeParam::RESOLUTION);
  }
}
// -----------------------------------------------------------------------------------------------
template <typename T>
class UnsignedFixedPointTest : public ::testing::Test
{
public:
  constexpr static std::array<float, 10> CTOR_TEST_EXPECTED_FLOAT = {0.0F,   1.0F,   7.23F,  7.89F,  8.0F,
                                                                     11.29F, 12.19F, 30.06F, 84.17F, 127.0F};
  constexpr static std::array<double, 10> CTOR_TEST_EXPECTED_DOUBLE = {0.0,   1.0,   7.23,  7.89,  8.0,
                                                                       11.29, 12.19, 30.06, 84.17, 127.0};
  constexpr static std::array<std::uint32_t, 10> CTOR_TEST_EXPECTED_UINT32 = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
};

using UnsignedFixedPointTypes =
    ::testing::Types<rtw::fixed_point::FixedPoint8u, rtw::fixed_point::FixedPoint16u, rtw::fixed_point::FixedPoint32u>;
TYPED_TEST_SUITE(UnsignedFixedPointTest, UnsignedFixedPointTypes, );

TYPED_TEST(UnsignedFixedPointTest, ctor)
{
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_FLOAT)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_DOUBLE)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_UINT32)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<std::uint32_t>(fp), e);
  }
}

TYPED_TEST(UnsignedFixedPointTest, arithmetic)
{
  {
    const TypeParam a(0.1);
    const TypeParam b(0.2);
    const TypeParam c = a + b;
    EXPECT_EQ(c, 0.3);
    EXPECT_NEAR(static_cast<float>(c), 0.1F + 0.2F, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(c), 0.1 + 0.2, TypeParam::RESOLUTION);
  }
  {
    const TypeParam a(2.3);
    const TypeParam b(1.5);
    const TypeParam c = a - b;
    EXPECT_NEAR(static_cast<double>(c), 2.3 - 1.5, TypeParam::RESOLUTION);
  }
  {
    const TypeParam a(1.5);
    const TypeParam b(2.25);
    const TypeParam c = a * b;
    EXPECT_NEAR(static_cast<float>(c), 1.5F * 2.25F, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(c), 1.5 * 2.25, TypeParam::RESOLUTION);
  }
  {
    const TypeParam a(1.0);
    const TypeParam b(2.0);
    const TypeParam c = a / b;
    EXPECT_NEAR(static_cast<float>(c), 1.0F / 2.0F, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(c), 1.0 / 2.0, TypeParam::RESOLUTION);
  }
}

TYPED_TEST(UnsignedFixedPointTest, arithmetic_saturate)
{
  {
    const TypeParam a(-1);
    EXPECT_EQ(a, TypeParam::max()); // See the comment in the implementation.
  }
  {
    const TypeParam a(-1.0);
    const TypeParam b(-1);
    EXPECT_EQ(a, b);
    EXPECT_EQ(a, TypeParam::max()); // See the comment in the implementation.
  }
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(1.0);
    const TypeParam c = a + b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(0.0);
    const TypeParam b(1.0);
    const TypeParam c = a - b;
    EXPECT_EQ(c, TypeParam::max()); // See the comment in the implementation.
  }
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(2.0);
    const TypeParam c = a * b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(0.0);
    const TypeParam b(2.0);
    const TypeParam c = a * b;
    EXPECT_EQ(c, 0.0);
  }
  {
    const TypeParam a(TypeParam::max());
    const TypeParam b(TypeParam::RESOLUTION);
    const TypeParam c = a / b;
    EXPECT_EQ(c, TypeParam::max());
  }
  {
    const TypeParam a(0.0);
    const TypeParam b(TypeParam::RESOLUTION);
    const TypeParam c = a / b;
    EXPECT_EQ(c, 0.0);
  }
}

TYPED_TEST(UnsignedFixedPointTest, assignment)
{
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a += b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 + 2.34, TypeParam::RESOLUTION);
  }
  {
    TypeParam a(2.34);
    const TypeParam b(1.23);
    a -= b;
    EXPECT_NEAR(static_cast<double>(a), 2.34 - 1.23, TypeParam::RESOLUTION);
  }
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a *= b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 * 2.34, TypeParam::RESOLUTION);
  }
  {
    TypeParam a(1.23);
    const TypeParam b(2.34);
    a /= b;
    EXPECT_NEAR(static_cast<double>(a), 1.23 / 2.34, TypeParam::RESOLUTION);
  }
}
// -----------------------------------------------------------------------------------------------
using WrapFixedPoint8 =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::FixedPoint8::type, rtw::fixed_point::FixedPoint8::FRACTIONAL_BITS,
                                 rtw::fixed_point::FixedPoint8::saturation_type,
                                 rtw::fixed_point::OverflowPolicy::WRAP>;

using WrapFixedPoint16 =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::FixedPoint16::type, rtw::fixed_point::FixedPoint16::FRACTIONAL_BITS,
                                 rtw::fixed_point::FixedPoint16::saturation_type,
                                 rtw::fixed_point::OverflowPolicy::WRAP>;

using WrapFixedPoint32 =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::FixedPoint32::type, rtw::fixed_point::FixedPoint32::FRACTIONAL_BITS,
                                 rtw::fixed_point::FixedPoint32::saturation_type,
                                 rtw::fixed_point::OverflowPolicy::WRAP>;

using WrapFixedPoint8u =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::FixedPoint8u::type, rtw::fixed_point::FixedPoint8u::FRACTIONAL_BITS,
                                 rtw::fixed_point::FixedPoint8u::saturation_type,
                                 rtw::fixed_point::OverflowPolicy::WRAP>;

using WrapFixedPoint16u = rtw::fixed_point::FixedPoint<
    rtw::fixed_point::FixedPoint16u::type, rtw::fixed_point::FixedPoint16u::FRACTIONAL_BITS,
    rtw::fixed_point::FixedPoint16u::saturation_type, rtw::fixed_point::OverflowPolicy::WRAP>;

using WrapFixedPoint32u = rtw::fixed_point::FixedPoint<
    rtw::fixed_point::FixedPoint32u::type, rtw::fixed_point::FixedPoint32u::FRACTIONAL_BITS,
    rtw::fixed_point::FixedPoint32u::saturation_type, rtw::fixed_point::OverflowPolicy::WRAP>;

template <typename T>
class WrapFixedPointTest : public ::testing::Test
{
public:
  /// All these functions trigger the wrap policy
  /// @{
  constexpr double expected_plus_max_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{127} + std::int8_t{1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{32'767} + std::int16_t{1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::max() + 1L));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{255} + std::uint8_t{1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::uint16_t{65'535} + std::uint16_t{1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(
          static_cast<std::uint32_t>(/*std::numeric_limits<std::uint32_t>::max()*/ 4'294'967'295U + 1U));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_plus_min_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{-128} + std::int8_t{-1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{-32'768} + std::int16_t{-1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::min() + -1L));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{0} + static_cast<std::uint8_t>(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::uint16_t{0} + static_cast<std::uint16_t>(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(static_cast<std::uint32_t>(std::uint32_t{0} + static_cast<std::uint32_t>(-1)));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_minus_max_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{127} - std::int8_t{-1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{32'767} - std::int16_t{-1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::max() - -1L));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{255} - static_cast<std::uint8_t>(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::uint16_t{65'535} - static_cast<std::uint16_t>(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(
          static_cast<std::uint32_t>(std::numeric_limits<std::uint32_t>::max() - static_cast<std::uint32_t>(-1)));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_minus_min_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{-128} - std::int8_t{1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{-32'768} - std::int16_t{1}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::min() - 1L));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{0} - static_cast<std::uint8_t>(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::int16_t{0} - static_cast<std::uint16_t>(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(static_cast<std::uint32_t>(std::int32_t{0} - static_cast<std::uint32_t>(1)));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_mul_min_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{127} * std::int8_t{2}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{32'767} * std::int16_t{2}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::max() * 2L));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{255} * static_cast<std::uint8_t>(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::uint16_t{65'535} * static_cast<std::uint16_t>(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(
          static_cast<std::uint32_t>(std::numeric_limits<std::uint32_t>::max() * static_cast<std::uint32_t>(2)));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_mul_max_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{-128} * std::int8_t{2}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{-32'768} * std::int16_t{2}));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::numeric_limits<std::int32_t>::min() * 2L));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{0} * static_cast<std::uint8_t>(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::uint16_t{0} * static_cast<std::uint16_t>(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(static_cast<std::uint32_t>(std::uint32_t{0} * static_cast<std::uint32_t>(2)));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_div_min_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{127} * (std::int8_t{1} / std::int8_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(static_cast<std::int16_t>(std::int16_t{32'767} * (std::int16_t{1} / std::int16_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(static_cast<std::int32_t>(std::int32_t{32'767} * (std::int32_t{1} / std::int32_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{255} * (std::uint8_t{1} / std::uint8_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(
          static_cast<std::uint16_t>(std::uint16_t{65'535} * (std::uint16_t{1} / std::uint16_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(
          static_cast<std::uint32_t>(std::uint32_t{65'535} * (std::uint32_t{1} / std::uint32_t{2})));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr double expected_div_max_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(static_cast<std::int8_t>(std::int8_t{-128} * (std::int8_t{1} / std::int8_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(
          static_cast<std::int16_t>(std::int16_t{-32'767} * (std::int16_t{1} / std::int16_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int64_t>)
    {
      return static_cast<double>(
          static_cast<std::int32_t>(std::int32_t{-32'767} * (std::int32_t{1} / std::int32_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(static_cast<std::uint8_t>(std::uint8_t{0} * (std::uint8_t{1} / std::uint8_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(static_cast<std::uint16_t>(std::uint16_t{0} * (std::uint16_t{1} / std::uint16_t{2})));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint64_t>)
    {
      return static_cast<double>(static_cast<std::uint32_t>(std::uint32_t{0} * (std::uint32_t{1} / std::uint32_t{2})));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }
  /// @}
};

using WrapFixedPointTypes = ::testing::Types<WrapFixedPoint8, WrapFixedPoint16, WrapFixedPoint32, WrapFixedPoint8u,
                                             WrapFixedPoint16u, WrapFixedPoint32u>;
TYPED_TEST_SUITE(WrapFixedPointTest, WrapFixedPointTypes, );

TYPED_TEST(WrapFixedPointTest, arithmetic_wrap)
{
  {
    const TypeParam a(static_cast<double>(TypeParam::MAX));
    const TypeParam b(1.0);
    const auto c = a + b;
    EXPECT_EQ(c, this->expected_plus_max_value());
  }
  if constexpr (std::is_signed_v<typename TypeParam::type>)
  {
    {
      const TypeParam a(static_cast<double>(TypeParam::MIN));
      const TypeParam b(-1.0);
      const auto c = a + b;
      EXPECT_EQ(c, this->expected_plus_min_value());
    }
    {
      const TypeParam a(static_cast<double>(TypeParam::MAX));
      const TypeParam b(-1.0);
      const auto c = a - b;
      EXPECT_EQ(c, this->expected_minus_max_value());
    }
  }
  {
    const TypeParam a(static_cast<double>(TypeParam::MIN));
    const TypeParam b(1.0);
    const auto c = a - b;
    EXPECT_EQ(c, this->expected_minus_min_value());
  }
  {
    const TypeParam a(static_cast<double>(TypeParam::MAX));
    const TypeParam b(2.0);
    const auto c = a * b;
    EXPECT_EQ(c, this->expected_mul_min_value());
  }
  {
    const TypeParam a(static_cast<double>(TypeParam::MIN));
    const TypeParam b(2.0);
    const auto c = a * b;
    EXPECT_EQ(c, this->expected_mul_max_value());
  }
  {
    const TypeParam a(static_cast<double>(TypeParam::MAX));
    const TypeParam b(TypeParam::RESOLUTION);
    const auto c = a / b;
    EXPECT_EQ(c, this->expected_div_min_value());
  }
  {
    const TypeParam a(static_cast<double>(TypeParam::MIN));
    const TypeParam b(TypeParam::RESOLUTION);
    const auto c = a / b;
    EXPECT_EQ(c, this->expected_div_max_value());
  }
}
// -----------------------------------------------------------------------------------------------
TEST(fixed_point, operator_stream)
{
  {
    const rtw::fixed_point::FixedPoint8 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8s(1.23047)");
  }
  {
    const rtw::fixed_point::FixedPoint16 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16s(1.23)");
  }
  {
    const rtw::fixed_point::FixedPoint32 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp32s(1.23)");
  }
  {
    const rtw::fixed_point::FixedPoint8u a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp8s(1.23047)");
  }
  {
    const rtw::fixed_point::FixedPoint16u a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp16s(1.23)");
  }
  {
    const rtw::fixed_point::FixedPoint32u a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp32s(1.23)");
  }
  {
    const WrapFixedPoint8 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8w(1.23047)");
  }
  {
    const WrapFixedPoint16 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16w(1.23)");
  }
  {
    const WrapFixedPoint32 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp32w(1.23)");
  }
  {
    const WrapFixedPoint8u a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp8w(1.23047)");
  }
  {
    const WrapFixedPoint16u a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp16w(1.23)");
  }
  {
    const WrapFixedPoint32u a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp32w(1.23)");
  }
  {
    const rtw::fixed_point::FixedPoint8 a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8s(-123)");
  }
  {
    const rtw::fixed_point::FixedPoint16 a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16s(-123)");
  }
  {
    const rtw::fixed_point::FixedPoint32 a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp32s(-123)");
  }
}
