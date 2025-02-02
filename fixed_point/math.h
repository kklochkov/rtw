#pragma once

#include "fixed_point/fixed_point.h"

namespace rtw::fixed_point::math
{

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> abs(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  return FixedPoint(FixedPoint::PRIVATE_CTOR, std::abs(value.value_));
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> floor(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round down
  return FixedPoint(FixedPoint::PRIVATE_CTOR, value.value_ & FixedPoint::INTEGER_MASK);
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> ceil(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round up
  return FixedPoint(FixedPoint::PRIVATE_CTOR, (value.value_ + FixedPoint::FRACTION_MASK) & FixedPoint::INTEGER_MASK);
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> round(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  // Clear the fractional part and round to the nearest integer
  return FixedPoint(FixedPoint::PRIVATE_CTOR, (value.value_ + FixedPoint::HALF) & FixedPoint::INTEGER_MASK);
}

/// Calculate the square root of a fixed-point number using the Heron's (Babylonian) method.
/// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Heron's_method
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sqrt(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;
  if (value.value_ == 0)
  {
    return value;
  }

  assert((value.value_ > 0) && "Cannot calculate the square root of a negative number");

  constexpr FixedPoint HALF(FixedPoint::PRIVATE_CTOR, FixedPoint::HALF);

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

constexpr double ONE_OVER_TWO_FACTORIAL = 1.0 / factorial(2U);
constexpr double ONE_OVER_THREE_FACTORIAL = 1.0 / factorial(3U);
constexpr double ONE_OVER_FOUR_FACTORIAL = 1.0 / factorial(4U);
constexpr double ONE_OVER_FIVE_FACTORIAL = 1.0 / factorial(5U);
constexpr double ONE_OVER_SIX_FACTORIAL = 1.0 / factorial(6U);
constexpr double ONE_OVER_SEVEN_FACTORIAL = 1.0 / factorial(7U);

} // namespace details

/// Calculate the sine/cosine of a fixed-point number using the Taylor series expansion.
/// We take first four terms of the Taylor series expansion and reduce the input angle to the range [-PI/2, PI/2].
/// sin(x) = x - x^3/3! + x^5/5! - x^7/7!
/// cos(x) = 1 - x^2/2! + x^4/4! - x^6/6!
/// where x is in the range [-PI/2, PI/2].
/// The absolute error is less than 0.000897.
/// Side note: an alternative approach is to use Chebyshev polynomials or Bhaskara I's sine approximation formula.
/// Robin Green's "Even Faster Math Functions" provides a good overview of the different approaches.
/// https://basesandframes.wordpress.com/2020/04/04/even-faster-math-functions/
/// @param value The angle in radians. The angle must be in the range [-PI/2, PI/2].
/// @return The sine of the angle.
/// @{
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sin(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  const FixedPoint x_pow_2 = value * value;
  const FixedPoint x_pow_3 = value * x_pow_2;
  const FixedPoint x_pow_5 = x_pow_3 * x_pow_2;
  const FixedPoint x_pow_7 = x_pow_5 * x_pow_2;

  FixedPoint result = value;
  result -= x_pow_3 * details::ONE_OVER_THREE_FACTORIAL;
  result += x_pow_5 * details::ONE_OVER_FIVE_FACTORIAL;
  result -= x_pow_7 * details::ONE_OVER_SEVEN_FACTORIAL;

  return result;
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> cos(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  const FixedPoint x_pow_2 = value * value;
  const FixedPoint x_pow_4 = x_pow_2 * x_pow_2;
  const FixedPoint x_pow_6 = x_pow_4 * x_pow_2;

  FixedPoint result{1};
  result -= x_pow_2 * details::ONE_OVER_TWO_FACTORIAL;
  result += x_pow_4 * details::ONE_OVER_FOUR_FACTORIAL;
  result -= x_pow_6 * details::ONE_OVER_SIX_FACTORIAL;

  return result;
}
/// @}

} // namespace rtw::fixed_point::math
