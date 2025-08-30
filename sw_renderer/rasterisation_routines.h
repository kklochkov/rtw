#pragma once

#include "sw_renderer/types.h"
#include "sw_renderer/vertex.h"

#include <algorithm>
#include <type_traits>

namespace rtw::sw_renderer
{

namespace details
{

template <typename RasteriseCallbackT>
constexpr inline bool IS_PIXEL_RASTERISE_CALLBACK_V = std::is_invocable_r_v<void, RasteriseCallbackT, const Point2I&>;

template <typename RasteriseCallbackT>
constexpr inline bool IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const VertexF&, const VertexF&, const VertexF&, const Point2I&,
                          const BarycentricF&>;

template <typename RasteriseCallbackT>
constexpr inline bool IS_TRIANGLE_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const VertexF&, const VertexF&, const VertexF&, const Point2I&>;

} // namespace details

/// Draw a line using the DDA (Digital Differential Analyzer) algorithm.
/// See https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm) for more details.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const Point2I&).
/// @param[in] p0 The first point of the line.
/// @param[in] p1 The second point of the line.
/// @param[in] rasterise The callback function.
template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_PIXEL_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void draw_line_dda(const Point2I& p0, const Point2I& p1, RasteriseCallbackT rasterise)
{
  using multiprecision::math::abs;
  using std::abs;

  Vector2F delta = (p1 - p0).cast<single_precision>();
  const auto steps = static_cast<std::int32_t>(std::max(abs(delta.x()), abs(delta.y())));

  assert(steps > 0);

  delta /= static_cast<single_precision>(steps);

  auto p = p0.cast<single_precision>();
  for (std::int32_t i = 0U; i <= steps; ++i)
  {
    rasterise(p.cast<std::int32_t>());
    p += delta;
  }
}

/// Draw a line using the Bresenham's algorithm.
/// See https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm for more details.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const Point2I&).
/// @param[in] p0 The first point of the line.
/// @param[in] p1 The second point of the line.
/// @param[in] rasterise The callback function.
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

/// Check if the edge of a triangle is top-left.
/// It is used to apply the top-left fill convention.
/// @tparam T The type of the elements.
/// @param[in] edge The edge of the triangle.
/// @return True if the edge is top-left, false otherwise.
template <typename T>
constexpr bool is_top_left(const math::Vector2<T>& edge) noexcept
{
  const bool is_top = edge.y() == T{0} && edge.x() < T{0};
  const bool is_left = edge.y() > T{0};
  return is_top || is_left;
}

/// Rasterise a triangle by visiting pixels in a bounding box using top-left fill convention.
/// The vertex order is counter-clockwise.
/// The algorithm is based on the Juan Pineda's paper "A Parallel Algorithm for Polygon Rasterization".
/// The test if a pixel is inside the triangle is done by using barycentric coordinates.
/// Calls the callback function for each pixel inside the triangle.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const VertexF&, const VertexF&, const VertexF&, const Point2I&, const BarycentricF&).
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @param[in] rasterise_inside The callback function which is called for each pixel inside the triangle.
template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, RasteriseCallbackT rasterise)
{
  using multiprecision::math::ceil;
  using multiprecision::math::floor;
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
  const Point2F p0{static_cast<single_precision>(min_x) + 0.5F, static_cast<single_precision>(min_y) + 0.5F};
  auto w0_init = math::cross(edge_a, p0 - vc);
  auto w1_init = math::cross(edge_b, p0 - va);
  auto w2_init = math::cross(edge_c, p0 - vb);

  // Apply top-left fill convention.
  constexpr single_precision ZERO{0.0F};
  w0_init += is_top_left(edge_a) ? ZERO : -ULP;
  w1_init += is_top_left(edge_b) ? ZERO : -ULP;
  w2_init += is_top_left(edge_c) ? ZERO : -ULP;

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
      if ((w0 >= 0) && (w1 >= 0) && (w2 >= 0))
      {
        const Point2I p{x, y};
        const BarycentricF b{w0, w1, w2};
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

namespace details
{
/// Visit pixels in a triangle.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const VertexF&, const VertexF&, const VertexF&, const Point2I&).
/// The vertex order is counter-clockwise. The vectrices are sorted by y-coordinate.
/// @param[in] v0_raster The first vertex of the triangle.
/// @param[in] v1_raster The second vertex of the triangle.
/// @param[in] inv_slope0 The inverse slope of the first edge of the triangle.
/// @param[in] inv_slope1 The inverse slope of the second edge of the triangle.
/// @param[in] rasterise The callback function.
/// @param[in] v0 The first vertex with additional data.
/// @param[in] v1 The second vertex with additional data.
/// @param[in] v2 The third vertex with additional data.
template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_TRIANGLE_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void visit_pixels(const Point2I& v0_raster, const Point2I& v1_raster, const single_precision inv_slope0,
                  const single_precision inv_slope1, RasteriseCallbackT rasterise, const VertexF& v0, const VertexF& v1,
                  const VertexF& v2)
{
  for (auto y = v0_raster.y(); y <= v1_raster.y(); ++y)
  {
    auto x_start = static_cast<std::int32_t>((inv_slope0 * static_cast<single_precision>(y - v0_raster.y()))
                                             + static_cast<single_precision>(v0_raster.x()));
    auto x_end = static_cast<std::int32_t>((inv_slope1 * static_cast<single_precision>(y - v1_raster.y()))
                                           + static_cast<single_precision>(v1_raster.x()));

    if (x_start > x_end)
    {
      std::swap(x_start, x_end);
    }

    for (auto x = x_start; x <= x_end; ++x)
    {
      rasterise(v0, v1, v2, Point2I{x, y});
    }
  }
}
} // namespace details

/// Fill a triangle using the scanline algorithm.
/// @tparam RasteriseCallbackT The type of the callback function which is called for each pixel.
/// The function must have the following signature:
/// void(const VertexF&, const VertexF&, const VertexF&, const Point2I&).
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
template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_TRIANGLE_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void fill_triangle_scanline(VertexF v0, VertexF v1, VertexF v2, RasteriseCallbackT rasterise)
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

  const auto v0_raster = v0.point.xy().cast<std::int32_t>();
  const auto v1_raster = v1.point.xy().cast<std::int32_t>();
  const auto v2_raster = v2.point.xy().cast<std::int32_t>();

  // Calculate edges.
  const auto v0v1 = v1_raster - v0_raster;
  const auto v0v2 = v2_raster - v0_raster;
  const auto v1v2 = v2_raster - v1_raster;

  // Check if the triangle is degenerate.
  if (v0v2.y() == 0)
  {
    return;
  }

  const auto v0v2_inv_slope = static_cast<single_precision>(v0v2.x()) / static_cast<single_precision>(v0v2.y());
  if (v0v1.y() != 0) // Check if the triangle is flat-bottom and fill it.
  {
    const auto v0v1_inv_slope = static_cast<single_precision>(v0v1.x()) / static_cast<single_precision>(v0v1.y());
    details::visit_pixels(v0_raster, v1_raster, v0v2_inv_slope, v0v1_inv_slope, rasterise, v0, v1, v2);
  }

  if (v1v2.y() != 0) // Check if the triangle is flat-top and fill it.
  {
    const auto v1v2_inv_slope = static_cast<single_precision>(v1v2.x()) / static_cast<single_precision>(v1v2.y());
    details::visit_pixels(v1_raster, v2_raster, v1v2_inv_slope, v0v2_inv_slope, rasterise, v0, v1, v2);
  }
}

} // namespace rtw::sw_renderer
