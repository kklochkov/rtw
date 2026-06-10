#include "multiprecision/format.h" // IWYU pragma: keep
#include "multiprecision/rational_math.h"

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

// =============================================================================
// Division by zero death test
// =============================================================================
TEST(RationalDeathTest, division_by_zero)
{
  using Rational32 = rtw::multiprecision::Rational32;
  const Rational32 a(1, 2);
  const Rational32 zero(0);
  EXPECT_DEATH(a / zero, ".*");
}

TEST(RationalDeathTest, construct_zero_denominator)
{
  using Rational32 = rtw::multiprecision::Rational32;
  EXPECT_DEATH(Rational32(1, 0), ".*");
}

TEST(RationalDeathTest, min_numerator_negative_odd_denominator)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // MIN / -odd: result has numerator 2^(N-1) which > MAX, unrepresentable
  EXPECT_DEATH(Rational32(std::numeric_limits<std::int32_t>::min(), -1), ".*");
  EXPECT_DEATH(Rational32(std::numeric_limits<std::int32_t>::min(), -3), ".*");
}

TEST(RationalDeathTest, arithmetic_overflow_addition)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // MAX/1 + 1/1 = (MAX+1)/1 — exceeds int32_t range after normalization
  Rational32 a(std::numeric_limits<std::int32_t>::max(), 1);
  EXPECT_DEATH(a += Rational32(1, 1), ".*");
}

TEST(RationalDeathTest, arithmetic_overflow_subtraction)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // MIN/1 - 1/1 = (MIN-1)/1 — exceeds int32_t range after normalization
  Rational32 a(std::numeric_limits<std::int32_t>::min(), 1);
  EXPECT_DEATH(a -= Rational32(1, 1), ".*");
}

TEST(RationalDeathTest, arithmetic_overflow_multiplication)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // (MAX/1) * (2/1) = 2*MAX/1 — exceeds int32_t range
  Rational32 a(std::numeric_limits<std::int32_t>::max(), 1);
  EXPECT_DEATH(a *= Rational32(2, 1), ".*");
}

TEST(RationalDeathTest, arithmetic_overflow_increment)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // MAX/1 + 1 — exceeds int32_t range
  Rational32 a(std::numeric_limits<std::int32_t>::max(), 1);
  EXPECT_DEATH(++a, ".*");
}

TEST(RationalDeathTest, arithmetic_overflow_decrement)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // MIN/1 - 1 — exceeds int32_t range
  Rational32 a(std::numeric_limits<std::int32_t>::min(), 1);
  EXPECT_DEATH(--a, ".*");
}

TEST(RationalDeathTest, negate_min_odd_denominator)
{
  using Rational32 = rtw::multiprecision::Rational32;
  // -(MIN/1) requires numerator = -MIN = MAX+1, unrepresentable.
  // MIN/1 is already normalized (gcd(|MIN|,1)=1), and numerator is MIN with odd denominator.
  const Rational32 a(std::numeric_limits<std::int32_t>::min(), 1);
  EXPECT_DEATH(-a, ".*");
  // MIN/3 also: after normalization MIN/3 stays as-is (gcd(2^31, 3)=1 since 3 is odd).
  // -(MIN/3) triggers the same assert.
  const Rational32 b(std::numeric_limits<std::int32_t>::min(), 3);
  EXPECT_DEATH(-b, ".*");
}

// =============================================================================
// Rational MIN normalization runtime tests
// =============================================================================
TYPED_TEST(RationalTest, min_numerator_normalization)
{
  using T = decltype(std::declval<TypeParam>().numerator());
  constexpr T MIN_VAL = std::numeric_limits<T>::min();

  // MIN / 2 should reduce to (MIN/2) / 1
  const TypeParam r1(MIN_VAL, T{2});
  EXPECT_EQ(r1.numerator(), MIN_VAL / T{2});
  EXPECT_EQ(r1.denominator(), T{1});

  // MIN / 4 should reduce to (MIN/4) / 1
  const TypeParam r2(MIN_VAL, T{4});
  EXPECT_EQ(r2.numerator(), MIN_VAL / T{4});
  EXPECT_EQ(r2.denominator(), T{1});

  // MIN / 3 should stay as MIN / 3 (gcd(2^(N-1), 3) = 1)
  const TypeParam r3(MIN_VAL, T{3});
  EXPECT_EQ(r3.numerator(), MIN_VAL);
  EXPECT_EQ(r3.denominator(), T{3});

  // MIN / 6: gcd(2^(N-1), 6) = 2, so reduces to (MIN/2) / 3
  const TypeParam r4(MIN_VAL, T{6});
  EXPECT_EQ(r4.numerator(), MIN_VAL / T{2});
  EXPECT_EQ(r4.denominator(), T{3});

  // MIN / 1 stays as MIN / 1
  const TypeParam r5(MIN_VAL, T{1});
  EXPECT_EQ(r5.numerator(), MIN_VAL);
  EXPECT_EQ(r5.denominator(), T{1});

  // MIN / -2 should give positive result: (|MIN|/2) / 1 = (-MIN/2)/1
  const TypeParam r6(MIN_VAL, T{-2});
  EXPECT_EQ(r6.numerator(), -(MIN_VAL / T{2}));
  EXPECT_EQ(r6.denominator(), T{1});
}

// =============================================================================
// Property-based tests
// =============================================================================
TYPED_TEST(RationalTest, property_commutativity)
{
  const TypeParam a(3, 7);
  const TypeParam b(5, 11);

  // Addition is commutative
  EXPECT_EQ(a + b, b + a);

  // Multiplication is commutative
  EXPECT_EQ(a * b, b * a);
}

TYPED_TEST(RationalTest, property_identity)
{
  const TypeParam a(3, 7);
  const TypeParam zero(0);
  const TypeParam one(1);

  // Additive identity
  EXPECT_EQ(a + zero, a);
  EXPECT_EQ(zero + a, a);

  // Multiplicative identity
  EXPECT_EQ(a * one, a);
  EXPECT_EQ(one * a, a);
}

TYPED_TEST(RationalTest, property_inverse)
{
  const TypeParam a(3, 7);
  const TypeParam zero(0);
  const TypeParam one(1);

  // Additive inverse
  EXPECT_EQ(a + (-a), zero);

  // Multiplicative inverse: a * (1/a) == 1
  const TypeParam inverse(7, 3);
  EXPECT_EQ(a * inverse, one);
}

TYPED_TEST(RationalTest, property_distributivity)
{
  const TypeParam a(2, 3);
  const TypeParam b(3, 5);
  const TypeParam c(4, 7);

  // a * (b + c) == a*b + a*c
  EXPECT_EQ(a * (b + c), a * b + a * c);
}

TYPED_TEST(RationalTest, property_subtraction_addition_inverse)
{
  const TypeParam a(5, 9);
  const TypeParam b(2, 3);

  // (a + b) - b == a
  EXPECT_EQ((a + b) - b, a);

  // (a - b) + b == a
  EXPECT_EQ((a - b) + b, a);
}

TYPED_TEST(RationalTest, property_division_multiplication_inverse)
{
  const TypeParam a(5, 9);
  const TypeParam b(2, 3);

  // (a * b) / b == a
  EXPECT_EQ((a * b) / b, a);

  // (a / b) * b == a
  EXPECT_EQ((a / b) * b, a);
}

// =============================================================================
// Constexpr static_assert tests for Rational arithmetic
// =============================================================================
namespace
{

using R32 = rtw::multiprecision::Rational32;

// Construction and normalization
static_assert(R32(6, 4).numerator() == 3, "R32(6,4) normalizes to 3/2");
static_assert(R32(6, 4).denominator() == 2, "R32(6,4) normalizes to 3/2");
static_assert(R32(-6, 4).numerator() == -3, "R32(-6,4) normalizes");
static_assert(R32(6, -4).numerator() == -3, "R32(6,-4) flips sign");
static_assert(R32(6, -4).denominator() == 2, "R32(6,-4) positive denom");
static_assert(R32(0, 5).numerator() == 0, "R32(0,5) == 0/1");
static_assert(R32(0, 5).denominator() == 1, "R32(0,5) denom normalized to 1");

// Addition
static_assert(R32(1, 2) + R32(1, 3) == R32(5, 6), "1/2 + 1/3 == 5/6");
static_assert(R32(1, 2) + R32(-1, 2) == R32(0), "1/2 + -1/2 == 0");

// Subtraction
static_assert(R32(3, 4) - R32(1, 4) == R32(1, 2), "3/4 - 1/4 == 1/2");

// Multiplication
static_assert(R32(2, 3) * R32(3, 4) == R32(1, 2), "2/3 * 3/4 == 1/2");
static_assert(R32(-1, 2) * R32(2, 3) == R32(-1, 3), "-1/2 * 2/3 == -1/3");

// Division
static_assert(R32(1, 2) / R32(2, 3) == R32(3, 4), "1/2 / 2/3 == 3/4");

// Comparison
static_assert(R32(1, 2) < R32(2, 3), "1/2 < 2/3");
static_assert(R32(2, 3) > R32(1, 2), "2/3 > 1/2");
static_assert(R32(1, 2) == R32(2, 4), "1/2 == 2/4 (normalized)");
static_assert(R32(1, 2) != R32(1, 3), "1/2 != 1/3");
static_assert(R32(-1, 2) < R32(0), "-1/2 < 0");

// Unary operators
static_assert(-R32(3, 4) == R32(-3, 4), "-(3/4) == -3/4");
static_assert(+R32(3, 4) == R32(3, 4), "+(3/4) == 3/4");

// Increment/Decrement
constexpr auto rational_increment()
{
  R32 r(1, 2);
  ++r;
  return r;
}
static_assert(rational_increment() == R32(3, 2), "++1/2 == 3/2");

constexpr auto rational_decrement()
{
  R32 r(3, 2);
  --r;
  return r;
}
static_assert(rational_decrement() == R32(1, 2), "--3/2 == 1/2");

// --- math::abs MIN saturation for native integers ---
static_assert(rtw::multiprecision::math::abs(std::int32_t{-42}) == 42, "abs(-42) == 42");
static_assert(rtw::multiprecision::math::abs(std::int32_t{0}) == 0, "abs(0) == 0");
static_assert(rtw::multiprecision::math::abs(std::int32_t{42}) == 42, "abs(42) == 42");
static_assert(rtw::multiprecision::math::abs(std::numeric_limits<std::int32_t>::min())
                  == std::numeric_limits<std::int32_t>::max(),
              "abs(INT32_MIN) saturates to INT32_MAX");
static_assert(rtw::multiprecision::math::abs(std::numeric_limits<std::int32_t>::max())
                  == std::numeric_limits<std::int32_t>::max(),
              "abs(INT32_MAX) == INT32_MAX");

// --- Rational(MIN, even) normalization edge cases ---
constexpr std::int32_t INT32_MIN_VAL = std::numeric_limits<std::int32_t>::min(); // -2^31

static_assert(R32(INT32_MIN_VAL, 2).numerator() == INT32_MIN_VAL / 2, "R32(MIN, 2) reduces numerator");
static_assert(R32(INT32_MIN_VAL, 2).denominator() == 1, "R32(MIN, 2) reduces to MIN/2 over 1");
static_assert(R32(INT32_MIN_VAL, 4).numerator() == INT32_MIN_VAL / 4, "R32(MIN, 4) reduces");
static_assert(R32(INT32_MIN_VAL, 4).denominator() == 1, "R32(MIN, 4) reduces to MIN/4 over 1");
static_assert(R32(INT32_MIN_VAL, 6).numerator() == INT32_MIN_VAL / 2, "R32(MIN, 6) reduces by gcd=2");
static_assert(R32(INT32_MIN_VAL, 6).denominator() == 3, "R32(MIN, 6) denominator is 3");
static_assert(R32(INT32_MIN_VAL, 1).numerator() == INT32_MIN_VAL, "R32(MIN, 1) stays MIN/1");
static_assert(R32(INT32_MIN_VAL, 1).denominator() == 1, "R32(MIN, 1) denominator is 1");
static_assert(R32(INT32_MIN_VAL, 3).numerator() == INT32_MIN_VAL, "R32(MIN, 3) unreduced (gcd=1)");
static_assert(R32(INT32_MIN_VAL, 3).denominator() == 3, "R32(MIN, 3) denominator stays 3");

// Rational(MIN, negative even) — sign flip + reduction
static_assert(R32(INT32_MIN_VAL, -2).numerator() == -(INT32_MIN_VAL / 2), "R32(MIN, -2) positive result");
static_assert(R32(INT32_MIN_VAL, -2).denominator() == 1, "R32(MIN, -2) denominator is 1");
static_assert(R32(INT32_MIN_VAL, -4).numerator() == -(INT32_MIN_VAL / 4), "R32(MIN, -4) positive result");
static_assert(R32(INT32_MIN_VAL, -4).denominator() == 1, "R32(MIN, -4) denominator is 1");

// --- rational_math functions: floor/ceil/round/trunc ---
static_assert(rtw::multiprecision::math::floor(R32(7, 2)) == R32(3), "floor(7/2) == 3");
static_assert(rtw::multiprecision::math::floor(R32(-7, 2)) == R32(-4), "floor(-7/2) == -4");
static_assert(rtw::multiprecision::math::floor(R32(4, 1)) == R32(4), "floor(4/1) == 4");
static_assert(rtw::multiprecision::math::floor(R32(-4, 1)) == R32(-4), "floor(-4/1) == -4");
static_assert(rtw::multiprecision::math::floor(R32(1, 3)) == R32(0), "floor(1/3) == 0");
static_assert(rtw::multiprecision::math::floor(R32(-1, 3)) == R32(-1), "floor(-1/3) == -1");

static_assert(rtw::multiprecision::math::ceil(R32(7, 2)) == R32(4), "ceil(7/2) == 4");
static_assert(rtw::multiprecision::math::ceil(R32(-7, 2)) == R32(-3), "ceil(-7/2) == -3");
static_assert(rtw::multiprecision::math::ceil(R32(4, 1)) == R32(4), "ceil(4/1) == 4");
static_assert(rtw::multiprecision::math::ceil(R32(-4, 1)) == R32(-4), "ceil(-4/1) == -4");
static_assert(rtw::multiprecision::math::ceil(R32(1, 3)) == R32(1), "ceil(1/3) == 1");
static_assert(rtw::multiprecision::math::ceil(R32(-1, 3)) == R32(0), "ceil(-1/3) == 0");

static_assert(rtw::multiprecision::math::round(R32(7, 2)) == R32(4), "round(7/2) == 4");
static_assert(rtw::multiprecision::math::round(R32(-7, 2)) == R32(-4), "round(-7/2) == -4");
static_assert(rtw::multiprecision::math::round(R32(3, 2)) == R32(2), "round(3/2) == 2");
static_assert(rtw::multiprecision::math::round(R32(-3, 2)) == R32(-2), "round(-3/2) == -2");
static_assert(rtw::multiprecision::math::round(R32(1, 3)) == R32(0), "round(1/3) == 0");
static_assert(rtw::multiprecision::math::round(R32(2, 3)) == R32(1), "round(2/3) == 1");
// round of integer values (including MIN) should be identity
static_assert(rtw::multiprecision::math::round(R32(INT32_MIN_VAL, 1)) == R32(INT32_MIN_VAL, 1),
              "round(MIN/1) == MIN/1 (identity for integers)");
static_assert(rtw::multiprecision::math::round(R32(INT32_MIN_VAL, 2)) == R32(INT32_MIN_VAL / 2, 1),
              "round(MIN/2) == MIN/2 (already integer after normalization)");

static_assert(rtw::multiprecision::math::trunc(R32(7, 2)) == R32(3), "trunc(7/2) == 3");
static_assert(rtw::multiprecision::math::trunc(R32(-7, 2)) == R32(-3), "trunc(-7/2) == -3");
static_assert(rtw::multiprecision::math::trunc(R32(1, 3)) == R32(0), "trunc(1/3) == 0");
static_assert(rtw::multiprecision::math::trunc(R32(-1, 3)) == R32(0), "trunc(-1/3) == 0");

// --- rational_math::abs ---
static_assert(rtw::multiprecision::math::abs(R32(-3, 4)) == R32(3, 4), "abs(-3/4) == 3/4");
static_assert(rtw::multiprecision::math::abs(R32(3, 4)) == R32(3, 4), "abs(3/4) == 3/4");
static_assert(rtw::multiprecision::math::abs(R32(0)) == R32(0), "abs(0) == 0");

// --- Rational operator-() with MIN numerator ---
// -Rational(MIN, 2): numerator==MIN, denom==1 after normalization → negate via denom safely
static_assert(-R32(INT32_MIN_VAL, 2) == R32(-(INT32_MIN_VAL / 2), 1), "-(MIN/2) == |MIN/2|/1");
static_assert(-R32(INT32_MIN_VAL, 4) == R32(-(INT32_MIN_VAL / 4), 1), "-(MIN/4) == |MIN/4|/1");
static_assert(-R32(3, 4) == R32(-3, 4), "-(3/4) == -3/4");
static_assert(-R32(-5, 7) == R32(5, 7), "-(-5/7) == 5/7");
static_assert(-R32(0) == R32(0), "-(0) == 0");

// --- Comparison with large values (cross-multiply overflow protection) ---
// These values would overflow int32_t in naive cross-multiply: 100000 * 99991 > INT32_MAX
static_assert(R32(99'989, 99'991) < R32(99'990, 99'991), "large-denom comparison works");
static_assert(R32(99'990, 99'991) > R32(99'989, 99'991), "large-denom comparison (reversed)");
// MAX/2 and MAX/3: cross-products exceed int32_t without widening
constexpr auto HALF_MAX = std::numeric_limits<std::int32_t>::max() / 2;
static_assert(R32(HALF_MAX, HALF_MAX - 1) > R32(1), "near-MAX comparison does not overflow");
static_assert(R32(HALF_MAX - 1, HALF_MAX) < R32(1), "near-MAX comparison (reversed)");
// Negative large values
static_assert(R32(-HALF_MAX, HALF_MAX - 1) < R32(-1), "negative near-MAX comparison");

// --- Arithmetic with large values (intermediate overflow protection) ---
// These products would overflow int32_t without widening to int64_t.
// (MAX/2)/1 + (MAX/2)/1 = (MAX-1)/1 — intermediate is MAX/2 * 1 + MAX/2 * 1 = MAX-1
constexpr auto arith_large_add()
{
  R32 a(HALF_MAX, 1);
  a += R32(HALF_MAX, 1);
  return a;
}
static_assert(arith_large_add() == R32(HALF_MAX * 2, 1), "large addition does not overflow intermediates");

// (2/99991) + (3/99991) = 5/99991 — denominator product would overflow int32_t
constexpr auto arith_large_denom_add()
{
  R32 a(2, 99'991);
  a += R32(3, 99'991);
  return a;
}
static_assert(arith_large_denom_add() == R32(5, 99'991), "large-denom addition normalizes correctly");

// Multiplication: (MAX/2, 1) * (2, MAX) = (1, 1)... let's pick a cleaner example.
// (50000/99991) * (99991/50000) = 1/1
constexpr auto arith_large_mul()
{
  R32 a(50'000, 99'991);
  a *= R32(99'991, 50'000);
  return a;
}
static_assert(arith_large_mul() == R32(1), "large multiplication normalizes to 1");

// Division: (1/99991) / (1/99991) = 1
constexpr auto arith_large_div()
{
  R32 a(1, 99'991);
  a /= R32(1, 99'991);
  return a;
}
static_assert(arith_large_div() == R32(1), "large division normalizes to 1");

// =============================================================================
// Rational64 static_assert proofs (Int128 widening path)
// =============================================================================
using R64 = rtw::multiprecision::Rational64;
constexpr std::int64_t INT64_MAX_VAL = std::numeric_limits<std::int64_t>::max();
constexpr std::int64_t INT64_MIN_VAL = std::numeric_limits<std::int64_t>::min();
constexpr std::int64_t HALF_MAX64 = INT64_MAX_VAL / 2;

// --- Basic arithmetic ---
static_assert(R64(1, 3) + R64(1, 3) == R64(2, 3), "R64 addition basic");
static_assert(R64(5, 7) - R64(2, 7) == R64(3, 7), "R64 subtraction basic");
static_assert(R64(2, 3) * R64(3, 4) == R64(1, 2), "R64 multiplication normalizes");
static_assert(R64(2, 3) / R64(4, 5) == R64(5, 6), "R64 division normalizes");

// --- Large-value comparison (cross-multiply would overflow int64_t without Int128) ---
static_assert(R64(99'999'999'999LL, 99'999'999'997LL) > R64(99'999'999'998LL, 99'999'999'997LL),
              "R64 large-denom comparison works");
static_assert(R64(HALF_MAX64, HALF_MAX64 - 1) > R64(1), "R64 near-MAX comparison does not overflow");
static_assert(R64(HALF_MAX64 - 1, HALF_MAX64) < R64(1), "R64 near-MAX comparison (reversed)");

// --- Large-value arithmetic (intermediate overflow protection via Int128) ---
constexpr auto arith64_large_add()
{
  R64 a(HALF_MAX64, 1);
  a += R64(HALF_MAX64, 1);
  return a;
}
static_assert(arith64_large_add() == R64(HALF_MAX64 * 2, 1), "R64 large addition safe");

// Cross-product of large coprime denominators would overflow int64_t.
// 2/999999937 + 3/999999937 = 5/999999937 (999999937 is prime)
constexpr auto arith64_large_denom_add()
{
  R64 a(2, 999'999'937LL);
  a += R64(3, 999'999'937LL);
  return a;
}
static_assert(arith64_large_denom_add() == R64(5, 999'999'937LL), "R64 large-denom addition normalizes");

// Multiplication: (999999937/999999929) * (999999929/999999937) = 1
// Both are large primes, so no GCD shortcuts — full cross-product needed.
constexpr auto arith64_large_mul()
{
  R64 a(999'999'937LL, 999'999'929LL);
  a *= R64(999'999'929LL, 999'999'937LL);
  return a;
}
static_assert(arith64_large_mul() == R64(1), "R64 large multiplication normalizes to 1");

// --- Negation with MIN ---
static_assert(-R64(INT64_MIN_VAL, 2) == R64(-(INT64_MIN_VAL / 2), 1), "R64 -(MIN/2) is safe");
static_assert(-R64(3, 4) == R64(-3, 4), "R64 negation basic");

// --- Increment/decrement ---
constexpr auto rational64_increment()
{
  R64 r(HALF_MAX64, 1);
  ++r;
  return r;
}
static_assert(rational64_increment() == R64(HALF_MAX64 + 1, 1), "R64 increment near MAX/2");

constexpr auto rational64_decrement()
{
  R64 r(INT64_MIN_VAL / 2, 1);
  --r;
  return r;
}
static_assert(rational64_decrement() == R64(INT64_MIN_VAL / 2 - 1, 1), "R64 decrement near MIN/2");

} // namespace
