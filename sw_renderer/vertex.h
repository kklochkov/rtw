#pragma once

#include "math/point.h"

#include "sw_renderer/color.h"
#include "sw_renderer/tex_coord.h"

namespace rtw::sw_renderer
{

template <typename T>
struct Vertex
{
  math::Point4<T> point{};
  TexCoord2<T> tex_coord{};
  math::Vector3<T> normal{};
  Color color{};
};

template <typename T>
using Vertex4 = Vertex<T>;
using Vertex4f = Vertex4<float>;
using Vertex4d = Vertex4<double>;

} // namespace rtw::sw_renderer
