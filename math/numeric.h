#pragma once

#include "multiprecision//math.h"
#include "multiprecision/fixed_point.h"

namespace rtw::math
{

/// Get the default near-zero epsilon for a given type.
/// For fixed-point types and integral types, the epsilon is zero.
/// @tparam T The type of the elements.
/// @return The default near-zero epsilon for the type.
template <typename T>
constexpr T default_near_zero_epsilon() noexcept
{
  if constexpr (multiprecision::IS_FIXED_POINT_V<T>)
  {
    return T{0};
  }
  else
  {
    return std::numeric_limits<T>::epsilon();
  }
}

/// Default tolerance for convergence in the eigenvalue algorithm.
/// The default tolerance is set to 100 times the machine epsilon for the floating-point type T,
/// for fixed-point types it is set to 1, since the epsilon for fixed-point types is exactly 1.
/// @tparam T The floating-point or fixed-point type.
/// @return The default tolerance.
template <typename T>
constexpr T default_tolerance() noexcept
{
  T tolerance_factor{100U};
  if constexpr (multiprecision::IS_FIXED_POINT_V<T>)
  {
    tolerance_factor = 1U;
  }
  return tolerance_factor * std::numeric_limits<T>::epsilon();
}

/// Check if a value is near zero within a given epsilon.
/// @tparam T The type of the elements.
/// @param[in] value The value to check.
/// @param[in] epsilon The near-zero epsilon to use for the comparison.
/// @return True if the value is near zero, false otherwise.
template <typename T>
constexpr bool is_near_zero(const T value, const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  using multiprecision::math::abs;
  using std::abs;

  return abs(value) <= epsilon;
}

/// Check if two values are nearly equal within a given epsilon.
/// @tparam T The type of the elements.
/// @param[in] lhs The left-hand side value.
/// @param[in] rhs The right-hand side value.
/// @param[in] epsilon The near-equal epsilon to use for the comparison.
/// @return True if the values are nearly equal, false otherwise.
template <typename T>
constexpr bool is_near_equal(const T lhs, const T rhs, const T epsilon = default_tolerance<T>()) noexcept
{
  using multiprecision::math::abs;
  using std::abs;

  return abs(lhs - rhs) <= epsilon;
}

} // namespace rtw::math
