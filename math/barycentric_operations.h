#pragma once

#include "math/barycentric.h"
#include "math/convex_polygon.h"
#include "math/point.h"
#include "math/vector_operations.h"

namespace rtw::math
{

/// Make a barycentric coordinate from a point and a triangle.
/// The winding order is counter-clockwise.
/// The coordinate system is right-handed.
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
/// The coordinate system is right-handed.
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
/// The coordinate system is right-handed.
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
