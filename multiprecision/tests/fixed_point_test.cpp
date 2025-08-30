#include "multiprecision/fixed_point.h"
#include "multiprecision/format.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Constants were taken from https://chummersone.github.io/qformat.html#converter

TEST(FixedPoint8, constants)
{
  static_assert(rtw::multiprecision::FixedPoint8::BITS == 16);
  static_assert(rtw::multiprecision::FixedPoint8::FRACTIONAL_BITS == 8);
  static_assert(rtw::multiprecision::FixedPoint8::INTEGER_BITS == 7);
  static_assert(rtw::multiprecision::FixedPoint8::ONE == 256);
  static_assert(rtw::multiprecision::FixedPoint8::RESOLUTION == 0.00390625);
  static_assert(rtw::multiprecision::FixedPoint8::min() == -128.0);
  static_assert(rtw::multiprecision::FixedPoint8::max() == 127.99609375);
  static_assert(rtw::multiprecision::FixedPoint8::PI_INTEGER == 804);
}

TEST(FixedPoint8u, constants)
{
  static_assert(rtw::multiprecision::FixedPoint8U::BITS == 16);
  static_assert(rtw::multiprecision::FixedPoint8U::FRACTIONAL_BITS == 8);
  static_assert(rtw::multiprecision::FixedPoint8U::INTEGER_BITS == 8);
  static_assert(rtw::multiprecision::FixedPoint8U::ONE == 256);
  static_assert(rtw::multiprecision::FixedPoint8U::RESOLUTION == 0.00390625);
  static_assert(rtw::multiprecision::FixedPoint8U::min() == 0.0);
  static_assert(rtw::multiprecision::FixedPoint8U::max() == 255.99609375);
  static_assert(rtw::multiprecision::FixedPoint8U::PI_INTEGER == 804);
}

TEST(FixedPoint16, constants)
{
  static_assert(rtw::multiprecision::FixedPoint16::BITS == 32);
  static_assert(rtw::multiprecision::FixedPoint16::FRACTIONAL_BITS == 16);
  static_assert(rtw::multiprecision::FixedPoint16::INTEGER_BITS == 15);
  static_assert(rtw::multiprecision::FixedPoint16::ONE == 65'536);
  static_assert(rtw::multiprecision::FixedPoint16::RESOLUTION == 0.0000152587890625);
  static_assert(rtw::multiprecision::FixedPoint16::min() == -32768.0);
  static_assert(rtw::multiprecision::FixedPoint16::max() == 32767.999984741211);
  static_assert(rtw::multiprecision::FixedPoint16::PI_INTEGER == 205'887);
}

TEST(FixedPoint16U, constants)
{
  static_assert(rtw::multiprecision::FixedPoint16U::BITS == 32);
  static_assert(rtw::multiprecision::FixedPoint16U::FRACTIONAL_BITS == 16);
  static_assert(rtw::multiprecision::FixedPoint16U::INTEGER_BITS == 16);
  static_assert(rtw::multiprecision::FixedPoint16U::ONE == 65'536);
  static_assert(rtw::multiprecision::FixedPoint16U::RESOLUTION == 0.0000152587890625);
  static_assert(rtw::multiprecision::FixedPoint16U::min() == 0.0);
  static_assert(rtw::multiprecision::FixedPoint16U::max() == 65535.999984741211);
  static_assert(rtw::multiprecision::FixedPoint16U::PI_INTEGER == 205'887);
}

TEST(FixedPoint32, constants)
{
  static_assert(rtw::multiprecision::FixedPoint32::BITS == 64);
  static_assert(rtw::multiprecision::FixedPoint32::FRACTIONAL_BITS == 32);
  static_assert(rtw::multiprecision::FixedPoint32::INTEGER_BITS == 31);
  static_assert(rtw::multiprecision::FixedPoint32::ONE == 4'294'967'296);
  static_assert(rtw::multiprecision::FixedPoint32::RESOLUTION == 0.00000000023283064365386962890625);
  static_assert(rtw::multiprecision::FixedPoint32::min() == -2147483648.0);
  static_assert(rtw::multiprecision::FixedPoint32::max() == 2147483648.0);
  static_assert(rtw::multiprecision::FixedPoint32::PI_INTEGER == 13'493'037'705);
}

TEST(FixedPoint32U, constants)
{
  static_assert(rtw::multiprecision::FixedPoint32U::BITS == 64);
  static_assert(rtw::multiprecision::FixedPoint32U::FRACTIONAL_BITS == 32);
  static_assert(rtw::multiprecision::FixedPoint32U::INTEGER_BITS == 32);
  static_assert(rtw::multiprecision::FixedPoint32U::ONE == 4'294'967'296);
  static_assert(rtw::multiprecision::FixedPoint32U::RESOLUTION == 0.00000000023283064365386962890625);
  static_assert(rtw::multiprecision::FixedPoint32U::min() == 0.0);
  static_assert(rtw::multiprecision::FixedPoint32U::max() == 4294967296.0);
  static_assert(rtw::multiprecision::FixedPoint32U::PI_INTEGER == 13'493'037'705);
}
// -----------------------------------------------------------------------------------------------
template <typename T>
class FixedPointTest : public ::testing::Test
{};

using FixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8, rtw::multiprecision::FixedPoint8U,
                                         rtw::multiprecision::FixedPoint16, rtw::multiprecision::FixedPoint16U,
                                         rtw::multiprecision::FixedPoint32, rtw::multiprecision::FixedPoint32U>;
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

using SignedFixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8, rtw::multiprecision::FixedPoint16,
                                               rtw::multiprecision::FixedPoint32>;
TYPED_TEST_SUITE(SignedFixedPointTest, SignedFixedPointTypes, );

TYPED_TEST(SignedFixedPointTest, ctor)
{
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_FLOAT)
  {
    const TypeParam fp(e);
    EXPECT_NEAR(static_cast<float>(fp), (e * TypeParam::ONE) / TypeParam::ONE, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<float>(fp), (e * TypeParam::ONE) * TypeParam::RESOLUTION, TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_DOUBLE)
  {
    const TypeParam fp(e);
    EXPECT_NEAR(static_cast<double>(fp), (e * TypeParam::ONE) / TypeParam::ONE, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(fp), (e * TypeParam::ONE) * TypeParam::RESOLUTION, TypeParam::RESOLUTION);
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
  EXPECT_EQ(TypeParam(20) / TypeParam(2.5), 8);
  EXPECT_EQ(TypeParam(20.0) / TypeParam(2.5), 8);
  {
    const TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a / b, "");
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
  {
    TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a /= b, "");
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
  {
    const TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a / b, "");
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

using UnsignedFixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8U, rtw::multiprecision::FixedPoint16U,
                                                 rtw::multiprecision::FixedPoint32U>;
TYPED_TEST_SUITE(UnsignedFixedPointTest, UnsignedFixedPointTypes, );

TYPED_TEST(UnsignedFixedPointTest, ctor)
{
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_FLOAT)
  {
    const TypeParam fp(e);
    EXPECT_NEAR(static_cast<float>(fp), (e * TypeParam::ONE) / TypeParam::ONE, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<float>(fp), (e * TypeParam::ONE) * TypeParam::RESOLUTION, TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::CTOR_TEST_EXPECTED_DOUBLE)
  {
    const TypeParam fp(e);
    EXPECT_NEAR(static_cast<double>(fp), (e * TypeParam::ONE) / TypeParam::ONE, TypeParam::RESOLUTION);
    EXPECT_NEAR(static_cast<double>(fp), (e * TypeParam::ONE) * TypeParam::RESOLUTION, TypeParam::RESOLUTION);
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
  EXPECT_EQ(TypeParam(20) / TypeParam(2.5), 8);
  EXPECT_EQ(TypeParam(20.0) / TypeParam(2.5), 8);
  {
    const TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a / b, "");
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
  {
    TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a /= b, "");
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
    EXPECT_EQ(a, TypeParam::min()); // See the comment in the implementation.
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
  {
    const TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a / b, "");
  }
}
// -----------------------------------------------------------------------------------------------
TEST(fixed_point, operator_stream)
{
  {
    const rtw::multiprecision::FixedPoint8 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8.8(1.23047)");
  }
  {
    const rtw::multiprecision::FixedPoint16 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16.16(1.23)");
  }
  {
    const rtw::multiprecision::FixedPoint32 a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp32.32(1.23)");
  }
  {
    const rtw::multiprecision::FixedPoint8U a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp8.8(1.23047)");
  }
  {
    const rtw::multiprecision::FixedPoint16U a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp16.16(1.23)");
  }
  {
    const rtw::multiprecision::FixedPoint32U a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp32.32(1.23)");
  }
  {
    const rtw::multiprecision::FixedPoint8 a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8.8(-123)");
  }
  {
    const rtw::multiprecision::FixedPoint16 a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16.16(-123)");
  }
  {
    const rtw::multiprecision::FixedPoint32 a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp32.32(-123)");
  }
  {
    const rtw::multiprecision::FixedPoint8U a(123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp8.8(123)");
  }
  {
    const rtw::multiprecision::FixedPoint16U a(123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp16.16(123)");
  }
  {
    const rtw::multiprecision::FixedPoint32U a(123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp32.32(123)");
  }
}
