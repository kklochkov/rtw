#include "multiprecision/fixed_point.h"
#include "multiprecision/fixed_point_math.h"
#include "multiprecision/format.h" // IWYU pragma: keep

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
  {
    TypeParam a(1.0);
    const TypeParam b(0.0);
    EXPECT_DEATH(a %= b, "");
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
  // Constructor saturation from out-of-range values
  {
    // Value larger than max should saturate to max
    const double large_value = static_cast<double>(TypeParam::max()) * 2.0;
    const TypeParam a(large_value);
    EXPECT_EQ(a, TypeParam::max());
  }
  {
    // Value smaller than min should saturate to min
    const double small_value = static_cast<double>(TypeParam::min()) * 2.0;
    const TypeParam a(small_value);
    EXPECT_EQ(a, TypeParam::min());
  }
  // Increment/decrement at boundaries
  {
    TypeParam a = TypeParam::max();
    ++a;
    EXPECT_EQ(a, TypeParam::max());
  }
  {
    TypeParam a = TypeParam::min();
    --a;
    EXPECT_EQ(a, TypeParam::min());
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
// -----------------------------------------------------------------------------------------------
TEST(NumericLimits, epsilon)
{
  // epsilon() should return the smallest representable positive value (ULP)
  {
    using FP = rtw::multiprecision::FixedPoint8;
    constexpr auto EPS = std::numeric_limits<FP>::epsilon();
    static_assert(EPS.raw_value() == 1);
    EXPECT_DOUBLE_EQ(static_cast<double>(EPS), FP::RESOLUTION);
  }
  {
    using FP = rtw::multiprecision::FixedPoint16;
    constexpr auto EPS = std::numeric_limits<FP>::epsilon();
    static_assert(EPS.raw_value() == 1);
    EXPECT_DOUBLE_EQ(static_cast<double>(EPS), FP::RESOLUTION);
  }
  {
    using FP = rtw::multiprecision::FixedPoint32;
    constexpr auto EPS = std::numeric_limits<FP>::epsilon();
    static_assert(EPS.raw_value() == 1);
    EXPECT_DOUBLE_EQ(static_cast<double>(EPS), FP::RESOLUTION);
  }
}

TEST(NumericLimits, round_error)
{
  // round_error() should return 0.5 (half of the ULP for round-to-nearest)
  {
    using FP = rtw::multiprecision::FixedPoint8;
    constexpr auto ERR = std::numeric_limits<FP>::round_error();
    static_assert(ERR.raw_value() == FP::HALF);
    EXPECT_DOUBLE_EQ(static_cast<double>(ERR), 0.5);
  }
  {
    using FP = rtw::multiprecision::FixedPoint16;
    constexpr auto ERR = std::numeric_limits<FP>::round_error();
    static_assert(ERR.raw_value() == FP::HALF);
    EXPECT_DOUBLE_EQ(static_cast<double>(ERR), 0.5);
  }
  {
    using FP = rtw::multiprecision::FixedPoint32;
    constexpr auto ERR = std::numeric_limits<FP>::round_error();
    static_assert(ERR.raw_value() == FP::HALF);
    EXPECT_DOUBLE_EQ(static_cast<double>(ERR), 0.5);
  }
}

TEST(FixedPointConversion, widening_is_lossless)
{
  using rtw::multiprecision::FixedPoint16;
  using rtw::multiprecision::FixedPoint32;

  // Widening FixedPoint16 -> FixedPoint32 preserves the value exactly.
  for (const double value : {0.0, 1.0, -1.0, 0.5, -0.5, 123.25, -123.25, 0.0000152587890625 /* 1 ULP */})
  {
    const FixedPoint16 narrow{value};
    const FixedPoint32 wide{narrow};
    EXPECT_DOUBLE_EQ(static_cast<double>(wide), static_cast<double>(narrow));
  }

  // The raw value is shifted left by the difference in fractional bits (16 -> 32 => << 16).
  const FixedPoint16 one_ulp{rtw::multiprecision::RAW_VALUE_CONSTRUCT, 1};
  const FixedPoint32 wide_one_ulp{one_ulp};
  EXPECT_EQ(wide_one_ulp.raw_value(), static_cast<std::int64_t>(1) << 16);
}

TEST(FixedPointConversion, narrowing_rounds_half_away_from_zero)
{
  using rtw::multiprecision::FixedPoint16;
  using rtw::multiprecision::FixedPoint32;

  // Values representable in both round-trip exactly.
  for (const double value : {0.0, 1.0, -1.0, 0.5, -0.5, 100.25, -100.25})
  {
    const FixedPoint32 wide{value};
    const FixedPoint16 narrow{wide};
    EXPECT_DOUBLE_EQ(static_cast<double>(narrow), value);
  }

  // 1.5 of FixedPoint16's ULP, expressed in FixedPoint32, rounds away from zero to 2 ULP.
  constexpr auto ULP16_IN_FP32 = static_cast<std::int64_t>(1) << 16; // 2^-16 expressed at 2^-32 scale.
  const FixedPoint32 one_and_half_ulp{rtw::multiprecision::RAW_VALUE_CONSTRUCT, ULP16_IN_FP32 + (ULP16_IN_FP32 / 2)};
  EXPECT_EQ(FixedPoint16{one_and_half_ulp}.raw_value(), 2);

  const FixedPoint32 neg_one_and_half_ulp{rtw::multiprecision::RAW_VALUE_CONSTRUCT,
                                          -(ULP16_IN_FP32 + (ULP16_IN_FP32 / 2))};
  EXPECT_EQ(FixedPoint16{neg_one_and_half_ulp}.raw_value(), -2);
}

TEST(FixedPointConversion, narrowing_saturates_out_of_range)
{
  using rtw::multiprecision::FixedPoint16;
  using rtw::multiprecision::FixedPoint32;

  // FixedPoint32 holds values far beyond FixedPoint16's range (~+-32768) -> clamp on narrowing.
  EXPECT_EQ(FixedPoint16{FixedPoint32{1.0e6}}, FixedPoint16::max());
  EXPECT_EQ(FixedPoint16{FixedPoint32{-1.0e6}}, FixedPoint16::min());
}

TEST(FixedPointConversion, round_trip_preserves_representable_values)
{
  using rtw::multiprecision::FixedPoint16;
  using rtw::multiprecision::FixedPoint32;

  for (const double value : {0.0, 1.0, -1.0, 0.25, -0.25, 12345.5, -12345.5})
  {
    const FixedPoint16 original{value};
    EXPECT_EQ(FixedPoint16{FixedPoint32{original}}, original);
  }
}

TEST(NumericLimits, exponent10_is_constexpr)
{
  // Verify min_exponent10 and max_exponent10 are constexpr (previously used non-constexpr std::log10)
  {
    using FP = rtw::multiprecision::FixedPoint8;
    constexpr std::int32_t MIN_EXP10 = std::numeric_limits<FP>::min_exponent10;
    constexpr std::int32_t MAX_EXP10 = std::numeric_limits<FP>::max_exponent10;
    // min_exponent10 should be negative (fractional bits give sub-1 resolution)
    EXPECT_LT(MIN_EXP10, 0);
    // max_exponent10 should be positive (integer bits give values > 1)
    EXPECT_GT(MAX_EXP10, 0);
  }
  {
    using FP = rtw::multiprecision::FixedPoint16;
    constexpr std::int32_t MIN_EXP10 = std::numeric_limits<FP>::min_exponent10;
    constexpr std::int32_t MAX_EXP10 = std::numeric_limits<FP>::max_exponent10;
    EXPECT_LT(MIN_EXP10, 0);
    EXPECT_GT(MAX_EXP10, 0);
  }
  {
    using FP = rtw::multiprecision::FixedPoint32;
    constexpr std::int32_t MIN_EXP10 = std::numeric_limits<FP>::min_exponent10;
    constexpr std::int32_t MAX_EXP10 = std::numeric_limits<FP>::max_exponent10;
    EXPECT_LT(MIN_EXP10, 0);
    EXPECT_GT(MAX_EXP10, 0);
  }
}

// =============================================================================
// Constexpr static_assert tests for FixedPoint arithmetic
// =============================================================================
namespace
{

using FP8 = rtw::multiprecision::FixedPoint8;

// Construction from positive integer
static_assert(FP8{1}.raw_value() == FP8::ONE, "FP8(1) == ONE");
static_assert(FP8{0}.raw_value() == 0, "FP8(0) == 0");
static_assert(FP8{5}.raw_value() == 5 * FP8::ONE, "FP8(5) == 5*ONE");

// Raw-value construction
static_assert(FP8(rtw::multiprecision::RAW_VALUE_CONSTRUCT, 256).raw_value() == 256, "FP8 raw 256");
static_assert(FP8(rtw::multiprecision::RAW_VALUE_CONSTRUCT, -256).raw_value() == -256, "FP8 raw -256");

// Addition
static_assert((FP8{1} + FP8{2}).raw_value() == 3 * FP8::ONE, "FP8 1+2 == 3");
static_assert((FP8{10} + FP8{20}).raw_value() == 30 * FP8::ONE, "FP8 10+20 == 30");

// Subtraction
static_assert((FP8{5} - FP8{3}).raw_value() == 2 * FP8::ONE, "FP8 5-3 == 2");
static_assert((FP8{1} - FP8{1}).raw_value() == 0, "FP8 1-1 == 0");

// Multiplication
static_assert((FP8{2} * FP8{3}).raw_value() == 6 * FP8::ONE, "FP8 2*3 == 6");
static_assert((FP8{4} * FP8{5}).raw_value() == 20 * FP8::ONE, "FP8 4*5 == 20");

// Division
static_assert((FP8{6} / FP8{2}).raw_value() == 3 * FP8::ONE, "FP8 6/2 == 3");
static_assert((FP8{10} / FP8{5}).raw_value() == 2 * FP8::ONE, "FP8 10/5 == 2");

// Comparison (positive values only for constexpr)
static_assert(FP8{1} > FP8{0}, "FP8 1 > 0");
static_assert(FP8{2} == FP8{2}, "FP8 2 == 2");
static_assert(FP8{1} != FP8{2}, "FP8 1 != 2");
static_assert(FP8{1} <= FP8{1}, "FP8 1 <= 1");
static_assert(FP8{1} >= FP8{1}, "FP8 1 >= 1");
static_assert(FP8{3} > FP8{2}, "FP8 3 > 2");
static_assert(FP8{2} < FP8{3}, "FP8 2 < 3");

// Saturation: addition that overflows should clamp to max
constexpr auto fp8_saturate_add()
{
  FP8 v = FP8::max();
  v += FP8{1};
  return v;
}
static_assert(fp8_saturate_add() == FP8::max(), "FP8 max+1 saturates to max");

// Subtraction resulting in smaller value
constexpr auto fp8_sub_chain()
{
  FP8 v{10};
  v -= FP8{3};
  v -= FP8{2};
  return v;
}
static_assert(fp8_sub_chain() == FP8{5}, "FP8 10-3-2 == 5");

// Multiplication and division compose
static_assert((FP8{6} / FP8{3}) * FP8{3} == FP8{6}, "FP8 (6/3)*3 == 6");

// Unary negation: -min() saturates to max()
static_assert((-FP8::min()) == FP8::max(), "FP8 -min() saturates to max()");
static_assert((-FP8{5}).raw_value() == -5 * FP8::ONE, "FP8 -5 negates correctly");
static_assert((-(-FP8{3})) == FP8{3}, "FP8 double negation is identity");

// round(min()) should not UB — min() has zero fractional part, returns itself
static_assert(rtw::multiprecision::math::round(FP8::min()) == FP8::min(), "FP8 round(min) == min");
static_assert(rtw::multiprecision::math::round(FP8{0}) == FP8{0}, "FP8 round(0) == 0");
static_assert(rtw::multiprecision::math::round(FP8{3}) == FP8{3}, "FP8 round(3) == 3 (integer stays)");
static_assert(rtw::multiprecision::math::trunc(FP8::min()) == FP8::min(), "FP8 trunc(min) == min");

// ceil: basic correctness
static_assert(rtw::multiprecision::math::ceil(FP8{3}) == FP8{3}, "FP8 ceil(3) == 3 (already integer)");
static_assert(rtw::multiprecision::math::ceil(FP8{0}) == FP8{0}, "FP8 ceil(0) == 0");
static_assert(rtw::multiprecision::math::ceil(FP8::min()) == FP8::min(), "FP8 ceil(min) == min (no frac)");

// ceil: fractional values round up
constexpr auto fp8_ceil_positive_frac()
{
  // 1.5 in raw: 1*256 + 128 = 384
  FP8 v(rtw::multiprecision::RAW_VALUE_CONSTRUCT, 384);
  return rtw::multiprecision::math::ceil(v);
}
static_assert(fp8_ceil_positive_frac() == FP8{2}, "FP8 ceil(1.5) == 2");

constexpr auto fp8_ceil_negative_frac()
{
  // -1.5 in raw: -(1*256 + 128) = -384
  FP8 v(rtw::multiprecision::RAW_VALUE_CONSTRUCT, -384);
  return rtw::multiprecision::math::ceil(v);
}
static_assert(fp8_ceil_negative_frac().raw_value() == -256, "FP8 ceil(-1.5) == -1");

// ceil: near-MAX overflow saturates to max()
constexpr auto fp8_ceil_near_max()
{
  // max() = 0x7FFF = 32767 raw. Integer part = 127, frac part = 0xFF = all 1s.
  // ceil(127.996) should be 128, but 128 can't be represented -> saturate to max().
  return rtw::multiprecision::math::ceil(FP8::max());
}
static_assert(fp8_ceil_near_max() == FP8::max(), "FP8 ceil(max) saturates to max");

// ceil: one-below-max integer has fractional bits, saturates
constexpr auto fp8_ceil_just_above_max_integer()
{
  // 127 + epsilon in raw: 127*256 + 1 = 32513
  FP8 v(rtw::multiprecision::RAW_VALUE_CONSTRUCT, 127 * 256 + 1);
  return rtw::multiprecision::math::ceil(v);
}
static_assert(fp8_ceil_just_above_max_integer() == FP8::max(), "FP8 ceil(127+eps) saturates to max");

// ceil: unsigned near-max overflow saturates
using FP8U = rtw::multiprecision::FixedPoint8U;
constexpr auto fp8u_ceil_near_max() { return rtw::multiprecision::math::ceil(FP8U::max()); }
static_assert(fp8u_ceil_near_max() == FP8U::max(), "FP8U ceil(max) saturates to max");

constexpr auto fp8u_ceil_just_above_max_integer()
{
  // 255 + epsilon in raw: 255*256 + 1 = 65281
  FP8U v(rtw::multiprecision::RAW_VALUE_CONSTRUCT, 255U * 256U + 1U);
  return rtw::multiprecision::math::ceil(v);
}
static_assert(fp8u_ceil_just_above_max_integer() == FP8U::max(), "FP8U ceil(255+eps) saturates to max");

// numeric_limits::digits: should be total value bits (INTEGER_BITS + FRACTIONAL_BITS)
static_assert(std::numeric_limits<FP8>::digits == 15, "FP8 digits = 7 int + 8 frac = 15");
static_assert(std::numeric_limits<FP8U>::digits == 16, "FP8U digits = 8 int + 8 frac = 16");
static_assert(std::numeric_limits<rtw::multiprecision::FixedPoint16>::digits == 31,
              "FP16 digits = 15 int + 16 frac = 31");
static_assert(std::numeric_limits<rtw::multiprecision::FixedPoint16U>::digits == 32,
              "FP16U digits = 16 int + 16 frac = 32");

// round: near-MAX positive saturates to max (signed)
constexpr auto fp8_round_near_max()
{
  // max() = 0x7FFF raw. Integer=127, frac=0xFF. 127.996... rounds to 128, but can't represent -> saturate.
  return rtw::multiprecision::math::round(FP8::max());
}
static_assert(fp8_round_near_max() == FP8::max(), "FP8 round(max) saturates to max");

// round: unsigned near-MAX saturates
constexpr auto fp8u_round_near_max() { return rtw::multiprecision::math::round(FP8U::max()); }
static_assert(fp8u_round_near_max() == FP8U::max(), "FP8U round(max) saturates to max");

// Converting constructor between fixed-point types is constexpr and value-preserving for
// representable values (widening is always lossless; narrowing of integers is exact).
static_assert(rtw::multiprecision::FixedPoint32{FP8{5}} == rtw::multiprecision::FixedPoint32{5},
              "FP8(5) widened to FP32 equals FP32(5)");
static_assert(FP8{rtw::multiprecision::FixedPoint32{5}} == FP8{5}, "FP32(5) narrowed to FP8 equals FP8(5)");
static_assert(FP8{rtw::multiprecision::FixedPoint32{-3.0}} == -FP8{3}, "FP32(-3) narrowed to FP8 equals FP8(-3)");

} // namespace
