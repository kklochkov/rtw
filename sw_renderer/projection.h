#pragma once

#include "math/matrix.h"
#include "math/point.h"

namespace rtw::sw_renderer
{

/// Creates a matrix that transforms from world space to view space.
/// @tparam T The type of the matrix elements.
/// @param eye The position of the camera.
/// @param target The position the camera is looking at.
/// @param up The up vector of the camera.
/// @return The view matrix.
template <typename T>
constexpr math::Matrix4x4<T> make_screen_space_matrix(const std::size_t width, const std::size_t height) noexcept
{
  const auto tx = (width - 1) / T{2};
  const auto ty = (height - 1) / T{2};
  const auto sx = tx;
  const auto sy = -ty; // y axis is flipped in screen space
  // clang-format off
  return math::Matrix4x4<T>{
        sx, T{0}, T{0},   tx,
      T{0},   sy, T{0},   ty,
      T{0}, T{0}, T{1}, T{0},
      T{0}, T{0}, T{0}, T{1},
  };
  // clang-format on
}

template <typename T>
constexpr math::Point4<T> ndc_to_screen_space(const math::Point4<T>& point,
                                              const math::Matrix4x4<T>& screen_space_matrix) noexcept
{
  const auto w = point.w(); // original w is needed for depth buffer and perspective correct interpolation
  const auto result = screen_space_matrix * point;
  return math::Point4<T>{result.x(), result.y(), result.z(), w};
}

} // namespace rtw::sw_renderer
