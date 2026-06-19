#pragma once

#include "sw_renderer/clipping.h"
#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/types.h"

#include "math/interpolation.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include <array>
#include <cstddef>

namespace rtw::sw_renderer
{

template <typename T>
struct ClipVertex
{
  math::Vector4<T> position;
  RegisterFile<T, MAX_VARYING_COUNT> varyings;
  T point_size{T{1}};
};

namespace clip_planes
{
template <typename T>
constexpr math::Vector4<T> left() noexcept
{
  return {T{1}, T{0}, T{0}, T{1}};
}
template <typename T>
constexpr math::Vector4<T> right() noexcept
{
  return {T{-1}, T{0}, T{0}, T{1}};
}
template <typename T>
constexpr math::Vector4<T> bottom() noexcept
{
  return {T{0}, T{1}, T{0}, T{1}};
}
template <typename T>
constexpr math::Vector4<T> top() noexcept
{
  return {T{0}, T{-1}, T{0}, T{1}};
}
template <typename T>
constexpr math::Vector4<T> near() noexcept
{
  return {T{0}, T{0}, T{1}, T{1}};
}
template <typename T>
constexpr math::Vector4<T> far() noexcept
{
  return {T{0}, T{0}, T{-1}, T{1}};
}
} // namespace clip_planes

template <typename T>
constexpr double_precision signed_distance(const ClipVertex<T>& vertex, const math::Vector4<T>& plane) noexcept
{
  return math::dot(plane.template cast<double_precision>(), vertex.position.template cast<double_precision>());
}

template <typename T>
constexpr ClipVertex<T> lerp(const ClipVertex<T>& v0, const ClipVertex<T>& v1, const T t) noexcept
{
  return ClipVertex<T>{math::lerp(v0.position, v1.position, t), lerp(v0.varyings, v1.varyings, t),
                       math::lerp(v0.point_size, v1.point_size, t)};
}

template <typename T, std::size_t CAPACITY = 9U>
constexpr math::ConvexPolygon<T, ClipVertex, CAPACITY> clip(const ClipVertex<T>& v0, const ClipVertex<T>& v1,
                                                            const ClipVertex<T>& v2) noexcept
{
  const std::array clip_plane_list{clip_planes::left<T>(), clip_planes::right<T>(), clip_planes::bottom<T>(),
                                   clip_planes::top<T>(),  clip_planes::near<T>(),  clip_planes::far<T>()};
  return clip(v0, v1, v2, stl::make_span(clip_plane_list));
}

} // namespace rtw::sw_renderer
