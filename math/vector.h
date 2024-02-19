#pragma once

#include "math/matrix.h"

#include <cmath>

namespace rtw::math {

template <typename T, std::uint16_t N>
class Vector : Matrix<T, N, 1>
{
public:
  using Matrix = Matrix<T, N, 1>;

  using Matrix::Matrix;
  using typename Matrix::reference;
  using typename Matrix::value_type;
  using Matrix::operator[];
  using Matrix::operator+=;
  using Matrix::operator-=;
  using Matrix::operator*=;
  using Matrix::operator/=;
  using Matrix::begin;
  using Matrix::cbegin;
  using Matrix::cend;
  using Matrix::data;
  using Matrix::end;
  using Matrix::size;
  using Matrix::w;
  using Matrix::x;
  using Matrix::y;
  using Matrix::z;

  template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, T> && (N == 2)>>
  constexpr explicit Vector(const U x, const U y) : Matrix(x, y)
  {
  }

  template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, T> && (N == 3)>>
  constexpr explicit Vector(const U x, const U y, const U z) : Matrix(x, y, z)
  {
  }

  /// Homogeneous 3D space constructor.
  /// @param x The x coordinate.
  /// @param y The y coordinate.
  /// @param z The z coordinate.
  /// @param w The w coordinate. 0 by default to represent a direction.
  template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, T> && (N == 4)>>
  constexpr explicit Vector(const U x, const U y, const U z, const U w = U{0}) : Matrix(x, y, z, w)
  {
  }

  template <typename U = T, std::uint16_t M, typename = std::enable_if_t<std::is_same_v<U, T> && (M <= N)>>
  constexpr explicit Vector(const Vector<U, M>& vector) : Matrix(vector.as_matrix())
  {
    operator[](M) = U{0}; // Set the last element to 0 to represent a direction.
  }

  constexpr explicit Vector(const Matrix& matrix) : Matrix(matrix) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  template <typename U = value_type, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr Vector<U, N> cast() const noexcept
  {
    return Vector<U, N>{as_matrix().template cast<U>()};
  }

  /// Swizzle operators.
  /// @{
  constexpr Vector<value_type, 2> xy() const noexcept { return Vector<value_type, 2>{as_matrix().xy()}; }

  constexpr Vector<value_type, 3> xyz() const noexcept { return Vector<value_type, 3>{as_matrix().xyz()}; }
  /// @}

  constexpr Vector& operator+=(const Vector& rhs)
  {
    as_matrix() += rhs.as_matrix();
    return *this;
  }

  constexpr Vector& operator-=(const Vector& rhs)
  {
    as_matrix() -= rhs.as_matrix();
    return *this;
  }

  constexpr Vector& operator/=(const value_type rhs)
  {
    as_matrix() /= rhs;
    return *this;
  }

  constexpr Vector& operator*=(const value_type rhs)
  {
    as_matrix() *= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr inline Vector operator+(const Vector& lhs, const Vector& rhs)
  {
    return Vector{lhs.as_matrix() + rhs.as_matrix()};
  }

  friend constexpr inline Vector operator-(const Vector& lhs, const Vector& rhs)
  {
    return Vector{lhs.as_matrix() - rhs.as_matrix()};
  }

  friend constexpr inline Vector operator*(const Vector& lhs, const value_type rhs)
  {
    return Vector{lhs.as_matrix() * rhs};
  }

  friend constexpr inline Vector operator*(const value_type lhs, const Vector& rhs) { return rhs * lhs; }

  friend constexpr inline Vector operator/(const Vector& lhs, const value_type rhs)
  {
    return Vector{lhs.as_matrix() / rhs};
  }

  friend constexpr inline bool operator==(const Vector& lhs, const Vector& rhs)
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr inline bool operator!=(const Vector& lhs, const Vector& rhs) { return !(lhs == rhs); }

  friend constexpr inline Vector operator-(const Vector& vector) { return Vector{-vector.as_matrix()}; }

  friend inline std::ostream& operator<<(std::ostream& os, const Vector& vector)
  {
    os << "Vector" << N;
    return vector.as_matrix().operator<<(os);
  }
  /// @}
};

/// 2D space aliases.
template <typename T>
using Vector2 = Vector<T, 2>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<std::int32_t>;

/// 3D space aliases.
template <typename T>
using Vector3 = Vector<T, 3>;
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<std::int32_t>;

/// Homogeneous 3D space aliases.
template <typename T>
using Vector4 = Vector<T, 4>;
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;
using Vector4i = Vector4<std::int32_t>;

template <typename T, std::uint16_t N, std::uint16_t M, std::uint16_t P, typename = std::enable_if_t<(P <= M)>>
constexpr inline Vector<T, N> operator*(const Matrix<T, N, M>& lhs, const Vector<T, P>& rhs)
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
constexpr inline T dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
{
  T result{0};
  for (std::uint16_t i = 0U; i < N; ++i)
  {
    result += lhs[i] * rhs[i];
  }
  return result;
}

template <typename T, std::uint16_t N>
constexpr inline T norm2(const Vector<T, N>& vector)
{
  return dot(vector, vector);
}

template <typename T, std::uint16_t N>
inline T norm(const Vector<T, N>& vector)
{
  return std::sqrt(norm2(vector));
}

template <typename T>
constexpr inline T cross(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
  return determinant(Matrix2x2<T>{lhs.x(), lhs.y(), rhs.x(), rhs.y()});
}

template <typename T>
constexpr inline Vector3<T> cross(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
  const auto x = lhs.y() * rhs.z() - lhs.z() * rhs.y();
  const auto y = lhs.z() * rhs.x() - lhs.x() * rhs.z();
  const auto z = lhs.x() * rhs.y() - lhs.y() * rhs.x();
  return Vector3<T>{x, y, z};
}

template <typename T, std::uint16_t N>
inline Vector<T, N> normalize(const Vector<T, N>& vector)
{
  return vector / norm(vector);
}

} // namespace rtw::math
