#pragma once

#include "math/matrix.h"

namespace rtw::math
{

template <typename T, std::uint16_t N>
class Vector;

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vector) noexcept;

template <typename T, std::uint16_t N>
class Vector : Matrix<T, N, 1>
{
public:
  using Matrix = Matrix<T, N, 1>;

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

  constexpr Vector() noexcept = default;

  constexpr explicit Vector(UninitializedTag tag) noexcept : Matrix(tag) {}
  constexpr Vector(InitializeWithValueTag tag, const T value) noexcept : Matrix(tag, value) {}

  template <typename U = T, typename = std::enable_if_t<std::is_convertible_v<U, T> && (N == 2)>>
  constexpr Vector(const U x, const U y) noexcept : Matrix(x, y)
  {
  }

  template <typename U = T, typename = std::enable_if_t<std::is_convertible_v<U, T> && (N == 3)>>
  constexpr Vector(const U x, const U y, const U z) noexcept : Matrix(x, y, z)
  {
  }

  /// Homogeneous 3D space constructor.
  /// @param x The x coordinate.
  /// @param y The y coordinate.
  /// @param z The z coordinate.
  /// @param w The w coordinate. 0 by default to represent a direction.
  template <typename U = T, typename = std::enable_if_t<std::is_convertible_v<U, T> && (N == 4)>>
  constexpr Vector(const U x, const U y, const U z, const U w = U{0}) noexcept : Matrix(x, y, z, w)
  {
  }

  template <typename U = T, std::uint16_t M, typename = std::enable_if_t<std::is_convertible_v<U, T> && (M <= N)>>
  constexpr explicit Vector(const Vector<U, M>& vector) noexcept : Matrix(vector.as_matrix())
  {
    operator[](M) = U{0}; // Set the last element to 0 to represent a direction.
  }

  constexpr explicit Vector(const Matrix& matrix) noexcept : Matrix(matrix) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  template <typename U = value_type, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<U>>>
  constexpr Vector<U, N> cast() const noexcept
  {
    return Vector<U, N>{as_matrix().template cast<U>()};
  }

  /// Swizzle operators.
  /// @{
  constexpr Vector<value_type, 2> xy() const noexcept { return Vector<value_type, 2>{as_matrix().xy()}; }

  constexpr Vector<value_type, 3> xyz() const noexcept { return Vector<value_type, 3>{as_matrix().xyz()}; }
  /// @}

  constexpr Vector& operator+=(const Vector& rhs) noexcept
  {
    as_matrix() += rhs.as_matrix();
    return *this;
  }

  constexpr Vector& operator-=(const Vector& rhs) noexcept
  {
    as_matrix() -= rhs.as_matrix();
    return *this;
  }

  constexpr Vector& operator/=(const value_type rhs) noexcept
  {
    as_matrix() /= rhs;
    return *this;
  }

  constexpr Vector& operator*=(const value_type rhs) noexcept
  {
    as_matrix() *= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Vector operator+(const Vector& lhs, const Vector& rhs) noexcept
  {
    return Vector{lhs.as_matrix() + rhs.as_matrix()};
  }

  friend constexpr Vector operator-(const Vector& lhs, const Vector& rhs) noexcept
  {
    return Vector{lhs.as_matrix() - rhs.as_matrix()};
  }

  friend constexpr Vector operator*(const Vector& lhs, const value_type rhs) noexcept
  {
    return Vector{lhs.as_matrix() * rhs};
  }

  friend constexpr Vector operator*(const value_type lhs, const Vector& rhs) noexcept { return rhs * lhs; }

  friend constexpr Vector operator/(const Vector& lhs, const value_type rhs) noexcept
  {
    return Vector{lhs.as_matrix() / rhs};
  }

  friend constexpr bool operator==(const Vector& lhs, const Vector& rhs) noexcept
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr bool operator!=(const Vector& lhs, const Vector& rhs) noexcept { return !(lhs == rhs); }

  friend constexpr Vector operator-(const Vector& vector) noexcept { return Vector{-vector.as_matrix()}; }
  /// @}

  friend std::ostream& operator<< <T, N>(std::ostream& os, const Vector& vector) noexcept;
};

/// 2D space aliases.
template <typename T>
using Vector2 = Vector<T, 2>;
using Vector2F = Vector2<float>;
using Vector2D = Vector2<double>;
using Vector2I = Vector2<std::int32_t>;
using Vector2Q16 = Vector2<multiprecision::FixedPoint16>;
using Vector2Q32 = Vector2<multiprecision::FixedPoint32>;

/// 3D space aliases.
template <typename T>
using Vector3 = Vector<T, 3>;
using Vector3F = Vector3<float>;
using Vector3D = Vector3<double>;
using Vector3I = Vector3<std::int32_t>;
using Vector3Q16 = Vector3<multiprecision::FixedPoint16>;
using Vector3Q32 = Vector3<multiprecision::FixedPoint32>;

/// Homogeneous 3D space aliases.
template <typename T>
using Vector4 = Vector<T, 4>;
using Vector4F = Vector4<float>;
using Vector4D = Vector4<double>;
using Vector4I = Vector4<std::int32_t>;
using Vector4Q16 = Vector4<multiprecision::FixedPoint16>;
using Vector4Q32 = Vector4<multiprecision::FixedPoint32>;

} // namespace rtw::math
