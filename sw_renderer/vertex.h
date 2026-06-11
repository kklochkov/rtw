#pragma once

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

} // namespace rtw::sw_renderer
