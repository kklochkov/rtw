#pragma once

#include "math/matrix.h"
#include "math/point.h"

namespace rtw::sw_renderer
{

/// Creates a matrix that transforms from normalized device coordinates (NDC) to screen space.
///
/// Maps x from [-1, 1] to [0, width-1] and y from [-1, 1] to [height-1, 0] (y-axis flipped).
/// @tparam T The scalar type of the matrix elements.
/// @tparam MEMORY_ORDER The memory layout order (row-major or column-major).
/// @param[in] width The screen width in pixels.
/// @param[in] height The screen height in pixels.
/// @return The screen-space transformation matrix.
template <typename T, math::MemoryOrder MEMORY_ORDER = math::DEFAULT_MEMORY_ORDER>
constexpr math::Matrix4x4<T, MEMORY_ORDER> make_screen_space_matrix(const std::size_t width,
                                                                    const std::size_t height) noexcept
{
  const auto tx = (width - 1) / T{2};
  const auto ty = (height - 1) / T{2};
  const auto sx = tx;
  const auto sy = -ty; // y axis is flipped in screen space
  // clang-format off
  return math::Matrix4x4<T, MEMORY_ORDER>{math::FROM_ROW_MAJOR,
        sx, T{0}, T{0},   tx,
      T{0},   sy, T{0},   ty,
      T{0}, T{0}, T{1}, T{0},
      T{0}, T{0}, T{0}, T{1},
  };
  // clang-format on
}

/// Transforms a point from normalized device coordinates to screen space.
/// Preserves the original w component for depth buffer and perspective-correct interpolation.
/// @tparam T The scalar type.
/// @tparam MEMORY_ORDER The memory layout order.
/// @param[in] point The point in NDC (after perspective division).
/// @param[in] screen_space_matrix The screen-space matrix from make_screen_space_matrix().
/// @return The point in screen pixel coordinates with original w preserved.
template <typename T, math::MemoryOrder MEMORY_ORDER>
constexpr math::Point4<T> ndc_to_screen_space(const math::Point4<T>& point,
                                              const math::Matrix4x4<T, MEMORY_ORDER>& screen_space_matrix) noexcept
{
  const auto w = point.w(); // original w is needed for depth buffer and perspective correct interpolation
  const auto result = screen_space_matrix * point;
  return math::Point4<T>{result.x(), result.y(), result.z(), w};
}

} // namespace rtw::sw_renderer
