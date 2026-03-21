#pragma once

#include "multiprecision/fixed_point.h"
#include "multiprecision/int128.h"
#include "multiprecision/operations.h"

#include <array>

namespace rtw::multiprecision::math
{

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT, typename = std::enable_if_t<std::is_signed_v<T>>>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> abs(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  return FixedPoint(RAW_VALUE_CONSTRUCT, std::abs(value.raw_value()));
}

template <typename T, typename = std::enable_if_t<std::is_signed_v<T>>>
constexpr Int<T> abs(const Int<T> value) noexcept
{
  return sign(value < Int<T>{0}) * value;
}

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

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> ceil(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round up
  return FixedPoint(RAW_VALUE_CONSTRUCT, (value.raw_value() + FixedPoint::FRACTION_MASK) & FixedPoint::INTEGER_MASK);
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> round(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round to the nearest integer
  return FixedPoint(RAW_VALUE_CONSTRUCT, (value.raw_value() + FixedPoint::HALF) & FixedPoint::INTEGER_MASK);
}

/// Calculate the square root of a fixed-point number using the Heron's (Babylonian) method.
/// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Heron's_method
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sqrt(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  if (value.raw_value() == 0)
  {
    return value;
  }

  assert((value.raw_value() > 0) && "Cannot calculate the square root of a negative number");

  constexpr FixedPoint HALF(RAW_VALUE_CONSTRUCT, FixedPoint::HALF);

  FixedPoint guess = value * HALF;
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
/// @param[in] value The input value in the range [-1, 1].
/// @return The arcsine of the value in radians, in the range [-PI/2, PI/2].
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> asin(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  assert((value >= FixedPoint{-1}) && (value <= FixedPoint{1}) && "asin input must be in the range [-1, 1]");

  constexpr FixedPoint HALF{0.5};
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
/// @param[in] value The angle in radians in the range (-PI/2, PI/2).
/// @return The tangent of the angle.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> tan(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  assert(abs(value) < FixedPoint::pi_2() && "The angle must be in the range (-PI/2, PI/2)");

  const auto two = FixedPoint{2};
  const auto angle = value / two;
  const auto half_tan = sin(angle) / cos(angle);
  const auto half_tan_pow_2 = half_tan * half_tan;

  return two * half_tan / (FixedPoint{1} - half_tan_pow_2);
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

  constexpr FixedPoint HALF{0.5};
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

} // namespace rtw::multiprecision::math
