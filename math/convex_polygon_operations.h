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
/// @note Doesn't handle collinear points. Use check_polygon for robust checks.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @return The winding order of the triangle.
/// @{
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
/// @}

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

struct PolygonInfo
{
  enum class Property : std::uint8_t
  {
    CONVEX,
    CONCAVE,
    INVALID_POLYGON,
    COLLINEAR_POINTS
  };

  constexpr explicit PolygonInfo(const Property property = Property::CONVEX,
                                 const WindingOrder winding_order = WindingOrder::COUNTER_CLOCKWISE) noexcept
      : property{property}, winding_order{winding_order}
  {
  }

  constexpr bool is_convex() const noexcept { return property == Property::CONVEX; }
  constexpr bool is_concave() const noexcept { return property == Property::CONCAVE; }
  constexpr bool is_invalid() const noexcept { return property == Property::INVALID_POLYGON; }
  constexpr bool has_collinear_points() const noexcept { return property == Property::COLLINEAR_POINTS; }

  Property property{Property::CONVEX};
  WindingOrder winding_order{WindingOrder::COUNTER_CLOCKWISE};
};

/// Check the properties of a given polygon.
/// @tparam T The type of the elements.
/// @tparam CAPACITY The capacity of the polygon.
/// @param[in] polygon The polygon to check.
/// @param[in] epsilon The near-zero epsilon to use for floating-point comparisons.
/// @return The result of the polygon check.
template <typename T, std::size_t CAPACITY>
constexpr PolygonInfo check_polygon(const ConvexPolygon2<T, CAPACITY>& polygon,
                                    const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  if (!polygon.is_valid())
  {
    return PolygonInfo{PolygonInfo::Property::INVALID_POLYGON};
  }

  using multiprecision::math::abs;
  using std::abs;

  T reference_orientation{0};
  WindingOrder winding_order{WindingOrder::COUNTER_CLOCKWISE};

  for (std::size_t i = 0U; i < polygon.size(); ++i)
  {
    const auto v0 = polygon[i];
    const auto v1 = polygon[(i + 1U) % polygon.size()];
    const auto v2 = polygon[(i + 2U) % polygon.size()];
    const auto current_orientation = triangle_orientation(v0, v1, v2);
    if (abs(current_orientation) <= epsilon)
    {
      return PolygonInfo{PolygonInfo::Property::COLLINEAR_POINTS, WindingOrder::COUNTER_CLOCKWISE};
    }

    if (abs(reference_orientation) <= epsilon)
    {
      reference_orientation = current_orientation;
      winding_order = current_orientation > T{0} ? WindingOrder::COUNTER_CLOCKWISE : WindingOrder::CLOCKWISE;
    }
    else if ((reference_orientation > epsilon) != (current_orientation > epsilon))
    {
      return PolygonInfo{PolygonInfo::Property::CONCAVE, winding_order};
    }
  }

  return PolygonInfo{PolygonInfo::Property::CONVEX, winding_order};
}

/// Check if a convex polygon is indeed convex.
/// @tparam T The type of the elements.
/// @tparam CAPACITY The capacity of the polygon.
/// @param[in] polygon The convex polygon to check.
/// @param[in] epsilon The near-zero epsilon to use for floating-point comparisons.
/// @return True if the polygon is convex, false otherwise.
template <typename T, std::size_t CAPACITY>
constexpr bool is_convex(const ConvexPolygon2<T, CAPACITY>& polygon,
                         const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  return check_polygon(polygon, epsilon).is_convex();
}

/// Get the winding order of a convex polygon.
/// @tparam T The type of the elements.
/// @tparam CAPACITY The capacity of the polygon.
/// @param[in] polygon The convex polygon.
/// @param[in] epsilon The near-zero epsilon to use for floating-point comparisons.
/// @return The winding order of the polygon.
template <typename T, std::size_t CAPACITY, typename = std::enable_if_t<(CAPACITY > 3U)>>
constexpr WindingOrder winding_order(const ConvexPolygon2<T, CAPACITY>& polygon,
                                     const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  return check_polygon(polygon, epsilon).winding_order;
}

} // namespace rtw::math
