#include "fixed_point/fixed_point.h"

#include <gtest/gtest.h>

TEST(fp8_t, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::fp8_t::type>::min() == rtw::fixed_point::fp8_t::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::fp8_t::type>::max() == rtw::fixed_point::fp8_t::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::fp8_t::BITS, 16);
  EXPECT_EQ(rtw::fixed_point::fp8_t::SIGN_BIT, 1);
  EXPECT_EQ(rtw::fixed_point::fp8_t::FRACTIONAL_BITS, 8);
  EXPECT_EQ(rtw::fixed_point::fp8_t::INTEGER_BITS, 7);
  EXPECT_EQ(rtw::fixed_point::fp8_t::ONE, 256);
  EXPECT_EQ(rtw::fixed_point::fp8_t::RESOLUTION, 0.00390625);
  EXPECT_EQ(rtw::fixed_point::fp8_t::min(), -128.0);
  EXPECT_EQ(rtw::fixed_point::fp8_t::max(), 127.99609375);
  EXPECT_EQ(rtw::fixed_point::fp8_t::MIN, -128);
  EXPECT_EQ(rtw::fixed_point::fp8_t::MAX, 127);
}

TEST(ufp8_t, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::ufp8_t::type>::min() == rtw::fixed_point::ufp8_t::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::ufp8_t::type>::max() == rtw::fixed_point::ufp8_t::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::ufp8_t::BITS, 16);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::SIGN_BIT, 0);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::FRACTIONAL_BITS, 8);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::INTEGER_BITS, 8);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::ONE, 256);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::RESOLUTION, 0.00390625);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::min(), 0.0);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::max(), 255.99609375);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::MIN, 0);
  EXPECT_EQ(rtw::fixed_point::ufp8_t::MAX, 255);
}

TEST(fp16_t, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::fp16_t::type>::min() == rtw::fixed_point::fp16_t::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::fp16_t::type>::max() == rtw::fixed_point::fp16_t::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::fp16_t::BITS, 32);
  EXPECT_EQ(rtw::fixed_point::fp16_t::SIGN_BIT, 1);
  EXPECT_EQ(rtw::fixed_point::fp16_t::FRACTIONAL_BITS, 16);
  EXPECT_EQ(rtw::fixed_point::fp16_t::INTEGER_BITS, 15);
  EXPECT_EQ(rtw::fixed_point::fp16_t::ONE, 65536);
  EXPECT_EQ(rtw::fixed_point::fp16_t::RESOLUTION, 0.0000152587890625);
  EXPECT_EQ(rtw::fixed_point::fp16_t::min(), -32768.0);
  EXPECT_EQ(rtw::fixed_point::fp16_t::max(), 32767.999984741211);
  EXPECT_EQ(rtw::fixed_point::fp16_t::MIN, -32768);
  EXPECT_EQ(rtw::fixed_point::fp16_t::MAX, 32767);
}

TEST(ufp16_t, constants)
{
  static_assert(std::numeric_limits<rtw::fixed_point::ufp16_t::type>::min() == rtw::fixed_point::ufp16_t::MIN_INTEGER,
                "The minimum value of the underlying type must be correct");
  static_assert(std::numeric_limits<rtw::fixed_point::ufp16_t::type>::max() == rtw::fixed_point::ufp16_t::MAX_INTEGER,
                "The maximum value of the underlying type must be correct");
  EXPECT_EQ(rtw::fixed_point::ufp16_t::BITS, 32);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::SIGN_BIT, 0);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::FRACTIONAL_BITS, 16);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::INTEGER_BITS, 16);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::ONE, 65536);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::RESOLUTION, 0.0000152587890625);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::min(), 0.0);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::max(), 65535.999984741211);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::MIN, 0);
  EXPECT_EQ(rtw::fixed_point::ufp16_t::MAX, 65535);
}
// -----------------------------------------------------------------------------------------------
template <typename T>
class fixed_point_test : public ::testing::Test
{
};

using FixedPointTypes = ::testing::Types<rtw::fixed_point::fp8_t, rtw::fixed_point::ufp8_t, rtw::fixed_point::fp16_t,
                                         rtw::fixed_point::ufp16_t>;
TYPED_TEST_SUITE(fixed_point_test, FixedPointTypes, );

TYPED_TEST(fixed_point_test, ctor_default)
{
  constexpr TypeParam fp;
  EXPECT_EQ(static_cast<float>(fp), 0.0F);
  EXPECT_EQ(static_cast<double>(fp), 0.0);
  EXPECT_EQ(static_cast<std::int32_t>(fp), 0);
}

TYPED_TEST(fixed_point_test, comparison)
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
class signed_fixed_point_test : public ::testing::Test
{
public:
  constexpr static std::array<float, 10> ctor_test_expected_float = {-128.0F, -35.56F, -7.23F, -1.0F,  0.0F,
                                                                     1.0F,    7.23F,   35.56F, 127.0F, 0.0F};
  constexpr static std::array<double, 10> ctor_test_expected_double = {-128.0, -35.56, -7.23, -1.0,  0.0,
                                                                       1.0,    7.23,   35.56, 127.0, 0.0};
  constexpr static std::array<std::int32_t, 10> ctor_test_expected_int32 = {-128, -36, -7, -1, 0, 1, 7, 29, 36, 127};
  constexpr static std::array<std::uint32_t, 10> ctor_test_expected_uint32 = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
};

using SignedFixedPointTypes = ::testing::Types<rtw::fixed_point::fp8_t, rtw::fixed_point::fp16_t>;
TYPED_TEST_SUITE(signed_fixed_point_test, SignedFixedPointTypes, );

TYPED_TEST(signed_fixed_point_test, ctor)
{
  for (const auto e : TestFixture::ctor_test_expected_float)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::ctor_test_expected_double)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::ctor_test_expected_int32)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<std::int32_t>(fp), e);
  }
  for (const auto e : TestFixture::ctor_test_expected_uint32)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<std::uint32_t>(fp), e);
  }
}

TYPED_TEST(signed_fixed_point_test, arithmetic)
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
    EXPECT_NEAR(static_cast<float>(c), 1.5F - 2.3F, TypeParam::RESOLUTION);
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

TYPED_TEST(signed_fixed_point_test, arithmetic_saturate)
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

TYPED_TEST(signed_fixed_point_test, unary_minus)
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

TYPED_TEST(signed_fixed_point_test, assignment)
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
class unsigned_fixed_point_test : public ::testing::Test
{
public:
  constexpr static std::array<float, 10> ctor_test_expected_float = {0.0F,   1.0F,   7.23F,  7.89F,  8.0F,
                                                                     11.29F, 12.19F, 30.06F, 84.17F, 127.0F};
  constexpr static std::array<double, 10> ctor_test_expected_double = {0.0,   1.0,   7.23,  7.89,  8.0,
                                                                       11.29, 12.19, 30.06, 84.17, 127.0};
  constexpr static std::array<std::uint32_t, 10> ctor_test_expected_uint32 = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
};

using UnsignedFixedPointTypes = ::testing::Types<rtw::fixed_point::ufp8_t, rtw::fixed_point::ufp16_t>;
TYPED_TEST_SUITE(unsigned_fixed_point_test, UnsignedFixedPointTypes, );

TYPED_TEST(unsigned_fixed_point_test, ctor)
{
  for (const auto e : TestFixture::ctor_test_expected_float)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<float>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::ctor_test_expected_double)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) / TypeParam::ONE);
    EXPECT_EQ(static_cast<double>(fp), std::round(e * TypeParam::ONE) * TypeParam::RESOLUTION);
  }
  for (const auto e : TestFixture::ctor_test_expected_uint32)
  {
    const TypeParam fp(e);
    EXPECT_EQ(static_cast<std::uint32_t>(fp), e);
  }
}

TYPED_TEST(unsigned_fixed_point_test, arithmetic)
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
    EXPECT_NEAR(static_cast<float>(c), 2.3F - 1.5F, TypeParam::RESOLUTION);
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

TYPED_TEST(unsigned_fixed_point_test, arithmetic_saturate)
{
  {
    const TypeParam a(-1);
    EXPECT_EQ(a, TypeParam::max()); // See the comment in the implementation.
  }
  {
    const TypeParam a(-1.0);
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

TYPED_TEST(unsigned_fixed_point_test, assignment)
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
using wrap_fp8_t =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::fp8_t::type, rtw::fixed_point::fp8_t::FRACTIONAL_BITS,
                                 rtw::fixed_point::fp8_t::saturation_type, rtw::fixed_point::OverflowPolicy::Wrap>;

using wrap_fp16_t =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::fp16_t::type, rtw::fixed_point::fp16_t::FRACTIONAL_BITS,
                                 rtw::fixed_point::fp16_t::saturation_type, rtw::fixed_point::OverflowPolicy::Wrap>;

using wrap_ufp8_t =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::ufp8_t::type, rtw::fixed_point::ufp8_t::FRACTIONAL_BITS,
                                 rtw::fixed_point::ufp8_t::saturation_type, rtw::fixed_point::OverflowPolicy::Wrap>;

using wrap_ufp16_t =
    rtw::fixed_point::FixedPoint<rtw::fixed_point::ufp16_t::type, rtw::fixed_point::ufp16_t::FRACTIONAL_BITS,
                                 rtw::fixed_point::ufp16_t::saturation_type, rtw::fixed_point::OverflowPolicy::Wrap>;

template <typename T>
class wrap_fixed_point_test : public ::testing::Test
{
public:
  /// All these functions trigger the wrap policy
  /// @{
  constexpr double expected_plus_max_value() const
  {
    if constexpr (std::is_same_v<typename T::type, std::int16_t>)
    {
      return static_cast<double>(std::int8_t(std::int8_t(127) + std::int8_t(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(32767) + std::int16_t(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(255) + std::uint8_t(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(65535) + std::uint16_t(1)));
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
      return static_cast<double>(std::int8_t(std::int8_t(-128) + std::int8_t(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(-32768) + std::int16_t(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(0) + std::uint8_t(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(0) + std::uint16_t(-1)));
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
      return static_cast<double>(std::int8_t(std::int8_t(127) - std::int8_t(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(32767) - std::int16_t(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(255) - std::uint8_t(-1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(65535) - std::uint16_t(-1)));
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
      return static_cast<double>(std::int8_t(std::int8_t(-128) - std::int8_t(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(-32768) - std::int16_t(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(0) - std::uint8_t(1)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::int16_t(0) - std::uint16_t(1)));
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
      return static_cast<double>(std::int8_t(std::int8_t(127) * std::int8_t(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(32767) * std::int16_t(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(255) * std::uint8_t(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(65535) * std::uint16_t(2)));
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
      return static_cast<double>(std::int8_t(std::int8_t(-128) * std::int8_t(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(-32768) * std::int16_t(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(0) * std::uint8_t(2)));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(0) * std::uint16_t(2)));
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
      return static_cast<double>(std::int8_t(std::int8_t(127) * (std::int8_t(1) / std::int8_t(2))));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(32767) * (std::int16_t(1) / std::int16_t(2))));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(255) * (std::uint8_t(1) / std::uint8_t(2))));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(65535) * (std::uint16_t(1) / std::uint16_t(2))));
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
      return static_cast<double>(std::int8_t(std::int8_t(-128) * (std::int8_t(1) / std::int8_t(2))));
    }
    else if constexpr (std::is_same_v<typename T::type, std::int32_t>)
    {
      return static_cast<double>(std::int16_t(std::int16_t(-32767) * (std::int16_t(1) / std::int16_t(2))));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint16_t>)
    {
      return static_cast<double>(std::uint8_t(std::uint8_t(0) * (std::uint8_t(1) / std::uint8_t(2))));
    }
    else if constexpr (std::is_same_v<typename T::type, std::uint32_t>)
    {
      return static_cast<double>(std::uint16_t(std::uint16_t(0) * (std::uint16_t(1) / std::uint16_t(2))));
    }
    else
    {
      static_assert(sizeof(T) == 0,
                    "Unsupported type"); // workaround before CWG2518/P2593R1
    }
  }
  /// @}
};

using WrapFixedPointTypes = ::testing::Types<wrap_fp8_t, wrap_fp16_t, wrap_ufp8_t, wrap_ufp16_t>;
TYPED_TEST_SUITE(wrap_fixed_point_test, WrapFixedPointTypes, );

TYPED_TEST(wrap_fixed_point_test, arithmetic_wrap)
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
    const rtw::fixed_point::fp8_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8s(1.23047)");
  }
  {
    const rtw::fixed_point::fp16_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16s(1.23)");
  }
  {
    const rtw::fixed_point::ufp8_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp8s(1.23047)");
  }
  {
    const rtw::fixed_point::ufp16_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp16s(1.23)");
  }
  {
    const wrap_fp8_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp8w(1.23047)");
  }
  {
    const wrap_fp16_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16w(1.23)");
  }
  {
    const wrap_ufp8_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp8w(1.23047)");
  }
  {
    const wrap_ufp16_t a(1.23);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "ufp16w(1.23)");
  }
  {
    const rtw::fixed_point::fp16_t a(-123);
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "fp16s(-123)");
  }
}
