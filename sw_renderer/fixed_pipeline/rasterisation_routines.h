#pragma once

#include "sw_renderer/raster_common.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex.h"

#include "math/point.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include <algorithm>
#include <type_traits>

namespace rtw::sw_renderer
{

namespace details
{

template <typename RasteriseCallbackT>
constexpr inline bool IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const VertexF&, const VertexF&, const VertexF&, const Point2I&,
                          const BarycentricF&>;

template <typename RasteriseCallbackT>
constexpr inline bool IS_TRIANGLE_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const VertexF&, const VertexF&, const VertexF&, const Point2I&>;

} // namespace details

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_BARYCENTRIC_TRIANGLE_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, RasteriseCallbackT rasterise)
{
  // Bounding-box rasterization via incremental edge functions in the style of
  // Juan Pineda's "A Parallel Algorithm for Polygon Rasterization".
  using multiprecision::math::ceil;
  using multiprecision::math::floor;
  using std::ceil;
  using std::floor;

  const auto min_x = static_cast<std::int32_t>(floor(std::min({v0.point.x(), v1.point.x(), v2.point.x()})));
  const auto min_y = static_cast<std::int32_t>(floor(std::min({v0.point.y(), v1.point.y(), v2.point.y()})));
  const auto max_x = static_cast<std::int32_t>(ceil(std::max({v0.point.x(), v1.point.x(), v2.point.x()})));
  const auto max_y = static_cast<std::int32_t>(ceil(std::max({v0.point.y(), v1.point.y(), v2.point.y()})));

  const auto va = v0.point.xy().cast<double_precision>();
  const auto vb = v1.point.xy().cast<double_precision>();
  const auto vc = v2.point.xy().cast<double_precision>();
  auto edge_a = vc - vb;
  auto edge_b = va - vc;
  auto edge_c = vb - va;
  const auto area = math::cross(edge_a, edge_b);

  const Point2D p0{static_cast<double_precision>(min_x) + double_precision{0.5},
                   static_cast<double_precision>(min_y) + double_precision{0.5}};
  auto w0_init = math::cross(edge_a, p0 - vc);
  auto w1_init = math::cross(edge_b, p0 - va);
  auto w2_init = math::cross(edge_c, p0 - vb);

  w0_init += fill_bias(area, edge_a);
  w1_init += fill_bias(area, edge_b);
  w2_init += fill_bias(area, edge_c);

  w0_init /= area;
  w1_init /= area;
  w2_init /= area;

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
        const BarycentricF b{static_cast<single_precision>(w0), static_cast<single_precision>(w1),
                             static_cast<single_precision>(w2)};
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

template <typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_TRIANGLE_RASTERISE_CALLBACK_V<RasteriseCallbackT>>>
void fill_triangle_scanline(VertexF v0, VertexF v1, VertexF v2, RasteriseCallbackT rasterise)
{
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

  const auto v0v1 = v1_raster - v0_raster;
  const auto v0v2 = v2_raster - v0_raster;
  const auto v1v2 = v2_raster - v1_raster;

  if (v0v2.y() == 0)
  {
    return;
  }

  const auto v0v2_inv_slope = static_cast<single_precision>(v0v2.x()) / static_cast<single_precision>(v0v2.y());
  if (v0v1.y() != 0)
  {
    const auto v0v1_inv_slope = static_cast<single_precision>(v0v1.x()) / static_cast<single_precision>(v0v1.y());
    details::visit_pixels(v0_raster, v1_raster, v0v2_inv_slope, v0v1_inv_slope, rasterise, v0, v1, v2);
  }

  if (v1v2.y() != 0)
  {
    const auto v1v2_inv_slope = static_cast<single_precision>(v1v2.x()) / static_cast<single_precision>(v1v2.y());
    details::visit_pixels(v1_raster, v2_raster, v1v2_inv_slope, v0v2_inv_slope, rasterise, v0, v1, v2);
  }
}

} // namespace rtw::sw_renderer
