#pragma once

#include "math/convex_polygon.h"
#include "math/frustum.h"
#include "math/plane.h"
#include "math/vector_operations.h"

#include "sw_renderer/vertex.h"

namespace rtw::sw_renderer
{

template <typename T, std::size_t CAPACITY>
using ConvexPolygonVertex = math::ConvexPolygon<T, Vertex, CAPACITY>;

template <typename T>
using TriangleVertex = ConvexPolygonVertex<T, 3>;

/// Clips a triangle against the frustum using the Sutherland-Hodgman algorithm.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// @param polygon The polygon to clip.
/// @param plane The plane to clip against.
/// @return The clipped polygon.
template <typename T, std::size_t CAPACITY = 8U>
constexpr ConvexPolygonVertex<T, CAPACITY> clip_against_plane(const ConvexPolygonVertex<T, CAPACITY>& polygon,
                                                              const math::Plane3<T>& plane) noexcept
{
  ConvexPolygonVertex<T, CAPACITY> clipped_result;
  for (std::size_t i = 0U; i < polygon.size(); ++i)
  {
    const auto& current = polygon.at(i);
    const auto& next = polygon.next(i);

    const auto current_n_dot =
        math::dot(static_cast<math::Vector3<T>>(current.point.xyz()), plane.normal) + plane.distance;
    const auto next_n_dot = math::dot(static_cast<math::Vector3<T>>(next.point.xyz()), plane.normal) + plane.distance;

    if (current_n_dot >= 0.0F)
    {
      clipped_result.push_back(current);
    }

    if (current_n_dot * next_n_dot < 0.0F)
    {
      const auto t = current_n_dot / (current_n_dot - next_n_dot);
      Vertex<T> intersection;
      intersection.point = math::lerp(current.point, next.point, t);
      intersection.color = lerp(current.color, next.color, static_cast<float>(t));
      intersection.tex_coord = lerp(current.tex_coord, next.tex_coord, t);
      clipped_result.push_back(intersection);
    }
  }
  return clipped_result;
}

/// Clips a triangle against the frustum using the Sutherland-Hodgman algorithm.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// @param p0 The first vertex of the triangle.
/// @param p1 The second vertex of the triangle.
/// @param p2 The third vertex of the triangle.
/// @param frustum The frustum to clip against.
/// @return The clipped triangle.
template <typename T, std::size_t CAPACITY = 8U>
constexpr ConvexPolygonVertex<T, CAPACITY> clip(const Vertex<T>& v0, const Vertex<T>& v1, const Vertex<T>& v2,
                                                const math::Frustum3<T>& frustum) noexcept
{
  ConvexPolygonVertex<T, CAPACITY> result;
  result.push_back(v0);
  result.push_back(v1);
  result.push_back(v2);

  result = clip_against_plane(result, frustum.left);
  result = clip_against_plane(result, frustum.right);
  result = clip_against_plane(result, frustum.top);
  result = clip_against_plane(result, frustum.bottom);
  result = clip_against_plane(result, frustum.near);
  result = clip_against_plane(result, frustum.far);
  return result;
}

/// The result of the triangulation of a convex polygon.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// @tparam T The type of the polygon's vertices.
template <typename T, std::size_t CAPACITY = 8U>
struct TriangulationResult
{
  std::array<TriangleVertex<T>, CAPACITY> triangles;
  std::size_t triangle_count{};
};

/// Triangulates a convex polygon.
/// Resulting triangles have counter-clockwise winding order.
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// @param polygon The polygon to triangulate.
/// @return The triangulation result.
template <typename T, std::size_t CAPACITY = 8U>
constexpr TriangulationResult<T, CAPACITY> triangulate(const ConvexPolygonVertex<T, CAPACITY>& polygon) noexcept
{
  TriangulationResult<T, CAPACITY> result;

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
