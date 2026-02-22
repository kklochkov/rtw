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

} // namespace rtw::math
