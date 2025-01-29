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

} // namespace rtw::fixed_point::math
