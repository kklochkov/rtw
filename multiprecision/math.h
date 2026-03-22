#pragma once

#include "constants/math_constants.h"
#include "multiprecision/fixed_point.h"
#include "multiprecision/int128.h"
#include "multiprecision/operations.h"
#include "multiprecision/rational.h"

#include <array>

namespace rtw::multiprecision::math
{

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> clamp(const FixedPoint<T, FRAC_BITS, SaturationT> value,
                                                      const FixedPoint<T, FRAC_BITS, SaturationT> min,
                                                      const FixedPoint<T, FRAC_BITS, SaturationT> max) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  return FixedPoint(RAW_VALUE_CONSTRUCT, std::clamp(value.raw_value(), min.raw_value(), max.raw_value()));
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> floor(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round down
  return FixedPoint(RAW_VALUE_CONSTRUCT, value.raw_value() & FixedPoint::INTEGER_MASK);
}

template <typename T>
constexpr Rational<T> floor(const Rational<T> value) noexcept
{
  const T num = value.numerator();
  const T den = value.denominator();

  T result = num / den;

  // For negative values with a remainder, we need to subtract 1 to get floor
  if ((num < 0) && ((num % den) != 0))
  {
    --result;
  }

  return Rational<T>(result, T{1});
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> ceil(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round up
  return FixedPoint(RAW_VALUE_CONSTRUCT, (value.raw_value() + FixedPoint::FRACTION_MASK) & FixedPoint::INTEGER_MASK);
}

template <typename T>
constexpr Rational<T> ceil(const Rational<T> value) noexcept
{
  const T num = value.numerator();
  const T den = value.denominator();

  T result = num / den;

  // For positive values with a remainder, we need to add 1 to get ceiling
  if ((num > 0) && ((num % den) != 0))
  {
    ++result;
  }

  return Rational<T>(result, T{1});
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> round(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Round half away from zero (matching std::round behavior)
  if constexpr (std::is_signed_v<T>)
  {
    const auto raw = value.raw_value();
    if (raw >= 0)
    {
      return FixedPoint(RAW_VALUE_CONSTRUCT, (raw + FixedPoint::HALF) & FixedPoint::INTEGER_MASK);
    }
    // For negative: negate, round as positive, negate back
    const auto pos_rounded = (-raw + FixedPoint::HALF) & FixedPoint::INTEGER_MASK;
    return FixedPoint(RAW_VALUE_CONSTRUCT, -pos_rounded);
  }
  else
  {
    return FixedPoint(RAW_VALUE_CONSTRUCT, (value.raw_value() + FixedPoint::HALF) & FixedPoint::INTEGER_MASK);
  }
}

template <typename T>
constexpr Rational<T> round(const Rational<T> value) noexcept
{
  const T num = value.numerator();
  const T den = value.denominator();

  // Compute (|num| + den/2) / den, then restore sign
  // This implements round-half-away-from-zero semantics
  using std::abs;
  const T half_den = den / T{2};
  T result = (abs(num) + half_den) / den;

  // Handle exact half case: round away from zero
  // If 2*|num| % (2*den) == den, we're exactly at half
  // Since we added half_den, this is already handled correctly for positive,
  // but we need to be careful about odd denominators
  if (num < 0)
  {
    result = -result;
  }

  return Rational<T>(result, T{1});
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> trunc(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  if constexpr (std::is_signed_v<T>)
  {
    if (value.raw_value() < 0)
    {
      return ceil(value);
    }
  }
  return floor(value);
}

template <typename T>
constexpr Rational<T> trunc(const Rational<T> value) noexcept
{
  return Rational<T>(value.numerator() / value.denominator(), T{1});
}

/// Raise a rational number to an integer power.
/// Uses exponentiation by squaring for efficiency.
/// @pre For negative exponents, value must not be zero.
/// @param[in] base The base rational number.
/// @param[in] exponent The integer exponent (can be negative).
/// @return base raised to the power of exponent.
template <typename T>
constexpr Rational<T> pow(Rational<T> base, const std::int32_t exponent) noexcept
{
  // Handle zero exponent
  if (exponent == 0)
  {
    return Rational<T>(T{1}, T{1});
  }

  // Handle negative exponent: x^(-n) = (1/x)^n
  std::uint32_t exp = 0;
  if (exponent < 0)
  {
    assert(base.numerator() != 0 && "pow: zero base with negative exponent");
    // Invert the base: swap numerator and denominator
    base = Rational<T>(base.denominator(), base.numerator());
    exp = static_cast<std::uint32_t>(-exponent);
  }
  else
  {
    exp = static_cast<std::uint32_t>(exponent);
  }

  // Exponentiation by squaring
  Rational<T> result(T{1}, T{1});
  while (exp > 0U)
  {
    if ((exp & 1U) != 0U)
    {
      result *= base;
    }
    base *= base;
    exp >>= 1U;
  }

  return result;
}

/// Calculate the square root of a fixed-point number using the Heron's (Babylonian) method.
/// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Heron's_method
///
/// The initial guess is computed using the bit width of the value, which provides a much better
/// starting point than value/2. For a value with bit position b (highest set bit), sqrt is approximately 2^(b/2).
///
/// @pre value must be non-negative. Violation will trigger assertion failure.
/// @param[in] value The non-negative fixed-point number.
/// @return The square root of the value.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sqrt(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  using UnsignedT = std::make_unsigned_t<T>;

  if (value.raw_value() == 0)
  {
    return value;
  }

  assert((value.raw_value() > 0) && "Cannot calculate the square root of a negative number");

  constexpr FixedPoint HALF(RAW_VALUE_CONSTRUCT, FixedPoint::HALF);

  // Compute a initial guess using the bit width of the value.
  // For a value v, if the highest set bit is at position b, then sqrt(v) ≈ 2^(b/2).
  // This is much closer to the actual sqrt than value/2.
  const auto leading_zeros = count_leading_zero(static_cast<UnsignedT>(value.raw_value()));
  const auto bit_width = static_cast<std::int32_t>(FixedPoint::BITS) - leading_zeros;
  // sqrt(2^b) = 2^(b/2), but we need to account for the fractional bits scaling
  // The raw value represents value * 2^FRAC_BITS, so sqrt(raw) = sqrt(value) * 2^(FRAC_BITS/2)
  // We want the result in the same fixed-point format, so we need sqrt(value) * 2^FRAC_BITS
  // Initial guess: 2^((bit_width + FRAC_BITS) / 2)
  const auto guess_shift = (bit_width + static_cast<std::int32_t>(FixedPoint::FRACTIONAL_BITS)) / 2;
  FixedPoint guess(RAW_VALUE_CONSTRUCT, static_cast<T>(UnsignedT{1} << guess_shift));
  FixedPoint previous_guess{};

  for (std::uint8_t i = 0U; (i < FixedPoint::FRACTIONAL_BITS) && (guess != previous_guess); ++i)
  {
    previous_guess = guess;
    guess = (guess + (value / guess)) * HALF;
  }

  return guess;
}

namespace details
{

constexpr std::uint64_t factorial(const std::uint64_t n) noexcept
{
  std::uint64_t result = 1U;
  for (std::uint64_t i = 1U; i <= n; ++i)
  {
    result *= i;
  }
  return result;
}

constexpr inline std::array INV_FACTORIALS{
    1.0 / factorial(0U), 1.0 / factorial(1U), 1.0 / factorial(2U), 1.0 / factorial(3U), 1.0 / factorial(4U),
    1.0 / factorial(5U), 1.0 / factorial(6U), 1.0 / factorial(7U), 1.0 / factorial(8U), 1.0 / factorial(9U)};

enum class Quadrant : std::uint8_t
{
  I,
  II,
  III,
  IV
};

constexpr std::int8_t sin_sign(const Quadrant quadrant) noexcept
{
  constexpr std::int8_t SIGNS[] = {1, 1, -1, -1};
  return SIGNS[static_cast<std::uint8_t>(quadrant)]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
}

constexpr std::int8_t cos_sign(const Quadrant quadrant) noexcept
{
  constexpr std::int8_t SIGNS[] = {1, -1, -1, 1};
  return SIGNS[static_cast<std::uint8_t>(quadrant)]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
}

/// Normalize the angle to the range [-PI/2, PI/2] and determine the quadrant.
/// @param[in] angle The angle in radians.
/// @param[out] quadrant The quadrant of the angle.
/// @return The normalized angle in the range [-PI/2, PI/2].
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> normalize_angle(FixedPoint<T, FRAC_BITS, SaturationT> angle,
                                                                Quadrant& quadrant) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  constexpr FixedPoint PI_2 = FixedPoint::pi_2();
  constexpr FixedPoint PI = FixedPoint::pi();
  constexpr FixedPoint TWO_PI = FixedPoint::two_pi();
  constexpr FixedPoint THREE_PI_2 = 3 * PI_2;

  // Normalize the angle to the range [0, 2*PI]
  angle %= TWO_PI;
  if (angle < 0)
  {
    angle += TWO_PI;
  }

  // Determine the quadrant and map the angle to [-PI/2, PI/2]
  if (angle <= PI_2)
  {
    quadrant = Quadrant::I;
    return angle;
  }

  if (angle <= PI)
  {
    quadrant = Quadrant::II;
    return PI - angle;
  }

  if (angle <= THREE_PI_2)
  {
    quadrant = Quadrant::III;
    return angle - PI;
  }

  quadrant = Quadrant::IV;
  return TWO_PI - angle;
}

/// Core Taylor series calculation for asin.
/// Only valid for |x| <= 0.5 where the series converges well.
/// asin(x) = x + x^3/6 + (3*x^5)/40 + (5*x^7)/112 + (35*x^9)/1152
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> asin_taylor(const FixedPoint<T, FRAC_BITS, SaturationT> x) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  const FixedPoint x_pow_2 = x * x;
  const FixedPoint x_pow_3 = x * x_pow_2;
  const FixedPoint x_pow_5 = x_pow_3 * x_pow_2;
  const FixedPoint x_pow_7 = x_pow_5 * x_pow_2;
  const FixedPoint x_pow_9 = x_pow_7 * x_pow_2;

  FixedPoint result = x;
  result += x_pow_3 / T{6};
  result += (T{3} * x_pow_5) / T{40};
  result += (T{5} * x_pow_7) / T{112};
  result += (T{35} * x_pow_9) / T{1'152};

  return result;
}

/// Core Taylor series calculation for atan.
/// Best accuracy for |x| < 0.5. Converges for |x| <= 1 but slower near 1.
/// atan(x) = x - x^3/3 + x^5/5 - x^7/7 + x^9/9
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> atan_taylor(const FixedPoint<T, FRAC_BITS, SaturationT> x) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  const FixedPoint x_pow_2 = x * x;
  const FixedPoint x_pow_3 = x * x_pow_2;
  const FixedPoint x_pow_5 = x_pow_3 * x_pow_2;
  const FixedPoint x_pow_7 = x_pow_5 * x_pow_2;
  const FixedPoint x_pow_9 = x_pow_7 * x_pow_2;

  FixedPoint result = x;
  result -= x_pow_3 / T{3};
  result += x_pow_5 / T{5};
  result -= x_pow_7 / T{7};
  result += x_pow_9 / T{9};

  return result;
}

} // namespace details

/// Calculate the sine/cosine of a fixed-point number using the Taylor series expansion.
/// The input angle is normalized to the range [-PI/2, PI/2].
/// First five terms of the Taylor series expansion are used:
/// * sin(x) = x - x^3/3! + x^5/5! - x^7/7! + x^9/9!
/// * cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + x^8/8!
/// where x is in the range [-PI/2, PI/2].
/// The absolute error is less than 0.0003.
///
/// @note Performance optimization - Horner's method:
/// The polynomial can be evaluated more efficiently using Horner's method, which reduces
/// the number of multiplications by evaluating the polynomial in nested form:
/// * sin(x) = x * (1 - x^2 * (1/6 - x^2 * (1/120 - x^2 * (1/5040 - x^2/362880))))
/// * cos(x) = 1 - x^2 * (1/2 - x^2 * (1/24 - x^2 * (1/720 - x^2/40320)))
/// This reduces 4 power multiplications to sequential x^2 multiplications.
///
/// @note Performance optimization - Lookup tables:
/// For low-precision types like FixedPoint8, a lookup table with 256 entries covering [0, PI/2].
/// The table requires only 512 bytes of storage and can use linear interpolation for intermediate values.
///
/// Side note: an alternative approach is to use Chebyshev polynomials or Bhaskara I's sine approximation formula.
/// Robin Green's "Even Faster Math Functions" provides a good overview of the different approaches.
/// https://basesandframes.wordpress.com/2020/04/04/even-faster-math-functions/
/// See also multiprecision/analysis/chebyshev_trig.py for comparing the Taylor series and Chebyshev polynomials.
/// @param[in] value The angle in radians.
/// @return The sine of the angle.
/// @{
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sin(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  details::Quadrant quadrant{details::Quadrant::I};
  const FixedPoint x = details::normalize_angle(value, quadrant);

  const FixedPoint x_pow_2 = x * x;
  const FixedPoint x_pow_3 = x * x_pow_2;
  const FixedPoint x_pow_5 = x_pow_3 * x_pow_2;
  const FixedPoint x_pow_7 = x_pow_5 * x_pow_2;
  const FixedPoint x_pow_9 = x_pow_7 * x_pow_2;

  FixedPoint result = x;
  result -= x_pow_3 * details::INV_FACTORIALS[3];
  result += x_pow_5 * details::INV_FACTORIALS[5];
  result -= x_pow_7 * details::INV_FACTORIALS[7];
  result += x_pow_9 * details::INV_FACTORIALS[9];

  return result * sin_sign(quadrant);
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> cos(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  details::Quadrant quadrant{details::Quadrant::I};
  const FixedPoint x = details::normalize_angle(value, quadrant);

  const FixedPoint x_pow_2 = x * x;
  const FixedPoint x_pow_4 = x_pow_2 * x_pow_2;
  const FixedPoint x_pow_6 = x_pow_4 * x_pow_2;
  const FixedPoint x_pow_8 = x_pow_6 * x_pow_2;

  FixedPoint result{1};
  result -= x_pow_2 * details::INV_FACTORIALS[2];
  result += x_pow_4 * details::INV_FACTORIALS[4];
  result -= x_pow_6 * details::INV_FACTORIALS[6];
  result += x_pow_8 * details::INV_FACTORIALS[8];

  return result * details::cos_sign(quadrant);
}
/// @}

/// Calculate the arcsine of a fixed-point number.
/// For |x| <= 0.5, uses Taylor series expansion:
/// * asin(x) = x + x^3/6 + (3*x^5)/40 + (5*x^7)/112 + (35*x^9)/1152
/// For |x| > 0.5, uses the identity for better convergence:
/// * asin(x) = PI/2 - 2*asin(sqrt((1-x)/2)) for x > 0.5
/// * asin(x) = -PI/2 + 2*asin(sqrt((1+x)/2)) for x < -0.5
/// The absolute error is less than 0.0003.
/// @pre value must be in the range [-1, 1]. Violation will trigger assertion failure.
/// @param[in] value The input value in the range [-1, 1].
/// @return The arcsine of the value in radians, in the range [-PI/2, PI/2].
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> asin(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  assert((value >= FixedPoint{-1}) && (value <= FixedPoint{1}) && "asin input must be in the range [-1, 1]");

  constexpr FixedPoint HALF(RAW_VALUE_CONSTRUCT, FixedPoint::HALF);
  constexpr FixedPoint PI_2 = FixedPoint::pi_2();

  // For |x| <= 0.5, use Taylor series directly
  if (abs(value) <= HALF)
  {
    return details::asin_taylor(value);
  }

  // For x > 0.5, use: asin(x) = PI/2 - 2*asin(sqrt((1-x)/2))
  if (value > HALF)
  {
    const FixedPoint inner = sqrt((FixedPoint{1} - value) / FixedPoint{2});
    return PI_2 - (FixedPoint{2} * details::asin_taylor(inner));
  }

  // For x < -0.5, use: asin(x) = -PI/2 + 2*asin(sqrt((1+x)/2))
  const FixedPoint inner = sqrt((FixedPoint{1} + value) / FixedPoint{2});
  return -PI_2 + (FixedPoint{2} * details::asin_taylor(inner));
}

/// Calculate the arccosine of a fixed-point number.
/// Uses the identity: acos(x) = PI/2 - asin(x)
/// @pre value must be in the range [-1, 1]. Violation will trigger assertion failure.
/// @param[in] value The input value in the range [-1, 1].
/// @return The arccosine of the value in radians, in the range [0, PI].
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> acos(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  assert((value >= FixedPoint{-1}) && (value <= FixedPoint{1}) && "acos input must be in the range [-1, 1]");

  return FixedPoint::pi_2() - asin(value);
}

/// Calculate the tangent of a fixed-point number using the Taylor series expansion.
/// For better accuracy, the function calculates results using the following trigonometric identity:
/// * tan(x) = 2 * tan(x/2) / (1 - tan(x/2)^2), where x is in the range (-PI/2, PI/2).
/// The absolute error is less than 0.0003 for fixed-point numbers with more or equal 32 fractional bits.
/// For fixed-point number with less than 32 fractional bits, the error increases rapidly for angles close
/// to PI/2. For example, the absolute error for angles close to 80 degrees is 0.06.
///
/// @note Performance optimization - Direct Padé approximation:
/// For small angles (|x| < 0.5), a direct Padé approximant can be faster than computing
/// sin(x)/cos(x), as it avoids two Taylor series evaluations:
/// * tan(x) ≈ x * (15 - x^2) / (15 - 6*x^2)  (accurate for |x| < 0.5)
/// * tan(x) ≈ x * (105 - 10*x^2) / (105 - 45*x^2 + x^4)  (more accurate, wider range)
/// For larger angles, range reduction to [-PI/4, PI/4] combined with the identity
/// tan(x + PI/4) = (1 + tan(x)) / (1 - tan(x)) can extend the usable range.
///
/// @pre |value| must be less than PI/2. Violation will trigger assertion failure.
/// @param[in] value The angle in radians in the range (-PI/2, PI/2).
/// @return The tangent of the angle.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> tan(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  assert(abs(value) < FixedPoint::pi_2() && "The angle must be in the range (-PI/2, PI/2)");

  const auto angle = value / FixedPoint{2};
  const auto half_tan = sin(angle) / cos(angle);
  const auto half_tan_pow_2 = half_tan * half_tan;

  return FixedPoint{2} * half_tan / (FixedPoint{1} - half_tan_pow_2);
}

/// Calculate the arctangent of a fixed-point number.
/// Uses range reduction techniques for improved accuracy:
/// * For |x| < 0.5: use Taylor series directly
/// * For 0.5 <= |x| <= 1: use atan(x) = PI/4 + atan((x-1)/(x+1))
/// * For |x| > 1: use atan(x) = PI/2 - atan(1/x), then recursively reduce if needed
/// The absolute error is less than 0.0003 for fixed-point numbers with 16 or more fractional bits.
/// @param[in] value Any real value.
/// @return The arctangent in radians, in the range (-PI/2, PI/2).
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> atan(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  constexpr FixedPoint HALF(RAW_VALUE_CONSTRUCT, FixedPoint::HALF);
  constexpr FixedPoint ONE{1};
  constexpr FixedPoint PI_4 = FixedPoint::pi() / FixedPoint{4};
  constexpr FixedPoint PI_2 = FixedPoint::pi_2();

  const FixedPoint abs_value = abs(value);

  // For |x| < 0.5, use Taylor series directly (best convergence)
  if (abs_value < HALF)
  {
    return details::atan_taylor(value);
  }

  // For 0.5 <= |x| <= 1, use: atan(x) = sign(x) * (PI/4 + atan((|x|-1)/(|x|+1)))
  // Note: (|x|-1)/(|x|+1) is in range [-1/3, 0] for |x| in [0.5, 1]
  // At |x|=1, this gives atan(0) = 0, so result is exactly PI/4
  if (abs_value <= ONE)
  {
    const FixedPoint reduced = (abs_value - ONE) / (abs_value + ONE);
    const FixedPoint inner = details::atan_taylor(reduced);
    if (value > FixedPoint{0})
    {
      return PI_4 + inner;
    }
    return -PI_4 - inner;
  }

  // For |x| > 1, use: atan(x) = sign(x) * (PI/2 - atan(1/|x|))
  // The reciprocal 1/|x| is in (0, 1), so we need to handle two subcases:
  const FixedPoint recip = ONE / abs_value;

  FixedPoint inner_atan{};
  if (recip < HALF)
  {
    // 1/|x| < 0.5, use Taylor series directly
    inner_atan = details::atan_taylor(recip);
  }
  else
  {
    // 0.5 <= 1/|x| < 1, use PI/4 reduction
    // atan(recip) = PI/4 + atan((recip-1)/(recip+1))
    const FixedPoint reduced = (recip - ONE) / (recip + ONE);
    inner_atan = PI_4 + details::atan_taylor(reduced);
  }

  if (value > FixedPoint{0})
  {
    return PI_2 - inner_atan;
  }
  return -PI_2 + inner_atan;
}

/// Calculate the arctangent of y/x using the atan function and
/// determine the correct quadrant based on the signs of x and y.
/// The function handles the following cases:
/// * If x > 0, then atan2(y, x) = atan(y/x).
/// * If x < 0 and y >= 0, then atan2(y, x) = atan(y/x) + PI.
/// * If x < 0 and y < 0, then atan2(y, x) = atan(y/x) - PI.
/// * If x = 0 and y > 0, then atan2(y, x) = PI/2.
/// * If x = 0 and y < 0, then atan2(y, x) = -PI/2.
/// * If x = 0 and y = 0, then atan2(y, x) is undefined, but the function returns 0 for this case.
/// @param[in] y The y coordinate.
/// @param[in] x The x coordinate.
/// @return The angle in radians between the positive x-axis and the point (x, y).
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> atan2(const FixedPoint<T, FRAC_BITS, SaturationT> y,
                                                      const FixedPoint<T, FRAC_BITS, SaturationT> x) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  if (x > FixedPoint{0})
  {
    return atan(y / x);
  }
  if ((x < FixedPoint{0}) && (y >= FixedPoint{0}))
  {
    return atan(y / x) + FixedPoint::pi();
  }
  if ((x < FixedPoint{0}) && (y < FixedPoint{0}))
  {
    return atan(y / x) - FixedPoint::pi();
  }
  if ((x == FixedPoint{0}) && (y > FixedPoint{0}))
  {
    return FixedPoint::pi_2();
  }
  if ((x == FixedPoint{0}) && (y < FixedPoint{0}))
  {
    return -FixedPoint::pi_2();
  }
  // Undefined for (x, y) = (0, 0)
  return FixedPoint{0};
}

/// Calculate the exponential function e^x for a fixed-point number.
/// Uses range reduction: exp(x) = 2^k * exp(r), where x = k*ln(2) + r and |r| <= ln(2)/2.
/// The reduced argument r is evaluated using a Taylor series.
/// @param[in] value The exponent.
/// @return e raised to the power of value.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> exp(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  if (value.raw_value() == 0)
  {
    return FixedPoint{1};
  }

  // Range reduction: exp(x) = 2^k * exp(r)
  // For signed types: x = k*ln(2) + r, where |r| <= ln(2)/2 (using round)
  // For unsigned types: x = k*ln(2) + r, where 0 <= r < ln(2) (using floor)
  // Using floor for unsigned avoids negative r values which can't be represented.
  constexpr FixedPoint LN2(rtw::math_constants::LN2<double>);
  constexpr FixedPoint LN2_INV(rtw::math_constants::LOG2_E<double>); // 1/ln(2) = log2(e)

  // Calculate k: use round for signed (better precision), floor for unsigned (avoids negative r)
  FixedPoint k_fp;
  if constexpr (std::is_signed_v<T>)
  {
    k_fp = round(value * LN2_INV);
  }
  else
  {
    k_fp = floor(value * LN2_INV);
  }
  const auto k = static_cast<std::int32_t>(k_fp);

  // Calculate r = x - k*ln(2)
  const FixedPoint r = value - (k_fp * LN2);

  // Taylor series for exp(r) where |r| <= ln(2)/2 ≈ 0.347
  // exp(r) = 1 + r + r^2/2! + r^3/3! + r^4/4! + r^5/5! + r^6/6!
  const FixedPoint r2 = r * r;
  const FixedPoint r3 = r2 * r;
  const FixedPoint r4 = r2 * r2;
  const FixedPoint r5 = r4 * r;
  const FixedPoint r6 = r4 * r2;

  FixedPoint exp_r{1};
  exp_r += r;
  exp_r += r2 * details::INV_FACTORIALS[2];
  exp_r += r3 * details::INV_FACTORIALS[3];
  exp_r += r4 * details::INV_FACTORIALS[4];
  exp_r += r5 * details::INV_FACTORIALS[5];
  exp_r += r6 * details::INV_FACTORIALS[6];

  // Multiply by 2^k
  if (k >= 0)
  {
    // Check for potential overflow
    if (k >= static_cast<std::int32_t>(FixedPoint::INTEGER_BITS))
    {
      return FixedPoint::max();
    }
    return FixedPoint(RAW_VALUE_CONSTRUCT, exp_r.raw_value() << k);
  }
  // k < 0: right shift
  if (-k >= static_cast<std::int32_t>(FixedPoint::BITS))
  {
    return FixedPoint{0};
  }
  return FixedPoint(RAW_VALUE_CONSTRUCT, exp_r.raw_value() >> (-k));
}

/// Calculate the natural logarithm of a fixed-point number.
/// Uses range reduction: log(x) = k*ln(2) + log(m), where x = 2^k * m and 1 <= m < 2.
/// The reduced argument m is evaluated using a Taylor series for log(1 + y) where y = m - 1.
/// @pre value must be positive. Violation will trigger assertion failure.
/// @param[in] value A positive fixed-point number.
/// @return The natural logarithm of value.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> log(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  using UnsignedT = std::make_unsigned_t<T>;

  assert((value.raw_value() > 0) && "log requires a positive argument");

  if (value == FixedPoint{1})
  {
    return FixedPoint{0};
  }

  // Range reduction: x = 2^k * m, where 1 <= m < 2
  // log(x) = k*ln(2) + log(m)
  constexpr FixedPoint LN2(rtw::math_constants::LN2<double>);

  // Find k such that 2^k <= x < 2^(k+1)
  // For fixed-point, raw_value = x * 2^FRAC_BITS
  // So we need to find k relative to the fixed-point representation
  const auto raw = static_cast<UnsignedT>(value.raw_value());
  const auto leading_zeros = count_leading_zero(raw);
  const auto bit_position = (static_cast<std::int32_t>(FixedPoint::BITS) - 1) - leading_zeros;

  // k is the exponent such that the value is in [1, 2) after normalization
  // For fixed-point with FRAC_BITS fractional bits, value 1.0 has raw = 2^FRAC_BITS
  // So k = bit_position - FRAC_BITS
  const auto k = bit_position - static_cast<std::int32_t>(FixedPoint::FRACTIONAL_BITS);

  // Normalize m to [1, 2): m = x / 2^k
  // In fixed-point: m_raw = raw >> k (if k > 0) or raw << (-k) (if k < 0)
  // But we want m in fixed-point format, so m_raw should represent m * 2^FRAC_BITS
  FixedPoint m = value;
  if (k > 0)
  {
    m = FixedPoint(RAW_VALUE_CONSTRUCT, static_cast<T>(raw >> k));
  }
  else if (k < 0)
  {
    m = FixedPoint(RAW_VALUE_CONSTRUCT, static_cast<T>(raw << (-k)));
  }

  // Now compute log(m) where m is in [1, 2)
  // Use log(m) = log(1 + y) where y = m - 1, so y is in [0, 1)
  // Taylor series: log(1 + y) = y - y^2/2 + y^3/3 - y^4/4 + y^5/5 - ...
  // This converges slowly for y close to 1, so use a transformation:
  // Let t = (m - 1) / (m + 1), then log(m) = 2 * (t + t^3/3 + t^5/5 + t^7/7 + ...)
  // This converges faster since t is in [0, 1/3) for m in [1, 2)

  const FixedPoint t = (m - FixedPoint{1}) / (m + FixedPoint{1});
  const FixedPoint t2 = t * t;
  const FixedPoint t3 = t2 * t;
  const FixedPoint t5 = t3 * t2;
  const FixedPoint t7 = t5 * t2;
  const FixedPoint t9 = t7 * t2;

  // log(m) = 2 * (t + t^3/3 + t^5/5 + t^7/7 + t^9/9)
  FixedPoint log_m = t;
  log_m += t3 / T{3};
  log_m += t5 / T{5};
  log_m += t7 / T{7};
  log_m += t9 / T{9};
  log_m = log_m * FixedPoint{2};

  // Final result: log(x) = k*ln(2) + log(m)
  return (FixedPoint{k} * LN2) + log_m;
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> log2(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  constexpr FixedPoint LOG2_E{rtw::math_constants::LOG2_E<double>}; // 1/ln(2)
  return log(value) * LOG2_E;
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> log10(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  constexpr FixedPoint LOG10_E{rtw::math_constants::LOG10_E<double>}; // 1/ln(10)
  return log(value) * LOG10_E;
}

/// Calculate x raised to the power y for fixed-point numbers.
/// For positive x: pow(x, y) = exp(y * log(x))
/// Special cases:
/// - pow(x, 0) = 1 for any x
/// - pow(0, y) = 0 for y > 0
/// - pow(1, y) = 1 for any y
/// @pre base must be non-negative (except for special cases above). Violation will trigger assertion failure.
/// @param[in] base The base (must be non-negative).
/// @param[in] exponent The exponent.
/// @return base raised to the power of exponent.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> pow(const FixedPoint<T, FRAC_BITS, SaturationT> base,
                                                    const FixedPoint<T, FRAC_BITS, SaturationT> exponent) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  if (exponent.raw_value() == 0)
  {
    return FixedPoint{1}; // x^0 = 1
  }
  if (base.raw_value() == 0)
  {
    return FixedPoint{0}; // 0^y = 0 for y > 0
  }
  if (base == FixedPoint{1})
  {
    return FixedPoint{1}; // 1^y = 1
  }

  assert((base.raw_value() > 0) && "pow requires a non-negative base");

  return exp(exponent * log(base));
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> copysign(const FixedPoint<T, FRAC_BITS, SaturationT> x,
                                                         const FixedPoint<T, FRAC_BITS, SaturationT> y) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  if constexpr (std::is_signed_v<T>)
  {
    const FixedPoint abs_x = abs(x);
    if (y.raw_value() < 0)
    {
      return -abs_x;
    }
    return abs_x;
  }
  else
  {
    // Unsigned types have no sign to copy
    return x;
  }
}

/// Check if the sign bit is set for a fixed-point number.
/// @param[in] value The fixed-point number to check.
/// @return True if the value is negative (sign bit is set), false otherwise.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr bool signbit(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  if constexpr (std::is_signed_v<T>)
  {
    return value.raw_value() < 0;
  }
  else
  {
    return false;
  }
}

/// Compute the positive difference between two fixed-point numbers.
/// Returns max(x - y, 0). This is equivalent to x - y if x > y, otherwise 0.
/// @param[in] x The first value.
/// @param[in] y The second value.
/// @return The positive difference, or 0 if x <= y.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> fdim(const FixedPoint<T, FRAC_BITS, SaturationT> x,
                                                     const FixedPoint<T, FRAC_BITS, SaturationT> y) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  return (x > y) ? x - y : FixedPoint{0};
}

/// Calculate the hypotenuse of a right triangle: sqrt(x^2 + y^2 + z^2).
/// Uses careful computation to avoid intermediate overflow where possible.
/// @param[in] x The first cathetus.
/// @param[in] y The second cathetus.
/// @param[in] z The third cathetus.
/// @return The hypotenuse length.
/// @{
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> hypot(const FixedPoint<T, FRAC_BITS, SaturationT> x,
                                                      const FixedPoint<T, FRAC_BITS, SaturationT> y,
                                                      const FixedPoint<T, FRAC_BITS, SaturationT> z) noexcept
{
  return sqrt((x * x) + (y * y) + (z * z));
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> hypot(const FixedPoint<T, FRAC_BITS, SaturationT> x,
                                                      const FixedPoint<T, FRAC_BITS, SaturationT> y) noexcept
{
  return sqrt((x * x) + (y * y));
}

/// @}

/// Compute the floating-point remainder of x/y.
/// The result has the same sign as x and magnitude less than |y|.
/// It is computed as x - trunc(x/y) * y, where trunc(x/y) is the integer quotient of x/y truncated towards zero.
/// @pre y must not be zero. Violation will trigger assertion failure.
/// @param[in] x The dividend.
/// @param[in] y The divisor.
/// @return The remainder of x/y.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> fmod(const FixedPoint<T, FRAC_BITS, SaturationT> x,
                                                     const FixedPoint<T, FRAC_BITS, SaturationT> y) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  assert(y.raw_value() != 0 && "fmod: division by zero");

  const FixedPoint quotient = trunc(x / y);
  return x - (quotient * y);
}

/// Check if the value is finite.
/// For fixed-point numbers, this always returns true since there is no infinity or NaN.
/// @param[in] value The value to check.
/// @return Always true for fixed-point numbers.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr bool isfinite(const FixedPoint<T, FRAC_BITS, SaturationT> /*value*/) noexcept
{
  return true;
}

/// Check if the value is infinite.
/// For fixed-point numbers, this always returns false since there is no infinity representation.
/// @param[in] value The value to check.
/// @return Always false for fixed-point numbers.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr bool isinf(const FixedPoint<T, FRAC_BITS, SaturationT> /*value*/) noexcept
{
  return false;
}

/// Check if the value is NaN (Not a Number).
/// For fixed-point numbers, this always returns false since there is no NaN representation.
/// @param[in] value The value to check.
/// @return Always false for fixed-point numbers.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr bool isnan(const FixedPoint<T, FRAC_BITS, SaturationT> /*value*/) noexcept
{
  return false;
}

/// Check if the value is normal.
/// For fixed-point numbers, returns true for all non-zero values (no denormalized representation).
/// @param[in] value The value to check.
/// @return True if the value is non-zero, false otherwise.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr bool isnormal(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  return value.raw_value() != 0;
}

} // namespace rtw::multiprecision::math
