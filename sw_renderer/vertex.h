#pragma once

#include "sw_renderer/color.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/tex_coord.h"

namespace rtw::sw_renderer
{

template <typename T>
struct Vertex
{
  explicit Vertex(const math::Point4<T>& point = {}, const TexCoord<T>& tex_coord = {},
                  const math::Vector3<T>& normal = {}, const Color& color = {})
      : point{point}, tex_coord{tex_coord}, normal{normal}, color{color}
  {
  }

  math::Point4<T> point;
  TexCoord<T> tex_coord;
  math::Vector3<T> normal;
  Color color;
};

using VertexF = Vertex<single_precision>;
using VertexD = Vertex<double_precision>;

} // namespace rtw::sw_renderer
