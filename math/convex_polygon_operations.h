#pragma once

#include "math/convex_polygon.h"
#include "math/vector_operations.h"

namespace rtw::math
{

enum class WindingOrder : std::uint8_t
{
  COUNTER_CLOCKWISE,
  CLOCKWISE
};

/// Check the winding order of a triangle.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @return The winding order of the triangle.
template <typename T>
constexpr WindingOrder winding_order(const Point2<T>& v0, const Point2<T>& v1, const Point2<T>& v2) noexcept
{
  return cross(v1 - v0, v2 - v1) > T{0} ? WindingOrder::COUNTER_CLOCKWISE : WindingOrder::CLOCKWISE;
}

template <typename T>
constexpr WindingOrder winding_order(const Triangle2<T>& triangle) noexcept
{
  return winding_order(triangle[0U], triangle[1U], triangle[2U]);
}

} // namespace rtw::math
