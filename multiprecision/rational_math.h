#pragma once

#include "multiprecision/rational.h"

namespace rtw::multiprecision::math
{

template <typename T, typename = std::enable_if_t<std::is_signed_v<T>>>
constexpr Rational<T> abs(const Rational<T> value) noexcept
{
  return Rational<T>(std::abs(value.numerator()), value.denominator());
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

} // namespace rtw::multiprecision::math
