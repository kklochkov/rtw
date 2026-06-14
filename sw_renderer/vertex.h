#pragma once

#include "math/point_operations.h"
#include "sw_renderer/color.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/tex_coord.h"

namespace rtw::sw_renderer
{

/// A vertex with position, texture coordinate, normal, and color attributes.
/// @tparam T The scalar type for geometric data (e.g., float, FixedPoint16).
template <typename T>
struct Vertex
{
  /// @param[in] point Homogeneous position in 4D space (default: origin).
  /// @param[in] tex_coord Texture coordinate (default: (0, 0)).
  /// @param[in] normal Surface normal vector (default: zero).
  /// @param[in] color Vertex color (default: transparent black).
  explicit Vertex(const math::Point4<T>& point = {}, const TexCoord<T>& tex_coord = {},
                  const math::Vector3<T>& normal = {}, const Color& color = {})
      : point{point}, tex_coord{tex_coord}, normal{normal}, color{color}
  {
  }

  math::Point4<T> point;   ///< Homogeneous position (x, y, z, w).
  TexCoord<T> tex_coord;   ///< Texture coordinate (u, v).
  math::Vector3<T> normal; ///< Surface normal.
  Color color;             ///< Vertex color.
};

using VertexF = Vertex<single_precision>; ///< Single-precision vertex.
using VertexD = Vertex<double_precision>; ///< Double-precision vertex.

// Verify expected layout: Point4(4*4) + TexCoord(2*4) + Vector3(3*4) + Color(4) = 40 bytes for float.
static_assert(sizeof(VertexF) == sizeof(float) * 4 + sizeof(float) * 2 + sizeof(float) * 3 + sizeof(Color),
              "VertexF has unexpected padding");

/// Linearly interpolates between two vertices.
/// @param[in] v0 The first vertex.
/// @param[in] v1 The second vertex.
/// @param[in] t The interpolation factor (0.0 to 1.0).
/// @return The interpolated vertex.
template <typename T>
constexpr Vertex<T> lerp(const Vertex<T>& v0, const Vertex<T>& v1, const T t) noexcept
{
  return Vertex<T>{math::lerp(v0.point, v1.point, t), lerp(v0.tex_coord, v1.tex_coord, t),
                   math::lerp(v0.normal, v1.normal, t), lerp(v0.color, v1.color, static_cast<float>(t))};
}

/// Computes the signed distance from a vertex to a plane.
/// @param[in] vertex The vertex to compute the distance from.
/// @param[in] plane The plane to compute the distance to.
/// @return The signed distance from the vertex to the plane.
template <typename T>
constexpr T signed_distance(const Vertex<T>& vertex, const math::Plane3<T>& plane) noexcept
{
  return math::signed_distance(vertex.point.xyz(), plane);
}

} // namespace rtw::sw_renderer
