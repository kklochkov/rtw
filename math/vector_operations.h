#pragma once

#include "math/matrix_operations.h"
#include "math/vector.h"

namespace rtw::math
{

template <typename T, std::uint16_t N, std::uint16_t M, std::uint16_t P, MemoryOrder MEMORY_ORDER,
          typename = std::enable_if_t<(P <= M)>>
constexpr Vector<T, N> operator*(const Matrix<T, N, M, MEMORY_ORDER>& lhs, const Vector<T, P>& rhs) noexcept
{
  if constexpr (M == P)
  {
    return Vector<T, M>{lhs * rhs.as_matrix()};
  }
  else
  {
    return Vector<T, M>{lhs * Vector<T, M>(rhs).as_matrix()};
  }
}

template <typename T, std::uint16_t N>
constexpr T dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
{
  return dot(lhs.as_matrix(), rhs.as_matrix());
}

template <typename T, std::uint16_t N>
constexpr T norm2(const Vector<T, N>& vector) noexcept
{
  return dot(vector, vector);
}

template <typename T, std::uint16_t N>
constexpr T norm(const Vector<T, N>& vector) noexcept
{
  return norm(vector.as_matrix());
}

template <typename T, MemoryOrder MEMORY_ORDER = DEFAULT_MEMORY_ORDER>
constexpr T cross(const Vector2<T>& lhs, const Vector2<T>& rhs) noexcept
{
  // clang-format off
  return determinant(Matrix2x2<T, MEMORY_ORDER>{FROM_ROW_MAJOR,
      lhs.x(), lhs.y(),
      rhs.x(), rhs.y(),
  });
  // clang-format on
}

template <typename T>
constexpr Vector3<T> cross(const Vector3<T>& lhs, const Vector3<T>& rhs) noexcept
{
  const auto x = lhs.y() * rhs.z() - lhs.z() * rhs.y();
  const auto y = lhs.z() * rhs.x() - lhs.x() * rhs.z();
  const auto z = lhs.x() * rhs.y() - lhs.y() * rhs.x();
  return Vector3<T>{x, y, z};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> normalize(const Vector<T, N>& vector) noexcept
{
  return Vector<T, N>{normalize(vector.as_matrix())};
}

template <typename T, std::uint16_t N>
constexpr T scalar_projection(const Vector<T, N>& vector, const Vector<T, N>& onto) noexcept
{
  const auto onto_norm = norm(onto);
  assert(onto_norm != T{0});
  return dot(vector, onto) / onto_norm;
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> vector_projection(const Vector<T, N>& vector, const Vector<T, N>& onto) noexcept
{
  const auto onto_norm2 = norm2(onto);
  assert(onto_norm2 != T{0});
  return (dot(vector, onto) / onto_norm2) * onto;
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> hadamard(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
{
  return Vector<T, N>{hadamard(lhs.as_matrix(), rhs.as_matrix())};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> min(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
{
  return Vector<T, N>{min(lhs.as_matrix(), rhs.as_matrix())};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> min(const Vector<T, N>& lhs, const T rhs) noexcept
{
  return Vector<T, N>{min(lhs.as_matrix(), rhs)};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> max(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
{
  return Vector<T, N>{max(lhs.as_matrix(), rhs.as_matrix())};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> max(const Vector<T, N>& lhs, const T rhs) noexcept
{
  return Vector<T, N>{max(lhs.as_matrix(), rhs)};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> clamp(const Vector<T, N>& value, const Vector<T, N>& lo, const Vector<T, N>& hi) noexcept
{
  return Vector<T, N>{clamp(value.as_matrix(), lo.as_matrix(), hi.as_matrix())};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> clamp(const Vector<T, N>& value, const T lo, const T hi) noexcept
{
  return Vector<T, N>{clamp(value.as_matrix(), lo, hi)};
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> saturate(const Vector<T, N>& value) noexcept
{
  return clamp(value, T{0}, T{1});
}

template <typename T, std::uint16_t N>
constexpr Vector<T, N> fract(const Vector<T, N>& value) noexcept
{
  return Vector<T, N>{fract(value.as_matrix())};
}

} // namespace rtw::math
