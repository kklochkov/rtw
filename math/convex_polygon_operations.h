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

template <typename T>
constexpr T triangle_orientation(const Point2<T>& v0, const Point2<T>& v1, const Point2<T>& v2) noexcept
{
  return cross(v1 - v0, v2 - v0);
}

/// Check the winding order of a triangle.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @return The winding order of the triangle.
template <typename T>
constexpr WindingOrder winding_order(const Point2<T>& v0, const Point2<T>& v1, const Point2<T>& v2) noexcept
{
  return triangle_orientation(v0, v1, v2) > T{0} ? WindingOrder::COUNTER_CLOCKWISE : WindingOrder::CLOCKWISE;
}

template <typename T>
constexpr WindingOrder winding_order(const Triangle2<T>& triangle) noexcept
{
  return winding_order(triangle[0U], triangle[1U], triangle[2U]);
}

/// Get the default near-zero epsilon for a given type.
/// For fixed-point types and integral types, the epsilon is zero.
/// @tparam T The type of the elements.
/// @return The default near-zero epsilon for the type.
template <typename T>
constexpr T default_near_zero_epsilon() noexcept
{
  if constexpr (multiprecision::IS_FIXED_POINT_V<T>)
  {
    return T{0};
  }
  else
  {
    return std::numeric_limits<T>::epsilon();
  }
}

enum class ConvexityCheckResult : std::uint8_t
{
  CONVEX,
  CONCAVE,
  INVALID_POLYGON,
  COLLINEAR_POINTS
};

/// Check if a convex polygon is indeed convex.
/// @tparam T The type of the elements.
/// @tparam CAPACITY The capacity of the polygon.
/// @param[in] polygon The convex polygon to check.
/// @param[in] epsilon The near-zero epsilon to use for floating-point comparisons.
/// @return The result of the convexity check.
template <typename T, std::size_t CAPACITY>
constexpr ConvexityCheckResult is_convex(const ConvexPolygon2<T, CAPACITY>& polygon,
                                         const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  if (!polygon.valid())
  {
    return ConvexityCheckResult::INVALID_POLYGON;
  }

  using multiprecision::math::abs;
  using std::abs;

  T reference_orientation{0};

  for (std::size_t i = 0U; i < polygon.size(); ++i)
  {
    const auto v0 = polygon[i];
    const auto v1 = polygon[(i + 1U) % polygon.size()];
    const auto v2 = polygon[(i + 2U) % polygon.size()];
    const auto current_orientation = triangle_orientation(v0, v1, v2);
    if (abs(current_orientation) <= epsilon)
    {
      return ConvexityCheckResult::COLLINEAR_POINTS;
    }

    if (abs(reference_orientation) <= epsilon)
    {
      reference_orientation = current_orientation;
    }
    else if ((reference_orientation > epsilon) != (current_orientation > epsilon))
    {
      return ConvexityCheckResult::CONCAVE;
    }
  }

  return ConvexityCheckResult::CONVEX;
}

} // namespace rtw::math
