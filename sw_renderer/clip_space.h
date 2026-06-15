#pragma once

#include "sw_renderer/clipping.h"
#include "sw_renderer/register_file.h"
#include "sw_renderer/types.h"

#include "math/interpolation.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include <array>
#include <cstddef>

namespace rtw::sw_renderer
{

/// A vertex in homogeneous clip space: clip-space position plus the full varying register file.
/// Single-type-parameter so it slots into `math::ConvexPolygon<T, ClipVertex, CAPACITY>`.
/// @tparam T The scalar component type.
template <typename T>
struct ClipVertex
{
  math::Vector4<T> position;                   ///< The clip-space position (gl_Position).
  RegisterFile<T, MAX_VARYING_COUNT> varyings; ///< The varyings, interpolated during clipping.
};

/// The six homogeneous clip-space half-spaces (OpenGL clip volume, NDC depth in [-1, 1]) as plane
/// coefficients `(a, b, c, d)`: a position is inside when `a*x + b*y + c*z + d*w >= 0`.
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

/// Signed distance from a clip-space vertex to a homogeneous clip plane; non-negative means inside.
///
/// @param[in] vertex The clip-space vertex to measure.
/// @param[in] plane The clip plane coefficients `(a, b, c, d)`.
/// @return The signed distance to the plane.
template <typename T>
constexpr double_precision signed_distance(const ClipVertex<T>& vertex, const math::Vector4<T>& plane) noexcept
{
  return math::dot(plane.template cast<double_precision>(), vertex.position.template cast<double_precision>());
}

/// Linearly interpolates between two clip-space vertices (position and varyings).
/// Customization point for `clip_against_plane`.
/// @param[in] v0 The first vertex.
/// @param[in] v1 The second vertex.
/// @param[in] t The interpolation factor (0.0 to 1.0).
/// @return The interpolated clip-space vertex.
template <typename T>
constexpr ClipVertex<T> lerp(const ClipVertex<T>& v0, const ClipVertex<T>& v1, const T t) noexcept
{
  return ClipVertex<T>{math::lerp(v0.position, v1.position, t), lerp(v0.varyings, v1.varyings, t)};
}

/// Clips a triangle in homogeneous clip space against the six clip planes using Sutherland-Hodgman.
///
/// @tparam CAPACITY The maximum number of vertices the polygon can hold.
/// Default is 9, which is the maximum number of vertices a triangle can have after clipping against all 6 clip
/// planes (3 original + 6 new = 9).
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @return The clipped polygon.
template <typename T, std::size_t CAPACITY = 9U>
constexpr math::ConvexPolygon<T, ClipVertex, CAPACITY> clip(const ClipVertex<T>& v0, const ClipVertex<T>& v1,
                                                            const ClipVertex<T>& v2) noexcept
{
  const std::array clip_planes{clip_planes::left<T>(), clip_planes::right<T>(), clip_planes::bottom<T>(),
                               clip_planes::top<T>(),  clip_planes::near<T>(),  clip_planes::far<T>()};
  return clip(v0, v1, v2, stl::make_span(clip_planes));
}

} // namespace rtw::sw_renderer
