#pragma once

#include "math/vector.h"

namespace rtw::math {

template <typename T, std::uint16_t N>
class Point : Matrix<T, N, 1>
{
public:
  using Matrix = Matrix<T, N, 1>;
  using Vector = Vector<T, N>;

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
  constexpr explicit Point(const T x, const T y) : Matrix(x, y)
  {
  }

  template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, T> && (N == 3)>>
  constexpr explicit Point(const T x, const T y, const T z) : Matrix(x, y, z)
  {
  }

  /// Homogeneous 3D space constructor.
  /// @param x The x coordinate.
  /// @param y The y coordinate.
  /// @param z The z coordinate.
  /// @param w The w coordinate. 1 by default to represent a position.
  template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, T> && (N == 4)>>
  constexpr explicit Point(const T x, const T y, const T z, const T w = T{1}) : Matrix(x, y, z, w)
  {
  }

  template <typename U = T, std::uint16_t M, typename = std::enable_if_t<std::is_same_v<U, T> && (M <= N)>>
  constexpr explicit Point(const Point<U, M>& point) : Matrix(point.as_matrix())
  {
    operator[](M) = U{1}; // Set the last element to 1 to represent a position.
  }

  constexpr explicit Point(const Matrix& matrix) : Matrix(matrix) {}
  constexpr explicit Point(const Vector& vector) : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const { return Vector(as_matrix()); }
  constexpr explicit operator Matrix() const { return as_matrix(); }

  template <typename U = value_type, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr Point<U, N> cast() const noexcept
  {
    return Point<U, N>{as_matrix().template cast<U>()};
  }

  /// Swizzle operators.
  /// @{
  constexpr Point<value_type, 2> xy() const noexcept { return Point<value_type, 2>{as_matrix().xy()}; }

  constexpr Point<value_type, 3> xyz() const noexcept { return Point<value_type, 3>{as_matrix().xyz()}; }
  /// @}

  constexpr Point& operator+=(const Vector& vector)
  {
    as_matrix() += vector.as_matrix();
    return *this;
  }

  constexpr Point& operator-=(const Vector& vector)
  {
    as_matrix() -= vector.as_matrix();
    return *this;
  }

  constexpr Point& operator*=(const value_type rhs)
  {
    as_matrix() *= rhs;
    return *this;
  }

  constexpr Point& operator/=(const value_type rhs)
  {
    as_matrix() /= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr inline Point operator+(const Point& point, const Vector& vector)
  {
    return Point{point.as_matrix() + vector.as_matrix()};
  }

  friend constexpr inline Point operator-(const Point& point, const Vector& vector)
  {
    return Point{point.as_matrix() - vector.as_matrix()};
  }

  friend constexpr inline Vector operator-(const Point& lhs, const Point& rhs)
  {
    return Vector{lhs.as_matrix() - rhs.as_matrix()};
  }

  friend constexpr inline bool operator==(const Point& lhs, const Point& rhs)
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr inline bool operator!=(const Point& lhs, const Point& rhs) { return !(lhs == rhs); }

  friend inline std::ostream& operator<<(std::ostream& os, const Point& point)
  {
    os << "Point" << N;
    return point.as_matrix().operator<<(os);
  }
  /// @}
};

template <typename T, std::uint16_t N, std::uint16_t M, std::uint16_t P, typename = std::enable_if_t<(P <= M)>>
constexpr inline Point<T, N> operator*(const Matrix<T, N, M>& lhs, const Point<T, P>& rhs)
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
using Point2f = Point2<float>;
using Point2d = Point2<double>;
using Point2i = Point2<std::int32_t>;

/// 3D space aliases.
template <typename T>
using Point3 = Point<T, 3>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;
using Point3i = Point3<std::int32_t>;

/// Homogeneous 3D space aliases.
template <typename T>
using Point4 = Point<T, 4>;
using Point4f = Point4<float>;
using Point4d = Point4<double>;
using Point4i = Point4<std::int32_t>;

} // namespace rtw::math
