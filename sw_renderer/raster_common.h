#pragma once

#include "sw_renderer/precision.h"
#include "sw_renderer/types.h"

#include "math/point.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include <algorithm>
#include <type_traits>

namespace rtw::sw_renderer
{

// Shared raster helpers for the bounding-box triangle walk used by both pipelines.
// The fill rules and incremental edge-function evaluation follow Juan Pineda's
// "A Parallel Algorithm for Polygon Rasterization".

namespace details
{

template <typename RasteriseCallbackT>
constexpr inline bool IS_PIXEL_RASTERISE_CALLBACK_V = std::is_invocable_r_v<void, RasteriseCallbackT, const Point2I&>;

} // namespace details

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_PIXEL_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void draw_line_dda(const Point2I& p0, const Point2I& p1, RasteriseCallbackT rasterise)
{
  using multiprecision::math::abs;
  using std::abs;

  Vector2F delta = (p1 - p0).cast<single_precision>();
  const auto steps = static_cast<std::int32_t>(std::max(abs(delta.x()), abs(delta.y())));

  assert(steps > 0);
  if (steps == 0)
  {
    rasterise(p0);
    return;
  }

  delta /= static_cast<single_precision>(steps);

  auto p = p0.cast<single_precision>();
  for (std::int32_t i = 0U; i <= steps; ++i)
  {
    rasterise(p.cast<std::int32_t>());
    p += delta;
  }
}

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_PIXEL_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void draw_line_bresenham(const Point2I& p0, const Point2I& p1, RasteriseCallbackT rasterise)
{
  const std::int32_t dx = std::abs(p1.x() - p0.x());
  const std::int32_t dy = std::abs(p1.y() - p0.y());

  const std::int32_t sx = (p0.x() < p1.x()) ? 1 : -1;
  const std::int32_t sy = (p0.y() < p1.y()) ? 1 : -1;

  std::int32_t err = dx - dy;

  Point2I p{p0.x(), p0.y()};

  while (true)
  {
    rasterise(p);

    if (p == p1)
    {
      break;
    }

    const std::int32_t e2 = 2 * err;

    if (e2 > -dy)
    {
      err -= dy;
      p.x() += sx;
    }

    if (e2 < dx)
    {
      err += dx;
      p.y() += sy;
    }
  }
}

template <typename T>
constexpr bool is_top_left(const math::Vector2<T>& edge) noexcept
{
  const bool is_top = (edge.y() == T{0}) && (edge.x() < T{0});
  const bool is_left = edge.y() > T{0};
  return is_top || is_left;
}

template <typename T>
constexpr T fill_bias(const T area, const math::Vector2<T>& edge) noexcept
{
  constexpr double_precision ZERO{0.0};
  const auto magnitude = static_cast<double_precision>(ULP);
  const auto bias = (area > ZERO) ? magnitude : -magnitude;
  return is_top_left(edge) ? ZERO : -bias;
}

} // namespace rtw::sw_renderer
