#pragma once

#include "math/convex_polygon.h"
#include "math/operations.h"

#include "sw_renderer/projection.h"
#include "sw_renderer/vertex.h"

namespace rtw::sw_renderer {

template <typename T, std::size_t Capacity>
using ConvexPolygonVertex4 = math::ConvexPolygon<T, Vertex4, Capacity>;
template <std::size_t Capacity>
using ConvexPolygonVertex4f = ConvexPolygonVertex4<float, Capacity>;
template <std::size_t Capacity>
using ConvexPolygonVertex4d = ConvexPolygonVertex4<double, Capacity>;
template <std::size_t Capacity>
using ConvexPolygonVertex4i = ConvexPolygonVertex4<int, Capacity>;

template <typename T>
using TriangleVertex4 = ConvexPolygonVertex4<T, 3>;
using TriangleVertex4f = TriangleVertex4<float>;
using TriangleVertex4d = TriangleVertex4<double>;
using TriangleVertex4i = TriangleVertex4<int>;

/// Clips a triangle against the frustum using the Sutherland-Hodgman algorithm.
/// @tparam Capacity The maximum number of vertices the polygon can hold.
/// @param polygon The polygon to clip.
/// @param plane The plane to clip against.
/// @return The clipped polygon.
template <typename T, std::size_t Capacity = 8U>
constexpr ConvexPolygonVertex4<T, Capacity> clip_against_plane(const ConvexPolygonVertex4<T, Capacity>& polygon,
                                                               const Plane3<T>& plane)
{
  ConvexPolygonVertex4<T, Capacity> clipped_result;
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
      Vertex4<T> intersection;
      intersection.point = math::lerp(current.point, next.point, t);
      intersection.color = lerp(current.color, next.color, t);
      intersection.tex_coord = lerp(current.tex_coord, next.tex_coord, t);
      clipped_result.push_back(intersection);
    }
  }
  return clipped_result;
}

/// Clips a triangle against the frustum using the Sutherland-Hodgman algorithm.
/// @tparam Capacity The maximum number of vertices the polygon can hold.
/// @param p0 The first vertex of the triangle.
/// @param p1 The second vertex of the triangle.
/// @param p2 The third vertex of the triangle.
/// @param frustum The frustum to clip against.
/// @return The clipped triangle.
template <typename T, std::size_t Capacity = 8U>
constexpr ConvexPolygonVertex4<T, Capacity> clip(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2,
                                                 const Frustum3<T>& frustum)
{
  ConvexPolygonVertex4<T, Capacity> result;
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
/// @tparam Capacity The maximum number of vertices the polygon can hold.
/// @tparam T The type of the polygon's vertices.
template <typename T, std::size_t Capacity = 8U>
struct TriangulationResult
{
  std::array<TriangleVertex4<T>, Capacity> triangles;
  std::size_t triangle_count{};
};

/// Triangulates a convex polygon.
/// Resulting triangles are counter-clockwise.
/// @tparam Capacity The maximum number of vertices the polygon can hold.
/// @param polygon The polygon to triangulate.
/// @return The triangulation result.
template <typename T, std::size_t Capacity = 8U>
constexpr inline TriangulationResult<T, Capacity> triangulate(const ConvexPolygonVertex4<T, Capacity>& polygon)
{
  TriangulationResult<T, Capacity> result;

  if (polygon.valid())
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
