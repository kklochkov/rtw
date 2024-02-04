#pragma once

#include "math/point.h"

#include "sw_renderer/barycentric.h"

namespace rtw::sw_renderer {

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
constexpr inline Barycentric<T> make_barycentric(const math::Point2<T>& v0, const math::Point2<T>& v1,
                                                 const math::Point2<T>& v2, const math::Point2<T>& p)
{
  const auto a = v2 - v1;
  const auto b = v0 - v2;
  const auto c = v1 - v0;
  const auto pa = p - v1;
  const auto pb = p - v2;
  const auto pc = p - v0;
  const auto area = math::cross(a, b);
  const auto alpha = math::cross(a, pa) / area;
  const auto beta = math::cross(b, pb) / area;
  const auto gamma = math::cross(c, pc) / area;
  return Barycentric<T>{alpha, beta, gamma};
}

/// Check if a point is inside or on the edge of a triangle.
/// The winding order is counter-clockwise.
/// The cooridate system is right-handed.
/// @tparam T The type of the elements.
/// @param[in] b The barycentric coordinate.
/// @return True if the point is inside the triangle, false otherwise.
template <typename T>
constexpr inline bool contains(const Barycentric<T>& b)
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
constexpr inline bool contains(const math::Point2<T>& v0, const math::Point2<T>& v1, const math::Point2<T>& v2,
                               const math::Point2<T>& p)
{
  const auto b = make_barycentric(v0, v1, v2, p);
  return contains(b);
}

template <typename T>
constexpr inline bool is_top_left(const math::Vector2<T>& edge)
{
  const bool is_top = edge.y() == T{0} && edge.x() < T{0};
  const bool is_left = edge.y() > T{0};
  return is_top || is_left;
}

enum class WindingOrder
{
  CounterClockwise,
  Clockwise
};

/// Check the winding order of a triangle.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @return The winding order of the triangle.
template <typename T>
constexpr WindingOrder winding_order(const math::Point2<T>& v0, const math::Point2<T>& v1, const math::Point2<T>& v2)
{
  return math::cross(v1 - v0, v2 - v1) > T{0} ? WindingOrder::CounterClockwise : WindingOrder::Clockwise;
}

} // namespace rtw::sw_renderer
