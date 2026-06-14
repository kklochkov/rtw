#pragma once

#include "math/convex_polygon.h"
#include "stl/span.h"

namespace rtw::sw_renderer
{

/// Clips a convex polygon against a single half-space using the Sutherland-Hodgman algorithm.
///
/// Generic over the vertex and plane representation: it works for any pair that provides the customization points
/// `signed_distance(vertex, plane)` (non-negative inside) and `lerp(vertex, vertex, t)`
///
/// @tparam T The scalar component type.
/// @tparam VertexT The vertex template.
/// @tparam PlaneT The half-space representation, e.g. `math::Plane3<T>` or a homogeneous `math::Vector4<T>` plane.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// Default is 9, which is the maximum number of vertices a triangle can have
/// after clipping against all 6 frustum planes (3 original + 6 new = 9).
/// @param[in] polygon The polygon to clip.
/// @param[in] plane The half-space to clip against.
/// @return The clipped polygon.
template <typename T, template <typename> typename VertexT, template <typename> typename PlaneT,
          std::size_t CAPACITY = 9U>
constexpr math::ConvexPolygon<T, VertexT, CAPACITY>
clip_against_plane(const math::ConvexPolygon<T, VertexT, CAPACITY>& polygon, const PlaneT<T>& plane) noexcept
{
  math::ConvexPolygon<T, VertexT, CAPACITY> clipped_result;
  for (std::size_t i = 0U; i < polygon.size(); ++i)
  {
    const auto& current = polygon.at(i);
    const auto& next = polygon.next(i);

    const auto current_distance = signed_distance(current, plane);
    const auto next_distance = signed_distance(next, plane);

    if (static_cast<T>(current_distance) >= T{0})
    {
      clipped_result.push_back(current);
    }

    if (static_cast<T>(current_distance * next_distance) < T{0})
    {
      const auto t = static_cast<T>(current_distance / (current_distance - next_distance));
      clipped_result.push_back(lerp(current, next, t));
    }
  }
  return clipped_result;
}

/// Clips a triangle against the frustum using the Sutherland-Hodgman algorithm.
///
/// @tparam T The scalar component type.
/// @tparam VertexT The vertex template.
/// @tparam PlaneT The half-space representation, e.g. `math::Plane3<T>` or a homogeneous `math::Vector4<T>` plane.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// Default is 9, which is the maximum number of vertices a triangle can have
/// after clipping against all 6 frustum planes (3 original + 6 new = 9).
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @param[in] frustum The frustum to clip against. Must have 6 planes: left, right, top, bottom, near, far.
/// @return The clipped triangle.
template <typename T, template <typename> typename VertexT, template <typename> typename PlaneT,
          std::size_t CAPACITY = 9U>
constexpr math::ConvexPolygon<T, VertexT, CAPACITY> clip(const VertexT<T>& v0, const VertexT<T>& v1,
                                                         const VertexT<T>& v2,
                                                         const stl::Span<const PlaneT<T>> frustum) noexcept
{
  assert((frustum.size() == 6U) && "Frustum must have 6 planes: left, right, top, bottom, near, far.");

  math::ConvexPolygon<T, VertexT, CAPACITY> result;
  result.push_back(v0);
  result.push_back(v1);
  result.push_back(v2);

  for (const auto& plane : frustum)
  {
    result = clip_against_plane(result, plane);
  }

  return result;
}

template <typename T, template <typename> typename VertexT>
using TriangleVertex = math::ConvexPolygon<T, VertexT, 3>;

/// The result of the triangulation of a convex polygon.
///
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// Default is 9, which is the maximum number of vertices a triangle can have
/// after clipping against all 6 frustum planes (3 original + 6 new = 9).
/// @tparam T The type of the polygon's vertices.
/// @tparam VertexT The vertex type of the polygon's vertices.
template <typename T, template <typename> typename VertexT, std::size_t CAPACITY = 9U>
struct TriangulationResult
{
  std::array<TriangleVertex<T, VertexT>, CAPACITY> triangles;
  std::size_t triangle_count{};
};

/// Triangulates a convex polygon.
///
/// Resulting triangles have counter-clockwise winding order.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// Default is 9, which is the maximum number of vertices a triangle can have
/// after clipping against all 6 frustum planes (3 original + 6 new = 9).
/// @tparam VertexT The vertex type of the polygon's vertices.
/// @param[in] polygon The polygon to triangulate.
/// @return The triangulation result.
template <typename T, template <typename> typename VertexT, std::size_t CAPACITY = 9U>
constexpr TriangulationResult<T, VertexT, CAPACITY>
triangulate(const math::ConvexPolygon<T, VertexT, CAPACITY>& polygon) noexcept
{
  TriangulationResult<T, VertexT, CAPACITY> result;

  if (polygon.is_valid())
  {
    result.triangle_count = polygon.size() - 2U;
    for (std::size_t i = 0U; i < result.triangle_count; ++i)
    {
      result.triangles[i] = {polygon.next(i + 1U), polygon.next(i), polygon.at(0)};
    }
  }

  return result;
}

} // namespace rtw::sw_renderer
