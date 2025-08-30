#pragma once

#include "math/point.h"
#include "multiprecision/fixed_point.h"

namespace rtw::math
{

/// Linearly interpolates between two values.
/// @param a The first value.
/// @param b The second value.
/// @param t The interpolation factor.
/// @return The interpolated value.
template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
constexpr T lerp(const T a, const T b, const T t) noexcept
{
  return a + (b - a) * t;
}

/// Linearly interpolates between two matrices.
/// @param a The first matrix.
/// @param b The second matrix.
/// @param t The interpolation factor.
/// @return The interpolated matrix.
template <typename T, std::uint16_t N, std::uint16_t M>
constexpr Matrix<T, N, M> lerp(const Matrix<T, N, M>& a, const Matrix<T, N, M>& b, const T t) noexcept
{
  Matrix<T, N, M> result{math::UNINITIALIZED};
  for (std::uint32_t i = 0U; i < a.size(); ++i)
  {
    result[i] = lerp(a[i], b[i], t);
  }
  return result;
}

/// Linearly interpolates between two vectors.
/// @param a The first vector.
/// @param b The second vector.
/// @param t The interpolation factor.
/// @return The interpolated vector.
template <typename T, std::uint16_t N>
constexpr Vector<T, N> lerp(const Vector<T, N>& a, const Vector<T, N>& b, const T t) noexcept
{
  return Vector<T, N>{lerp(a.as_matrix(), b.as_matrix(), t)};
}

/// Linearly interpolates between two points.
/// @param a The first point.
/// @param b The second point.
/// @param t The interpolation factor.
/// @return The interpolated point.
template <typename T, std::uint16_t N>
constexpr Point<T, N> lerp(const Point<T, N>& a, const Point<T, N>& b, const T t) noexcept
{
  return Point<T, N>{lerp(a.as_matrix(), b.as_matrix(), t)};
}

} // namespace rtw::math
