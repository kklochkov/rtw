#pragma once

#include "math/convex_polygon.h"
#include "math/point.h"

namespace rtw::math
{

template <typename T>
class Barycentric;

template <typename T>
std::ostream& operator<<(std::ostream& os, const Barycentric<T>& coord) noexcept;

template <typename T>
class Barycentric : Matrix<T, 3, 1>
{
public:
  using Matrix = Matrix<T, 3, 1>;
  using Vector = Vector<T, 3>;

  using typename Matrix::reference;
  using typename Matrix::value_type;
  using Matrix::operator[];
  using Matrix::begin;
  using Matrix::cbegin;
  using Matrix::cend;
  using Matrix::data;
  using Matrix::end;

  constexpr Barycentric() noexcept = default;

  constexpr explicit Barycentric(UninitializedTag tag) noexcept : Matrix(tag) {}
  constexpr Barycentric(InitializeWithValueTag tag, const T value) noexcept : Matrix(tag, value) {}

  constexpr Barycentric(const T w0, const T w1, const T w2) noexcept : Matrix(w0, w1, w2) {}

  constexpr explicit Barycentric(const Matrix& matrix) noexcept : Matrix(matrix) {}
  constexpr explicit Barycentric(const Vector& vector) noexcept : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const noexcept { return Vector{as_matrix()}; }
  constexpr explicit operator Matrix() const noexcept { return as_matrix(); }

  constexpr value_type w0() const noexcept { return operator[](0); }
  constexpr reference w0() noexcept { return operator[](0); }

  constexpr value_type lambda0() const noexcept { return w0(); }
  constexpr reference lambda0() noexcept { return w0(); }

  constexpr value_type alpha() const noexcept { return w0(); }
  constexpr reference alpha() noexcept { return w0(); }

  constexpr value_type w1() const noexcept { return operator[](1); }
  constexpr reference w1() noexcept { return operator[](1); }

  constexpr value_type lambda1() const noexcept { return w1(); }
  constexpr reference lambda1() noexcept { return w1(); }

  constexpr value_type beta() const noexcept { return w1(); }
  constexpr reference beta() noexcept { return w1(); }

  constexpr value_type w2() const noexcept { return operator[](2); }
  constexpr reference w2() noexcept { return operator[](2); }

  constexpr value_type lambda2() const noexcept { return w2(); }
  constexpr reference lambda2() noexcept { return w2(); }

  constexpr value_type gamma() const noexcept { return w2(); }
  constexpr reference gamma() noexcept { return w2(); }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr bool operator==(const Barycentric& lhs, const Barycentric& rhs) noexcept
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr bool operator!=(const Barycentric& lhs, const Barycentric& rhs) noexcept { return !(lhs == rhs); }
  /// @}

  friend std::ostream& operator<< <T>(std::ostream& os, const Barycentric& coord) noexcept;
};

using BarycentricF = Barycentric<float>;
using BarycentricD = Barycentric<double>;
using BarycentricQ16 = Barycentric<multiprecision::FixedPoint16>;
using BarycentricQ32 = Barycentric<multiprecision::FixedPoint32>;

/// Make a barycentric coordinate from a point and a triangle.
/// The winding order is counter-clockwise.
/// The cooridate system is right-handed.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @param[in] p The point.
/// @return The barycentric coordinate.
template <typename T>
constexpr Barycentric<T> make_barycentric(const Point2<T>& v0, const Point2<T>& v1, const Point2<T>& v2,
                                          const Point2<T>& p) noexcept
{
  const auto a = v2 - v1;
  const auto b = v0 - v2;
  const auto c = v1 - v0;
  const auto pa = p - v1;
  const auto pb = p - v2;
  const auto pc = p - v0;
  const auto area = cross(a, b);
  const auto alpha = cross(a, pa) / area;
  const auto beta = cross(b, pb) / area;
  const auto gamma = cross(c, pc) / area;
  return Barycentric<T>{alpha, beta, gamma};
}

template <typename T>
constexpr Barycentric<T> make_barycentric(const Triangle2<T>& triangle, const Point2<T>& p) noexcept
{
  return make_barycentric(triangle[0U], triangle[1U], triangle[2U], p);
}

/// Check if a point is inside or on the edge of a triangle.
/// The winding order is counter-clockwise.
/// The cooridate system is right-handed.
/// @tparam T The type of the elements.
/// @param[in] b The barycentric coordinate.
/// @return True if the point is inside the triangle, false otherwise.
template <typename T>
constexpr bool contains(const Barycentric<T>& b) noexcept
{
  return b.alpha() >= T{0} && b.beta() >= T{0} && b.gamma() >= T{0};
}

/// Check if a point is inside or on the edge of a triangle.
/// The winding order is counter-clockwise.
/// The cooridate system is right-handed.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @param[in] p The point.
/// @return True if the point is inside the triangle, false otherwise.
template <typename T>
constexpr bool contains(const Point2<T>& v0, const Point2<T>& v1, const Point2<T>& v2, const Point2<T>& p) noexcept
{
  const auto b = make_barycentric(v0, v1, v2, p);
  return contains(b);
}

template <typename T>
constexpr bool contains(const Triangle2<T>& triangle, const Point2<T>& p) noexcept
{
  return contains(triangle[0U], triangle[1U], triangle[2U], p);
}

} // namespace rtw::math
