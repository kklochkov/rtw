#pragma once

#include "fixed_point/math.h"
#include "sw_renderer/operations.h"
#include "sw_renderer/vertex.h"

#include <algorithm>
#include <type_traits>

namespace rtw::sw_renderer
{

namespace details
{

template <typename RasteriseCallbackT>
constexpr bool IS_PIXEL_RASTERISE_CALLBACK_V = std::is_invocable_r_v<void, RasteriseCallbackT, const math::Point2I&>;

template <typename T, typename RasteriseCallbackT>
constexpr bool IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const Vertex4<T>&, const Vertex4<T>&, const Vertex4<T>&,
                          const math::Point2I&, const Barycentric3<T>&>;

template <typename T, typename RasteriseCallbackT>
constexpr bool IS_TRIANGLE_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const Vertex4<T>&, const Vertex4<T>&, const Vertex4<T>&,
                          const math::Point2I&>;

} // namespace details

/// Draw a line using the DDA (Digital Differential Analyzer) algorithm.
/// See https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm) for more details.
/// @tparam T The type of the coordinates.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const math::Point2I&).
/// @param[in] p0 The first point of the line.
/// @param[in] p1 The second point of the line.
/// @param[in] rasterise The callback function.
/// @{
template <typename T, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_PIXEL_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void draw_line_dda(const math::Point2I& p0, const math::Point2I& p1, RasteriseCallbackT rasterise)
{
  math::Vector2<T> delta = (p1 - p0).cast<T>();
  const auto steps = static_cast<std::int32_t>(std::max(std::abs(delta.x()), std::abs(delta.y())));

  assert(steps > 0);

  delta /= static_cast<T>(steps);

  auto p = p0.cast<T>();
  for (std::int32_t i = 0U; i <= steps; ++i)
  {
    rasterise(p.template cast<std::int32_t>());
    p += delta;
  }
}

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_PIXEL_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void draw_line_dda(const math::Point2I& p0, const math::Point2I& p1, RasteriseCallbackT rasterise)
{
  draw_line_dda<float>(p0, p1, rasterise);
}
/// @}

/// Draw a line using the Bresenham's algorithm.
/// See https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm for more details.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const math::Point2I&).
/// @param[in] p0 The first point of the line.
/// @param[in] p1 The second point of the line.
/// @param[in] rasterise The callback function.
template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_PIXEL_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void draw_line_bresenham(const math::Point2I& p0, const math::Point2I& p1, RasteriseCallbackT rasterise)
{
  const std::int32_t dx = std::abs(p1.x() - p0.x());
  const std::int32_t dy = std::abs(p1.y() - p0.y());

  const std::int32_t sx = (p0.x() < p1.x()) ? 1 : -1;
  const std::int32_t sy = (p0.y() < p1.y()) ? 1 : -1;

  std::int32_t err = dx - dy;

  math::Point2I p{p0.x(), p0.y()};

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

namespace details
{
template <typename T>
constexpr T get_resolution() noexcept
{
  if constexpr (rtw::fixed_point::IS_FIXED_POINT_V<T>)
  {
    return T{T::RESOLUTION};
  }
  else
  {
    return T{0.001F};
  }
}
} // namespace details

/// Rasterise a triangle by visiting pixels in a bounding box using top-left fill convention.
/// The vertex order is counter-clockwise.
/// The algorithm is based on the Juan Pineda's paper "A Parallel Algorithm for Polygon Rasterization".
/// The test if a pixel is inside the triangle is done by using barycentric coordinates.
/// Calls the callback function for each pixel inside the triangle.
/// @tparam T The type of the coordinates.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const Vertex4F&, const Vertex4F&, const Vertex4F&, const math::Point2I&, const Barycentric3F&).
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @param[in] rasterise_inside The callback function which is called for each pixel inside the triangle.
/// @{
template <typename T, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V<T, RasteriseCallbackT>>>
void fill_triangle_bbox(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, RasteriseCallbackT rasterise)
{
  using rtw::fixed_point::math::ceil;
  using rtw::fixed_point::math::floor;
  using std::ceil;
  using std::floor;

  const auto min_x = static_cast<std::int32_t>(floor(std::min({v0.point.x(), v1.point.x(), v2.point.x()})));
  const auto min_y = static_cast<std::int32_t>(floor(std::min({v0.point.y(), v1.point.y(), v2.point.y()})));
  const auto max_x = static_cast<std::int32_t>(ceil(std::max({v0.point.x(), v1.point.x(), v2.point.x()})));
  const auto max_y = static_cast<std::int32_t>(ceil(std::max({v0.point.y(), v1.point.y(), v2.point.y()})));

  const auto va = v0.point.xy();
  const auto vb = v1.point.xy();
  const auto vc = v2.point.xy();
  auto edge_a = vc - vb; // a
  auto edge_b = va - vc; // b
  auto edge_c = vb - va; // c
  const auto area = math::cross(edge_a, edge_b);

  // Calculate the initial barycentric coordinates of the top-left corner of the bounding box with subpixel precision.
  const math::Point2<T> p0{static_cast<T>(min_x) + T{0.5F}, static_cast<T>(min_y) + T{0.5F}};
  auto w0_init = math::cross(edge_a, p0 - vc);
  auto w1_init = math::cross(edge_b, p0 - va);
  auto w2_init = math::cross(edge_c, p0 - vb);

  // Apply top-left fill convention.
  constexpr T RESOLUTION = details::get_resolution<T>();
  w0_init += is_top_left(edge_a) ? T{0.0F} : -RESOLUTION;
  w1_init += is_top_left(edge_b) ? T{0.0F} : -RESOLUTION;
  w2_init += is_top_left(edge_c) ? T{0.0F} : -RESOLUTION;

  // Normalize the barycentric coordinates to avoid division in the inner loop.
  w0_init /= area;
  w1_init /= area;
  w2_init /= area;

  // Scale the edge vectors to avoid division in the inner loop.
  edge_a /= area;
  edge_b /= area;
  edge_c /= area;

  for (std::int32_t y = min_y; y <= max_y; ++y)
  {
    auto w0 = w0_init;
    auto w1 = w1_init;
    auto w2 = w2_init;

    for (std::int32_t x = min_x; x <= max_x; ++x)
    {
      if ((w0 >= 0) && (w1 >= 0) && (w2 >= 0)) // NOLINT(hicpp-signed-bitwise)
      {
        const math::Point2I p{x, y};
        const Barycentric3<T> b{w0, w1, w2};
        rasterise(v0, v1, v2, p, b);
      }

      w0 -= edge_a.y();
      w1 -= edge_b.y();
      w2 -= edge_c.y();
    }

    w0_init += edge_a.x();
    w1_init += edge_b.x();
    w2_init += edge_c.x();
  }
}

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V<float, RasteriseCallbackT>>>
void fill_triangle_bbox(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, RasteriseCallbackT rasterise)
{
  fill_triangle_bbox<float>(v0, v1, v2, rasterise);
}
/// @}

namespace details
{
/// Visit pixels in a triangle.
/// @tparam T The type of the coordinates.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const Vertex4F&, const Vertex4F&, const Vertex4F&, const math::Point2I&).
/// The vertex order is counter-clockwise. The vectrices are sorted by y-coordinate.
/// @param[in] v0_raster The first vertex of the triangle.
/// @param[in] v1_raster The second vertex of the triangle.
/// @param[in] inv_slope0 The inverse slope of the first edge of the triangle.
/// @param[in] inv_slope1 The inverse slope of the second edge of the triangle.
/// @param[in] rasterise The callback function.
/// @param[in] v0 The first vertex with additional data.
/// @param[in] v1 The second vertex with additional data.
/// @param[in] v2 The third vertex with additional data.
template <typename T, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_TRIANGLE_RASTERISE_CALLBACK_V<T, RasteriseCallbackT>>>
void visit_pixels(const math::Point2I& v0_raster, const math::Point2I& v1_raster, const T inv_slope0,
                  const T inv_slope1, RasteriseCallbackT rasterise, const Vertex4<T>& v0, const Vertex4<T>& v1,
                  const Vertex4<T>& v2)
{
  for (std::int32_t y = v0_raster.y(); y <= v1_raster.y(); ++y)
  {
    auto x_start =
        static_cast<std::int32_t>(inv_slope0 * static_cast<T>(y - v0_raster.y()) + static_cast<T>(v0_raster.x()));
    auto x_end =
        static_cast<std::int32_t>(inv_slope1 * static_cast<T>(y - v1_raster.y()) + static_cast<T>(v1_raster.x()));

    if (x_start > x_end)
    {
      std::swap(x_start, x_end);
    }

    for (std::int32_t x = x_start; x <= x_end; ++x)
    {
      rasterise(v0, v1, v2, math::Point2I{x, y});
    }
  }
}
} // namespace details

/// Fill a triangle using the scanline algorithm.
/// @tparam T The type of the coordinates.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const Vertex4F&, const Vertex4F&, const Vertex4F&, const math::Point2I&).
/// The vertex order is counter-clockwise. It is assumed that the vectrices are sorted by
/// y-coordinate. The algorithm works in the following way:
/// 1. Sort vertices by y-coordinate.
/// 2. Calculate edges.
/// 3. Check if the triangle is degenerate.
/// 4. Calculate the inverse slope of the edges.
/// 5. Check if the triangle is flat-bottom and fill it.
/// 6. Check if the triangle is flat-top and fill it.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @param[in] rasterise The callback function.
/// @{
template <typename T, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_TRIANGLE_RASTERISE_CALLBACK_V<T, RasteriseCallbackT>>>
void fill_triangle_scanline(Vertex4<T> v0, Vertex4<T> v1, Vertex4<T> v2, RasteriseCallbackT rasterise)
{
  // Sort vertices by y-coordinate.
  if (v0.point.y() > v1.point.y())
  {
    std::swap(v0, v1);
  }
  if (v0.point.y() > v2.point.y())
  {
    std::swap(v0, v2);
  }
  if (v1.point.y() > v2.point.y())
  {
    std::swap(v1, v2);
  }

  const auto v0_raster = v0.point.xy().template cast<std::int32_t>();
  const auto v1_raster = v1.point.xy().template cast<std::int32_t>();
  const auto v2_raster = v2.point.xy().template cast<std::int32_t>();

  // Calculate edges.
  const auto v0v1 = v1_raster - v0_raster;
  const auto v0v2 = v2_raster - v0_raster;
  const auto v1v2 = v2_raster - v1_raster;

  // Check if the triangle is degenerate.
  if (v0v2.y() == 0)
  {
    return;
  }

  const auto v0v2_inv_slope = static_cast<T>(v0v2.x()) / static_cast<T>(v0v2.y());
  if (v0v1.y() != 0) // Check if the triangle is flat-bottom and fill it.
  {
    const auto v0v1_inv_slope = static_cast<T>(v0v1.x()) / static_cast<T>(v0v1.y());
    details::visit_pixels(v0_raster, v1_raster, v0v2_inv_slope, v0v1_inv_slope, rasterise, v0, v1, v2);
  }

  if (v1v2.y() != 0) // Check if the triangle is flat-top and fill it.
  {
    const auto v1v2_inv_slope = static_cast<T>(v1v2.x()) / static_cast<T>(v1v2.y());
    details::visit_pixels(v1_raster, v2_raster, v1v2_inv_slope, v0v2_inv_slope, rasterise, v0, v1, v2);
  }
}

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_TRIANGLE_RASTERISE_CALLBACK_V<float, RasteriseCallbackT>>>
void fill_triangle_scanline(Vertex4F v0, Vertex4F v1, Vertex4F v2, RasteriseCallbackT rasterise)
{
  fill_triangle_scanline<float>(v0, v1, v2, rasterise);
}
/// @}

} // namespace rtw::sw_renderer
