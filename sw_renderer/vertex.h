#pragma once

#include "math/point.h"

#include "sw_renderer/color.h"
#include "sw_renderer/tex_coord.h"

namespace rtw::sw_renderer
{

template <typename T>
struct Vertex
{
  explicit Vertex(const math::Point4<T>& point = {}, const TexCoord2<T>& tex_coord = {},
                  const math::Vector3<T>& normal = {}, const Color& color = {})
      : point{point}, tex_coord{tex_coord}, normal{normal}, color{color}
  {
  }

  math::Point4<T> point;
  TexCoord2<T> tex_coord;
  math::Vector3<T> normal;
  Color color;
};

template <typename T>
using Vertex4 = Vertex<T>;
using Vertex4F = Vertex4<float>;
using Vertex4D = Vertex4<double>;

} // namespace rtw::sw_renderer
