#pragma once

#include "math/vector.h"

namespace rtw::math
{

template <typename T, std::uint16_t N>
class Point;

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const Point<T, N>& point) noexcept;

template <typename T, std::uint16_t N>
class Point : Matrix<T, N, 1>
{
public:
  using Matrix = Matrix<T, N, 1>;
  using Vector = Vector<T, N>;

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

  constexpr Point() noexcept = default;

  constexpr explicit Point(UninitializedTag tag) noexcept : Matrix(tag) {}
  constexpr Point(InitializeWithValueTag tag, const T value) noexcept : Matrix(tag, value) {}

  template <typename U = T, typename = std::enable_if_t<std::is_convertible_v<U, T> && (N == 2)>>
  constexpr Point(const T x, const T y) noexcept : Matrix(x, y)
  {
  }

  template <typename U = T, typename = std::enable_if_t<std::is_convertible_v<U, T> && (N == 3)>>
  constexpr Point(const T x, const T y, const T z) noexcept : Matrix(x, y, z)
  {
  }

  /// Homogeneous 3D space constructor.
  /// @param x The x coordinate.
  /// @param y The y coordinate.
  /// @param z The z coordinate.
  /// @param w The w coordinate. 1 by default to represent a position.
  template <typename U = T, typename = std::enable_if_t<std::is_convertible_v<U, T> && (N == 4)>>
  constexpr Point(const T x, const T y, const T z, const T w = T{1}) noexcept : Matrix(x, y, z, w)
  {
  }

  template <typename U = T, std::uint16_t M, typename = std::enable_if_t<std::is_convertible_v<U, T> && (M <= N)>>
  constexpr explicit Point(const Point<U, M>& point) noexcept : Matrix(point.as_matrix())
  {
    operator[](M) = U{1}; // Set the last element to 1 to represent a position.
  }

  constexpr explicit Point(const Matrix& matrix) noexcept : Matrix(matrix) {}
  constexpr explicit Point(const Vector& vector) noexcept : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const noexcept { return Vector(as_matrix()); }
  constexpr explicit operator Matrix() const noexcept { return as_matrix(); }

  template <typename U = value_type, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<U>>>
  constexpr Point<U, N> cast() const noexcept
  {
    return Point<U, N>{as_matrix().template cast<U>()};
  }

  /// Swizzle operators.
  /// @{
  constexpr Point<value_type, 2> xy() const noexcept { return Point<value_type, 2>{as_matrix().xy()}; }

  constexpr Point<value_type, 3> xyz() const noexcept { return Point<value_type, 3>{as_matrix().xyz()}; }
  /// @}

  constexpr Point& operator+=(const Vector& vector) noexcept
  {
    as_matrix() += vector.as_matrix();
    return *this;
  }

  constexpr Point& operator-=(const Vector& vector) noexcept
  {
    as_matrix() -= vector.as_matrix();
    return *this;
  }

  constexpr Point& operator*=(const value_type rhs) noexcept
  {
    as_matrix() *= rhs;
    return *this;
  }

  constexpr Point& operator/=(const value_type rhs) noexcept
  {
    as_matrix() /= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Point operator+(const Point& point, const Vector& vector) noexcept
  {
    return Point{point.as_matrix() + vector.as_matrix()};
  }

  friend constexpr Point operator-(const Point& point, const Vector& vector) noexcept
  {
    return Point{point.as_matrix() - vector.as_matrix()};
  }

  friend constexpr Vector operator-(const Point& lhs, const Point& rhs) noexcept
  {
    return Vector{lhs.as_matrix() - rhs.as_matrix()};
  }

  friend constexpr bool operator==(const Point& lhs, const Point& rhs) noexcept
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr bool operator!=(const Point& lhs, const Point& rhs) noexcept { return !(lhs == rhs); }
  /// @}

  friend std::ostream& operator<< <T, N>(std::ostream& os, const Point& point) noexcept;
};

template <typename T, std::uint16_t N, std::uint16_t M, std::uint16_t P, MemoryOrder MEMORY_ORDER,
          typename = std::enable_if_t<(P <= M)>>
constexpr Point<T, N> operator*(const Matrix<T, N, M, MEMORY_ORDER>& lhs, const Point<T, P>& rhs) noexcept
{
  if constexpr (M == P)
  {
    return Point<T, M>{lhs * rhs.as_matrix()};
  }
  else
  {
    return Point<T, M>{lhs * Point<T, M>(rhs).as_matrix()};
  }
}

/// 2D space aliases.
template <typename T>
using Point2 = Point<T, 2>;
using Point2F = Point2<float>;
using Point2D = Point2<double>;
using Point2I = Point2<std::int32_t>;
using Point2Q16 = Point2<multiprecision::FixedPoint16>;
using Point2Q32 = Point2<multiprecision::FixedPoint32>;

/// 3D space aliases.
template <typename T>
using Point3 = Point<T, 3>;
using Point3F = Point3<float>;
using Point3D = Point3<double>;
using Point3I = Point3<std::int32_t>;
using Point3Q16 = Point3<multiprecision::FixedPoint16>;
using Point3Q32 = Point3<multiprecision::FixedPoint32>;

/// Homogeneous 3D space aliases.
template <typename T>
using Point4 = Point<T, 4>;
using Point4F = Point4<float>;
using Point4D = Point4<double>;
using Point4I = Point4<std::int32_t>;
using Point4Q16 = Point4<multiprecision::FixedPoint16>;
using Point4Q32 = Point4<multiprecision::FixedPoint32>;

} // namespace rtw::math
