#pragma once

#include "math/point.h"
#include "multiprecision/fixed_point.h"

namespace rtw::math
{

/// Linearly interpolates between two values.
/// @param[in] a The first value.
/// @param[in] b The second value.
/// @param[in] t The interpolation factor.
/// @return The interpolated value.
template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
constexpr T lerp(const T a, const T b, const T t) noexcept
{
  return a + (b - a) * t;
}

/// Linearly interpolates between two matrices.
/// @param[in] a The first matrix.
/// @param[in] b The second matrix.
/// @param[in] t The interpolation factor.
/// @return The interpolated matrix.
template <typename T, std::uint16_t N, std::uint16_t M, MemoryOrder MEMORY_ORDER>
constexpr Matrix<T, N, M, MEMORY_ORDER> lerp(const Matrix<T, N, M, MEMORY_ORDER>& a,
                                             const Matrix<T, N, M, MEMORY_ORDER>& b, const T t) noexcept
{
  Matrix<T, N, M, MEMORY_ORDER> result{math::UNINITIALIZED};
  for (std::uint32_t i = 0U; i < a.size(); ++i)
  {
    result[i] = lerp(a[i], b[i], t);
  }
  return result;
}

/// Linearly interpolates between two matrices element-wise.
/// @param[in] a The first matrix.
/// @param[in] b The second matrix.
/// @param[in] t The interpolation factor for each element.
/// @return The interpolated matrix.
template <typename T, std::uint16_t N, std::uint16_t M, MemoryOrder MEMORY_ORDER>
constexpr Matrix<T, N, M, MEMORY_ORDER> lerp(const Matrix<T, N, M, MEMORY_ORDER>& a,
                                             const Matrix<T, N, M, MEMORY_ORDER>& b,
                                             const Matrix<T, N, M, MEMORY_ORDER>& t) noexcept
{
  Matrix<T, N, M, MEMORY_ORDER> result{math::UNINITIALIZED};
  for (std::uint32_t i = 0U; i < a.size(); ++i)
  {
    result[i] = lerp(a[i], b[i], t[i]);
  }
  return result;
}

/// Linearly interpolates between two vectors.
/// @param[in] a The first vector.
/// @param[in] b The second vector.
/// @param[in] t The interpolation factor.
/// @return The interpolated vector.
template <typename T, std::uint16_t N>
constexpr Vector<T, N> lerp(const Vector<T, N>& a, const Vector<T, N>& b, const T t) noexcept
{
  return Vector<T, N>{lerp(a.as_matrix(), b.as_matrix(), t)};
}

/// Linearly interpolates between two vectors element-wise.
/// @param[in] a The first vector.
/// @param[in] b The second vector.
/// @param[in] t The interpolation factor for each element.
/// @return The interpolated vector.
template <typename T, std::uint16_t N>
constexpr Vector<T, N> lerp(const Vector<T, N>& a, const Vector<T, N>& b, const Vector<T, N>& t) noexcept
{
  return Vector<T, N>{lerp(a.as_matrix(), b.as_matrix(), t.as_matrix())};
}

/// Linearly interpolates between two points.
/// @param[in] a The first point.
/// @param[in] b The second point.
/// @param[in] t The interpolation factor.
/// @return The interpolated point.
template <typename T, std::uint16_t N>
constexpr Point<T, N> lerp(const Point<T, N>& a, const Point<T, N>& b, const T t) noexcept
{
  return Point<T, N>{lerp(a.as_matrix(), b.as_matrix(), t)};
}

} // namespace rtw::math
