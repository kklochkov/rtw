#pragma once

#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/raster_common.h"
#include "sw_renderer/types.h"

#include "math/bounding_box.h"
#include "math/point.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace rtw::sw_renderer
{

namespace details
{

template <std::uint16_t N, typename RasteriseCallbackT>
constexpr inline bool IS_VARYING_RASTERISE_CALLBACK_V =
    std::is_invocable_r_v<void, RasteriseCallbackT, const Point2I&, const RegisterFile<single_precision, N>&,
                          single_precision, single_precision>;

} // namespace details

template <std::uint16_t N, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_VARYING_RASTERISE_CALLBACK_V<N, RasteriseCallbackT>>>
constexpr void fill_triangle_bbox(const Vector4F& p0, const Vector4F& p1, const Vector4F& p2,
                                  const RegisterFile<single_precision, N>& varyings0,
                                  const RegisterFile<single_precision, N>& varyings1,
                                  const RegisterFile<single_precision, N>& varyings2, const math::BoundingBoxI& bounds,
                                  RasteriseCallbackT rasterise)
{
  // Bounding-box rasterization via incremental edge functions in the style of
  // Juan Pineda's "A Parallel Algorithm for Polygon Rasterization".
  using multiprecision::math::ceil;
  using multiprecision::math::floor;
  using std::ceil;
  using std::floor;

  const auto min_x = std::max(static_cast<std::int32_t>(floor(std::min({p0.x(), p1.x(), p2.x()}))), bounds.min_x);
  const auto min_y = std::max(static_cast<std::int32_t>(floor(std::min({p0.y(), p1.y(), p2.y()}))), bounds.min_y);
  const auto max_x = std::min(static_cast<std::int32_t>(ceil(std::max({p0.x(), p1.x(), p2.x()}))), bounds.max_x);
  const auto max_y = std::min(static_cast<std::int32_t>(ceil(std::max({p0.y(), p1.y(), p2.y()}))), bounds.max_y);

  const auto va = p0.xy().cast<double_precision>();
  const auto vb = p1.xy().cast<double_precision>();
  const auto vc = p2.xy().cast<double_precision>();
  auto edge_a = vc - vb;
  auto edge_b = va - vc;
  auto edge_c = vb - va;
  const auto area = math::cross(edge_a, edge_b);

  const Vector2D corner{static_cast<double_precision>(min_x) + double_precision{0.5},
                        static_cast<double_precision>(min_y) + double_precision{0.5}};
  auto w0_init = math::cross(edge_a, corner - vc);
  auto w1_init = math::cross(edge_b, corner - va);
  auto w2_init = math::cross(edge_c, corner - vb);

  w0_init += fill_bias(area, edge_a);
  w1_init += fill_bias(area, edge_b);
  w2_init += fill_bias(area, edge_c);

  w0_init /= area;
  w1_init /= area;
  w2_init /= area;
  edge_a /= area;
  edge_b /= area;
  edge_c /= area;

  const auto inv_w0 = p0.w();
  const auto inv_w1 = p1.w();
  const auto inv_w2 = p2.w();
  const auto z0 = static_cast<double_precision>(p0.z());
  const auto z1 = static_cast<double_precision>(p1.z());
  const auto z2 = static_cast<double_precision>(p2.z());

  // Screen-space depth is affine in (x, y), so accumulate window_z incrementally instead of recomputing the
  // barycentric weighting at every fragment. The deltas mirror the edge-function walk below: stepping +1 pixel in x
  // subtracts edge_*.y() from each weight and starting a new row adds edge_*.x(), so applying the same deltas to
  // window_z replaces three multiplies per fragment with a single add. The running sum stays in double_precision (as
  // the per-pixel recompute did) and is narrowed only at use.
  const auto dz_dx = -((edge_a.y() * z0) + (edge_b.y() * z1) + (edge_c.y() * z2));
  const auto dz_dy = (edge_a.x() * z0) + (edge_b.x() * z1) + (edge_c.x() * z2);

  // Determine the highest active varying slot once per triangle. A slot that is zero on all three vertices
  // interpolates to exactly zero, which the zero-initialised output already holds, so it can be skipped.
  std::uint16_t active_count = 0U;
  constexpr math::Vector4<single_precision> ZERO_VARYING{};
  for (std::uint16_t slot = 0U; slot < N; ++slot)
  {
    if (varyings0[slot] != ZERO_VARYING || varyings1[slot] != ZERO_VARYING || varyings2[slot] != ZERO_VARYING)
    {
      active_count = static_cast<std::uint16_t>(slot + 1U);
    }
  }

  // window_z_row holds the accumulated depth at the start of the current row; window_z_acc walks it across the row.
  // Both step in lockstep with the edge-function weights (window_z_acc += dz_dx per pixel, window_z_row += dz_dy per
  // row), so they stay aligned with w0/w1/w2 without the per-fragment multiply-add.
  auto window_z_row = (w0_init * z0) + (w1_init * z1) + (w2_init * z2);

  RegisterFile<single_precision, N> varyings;
  for (std::int32_t y = min_y; y <= max_y; ++y)
  {
    auto w0 = w0_init;
    auto w1 = w1_init;
    auto w2 = w2_init;
    auto window_z_acc = window_z_row;

    for (std::int32_t x = min_x; x <= max_x; ++x)
    {
      if ((w0 >= 0) && (w1 >= 0) && (w2 >= 0))
      {
        const auto window_z = static_cast<single_precision>(window_z_acc);

        const auto sw0 = static_cast<single_precision>(w0);
        const auto sw1 = static_cast<single_precision>(w1);
        const auto sw2 = static_cast<single_precision>(w2);
        const auto c0 = sw0 * inv_w0;
        const auto c1 = sw1 * inv_w1;
        const auto c2 = sw2 * inv_w2;
        const auto inv_w = c0 + c1 + c2;

        if (active_count > 0U)
        {
          const auto recip = single_precision{1} / inv_w;
          for (std::uint16_t slot = 0U; slot < active_count; ++slot)
          {
            const auto accumulated = (varyings0[slot] * c0) + (varyings1[slot] * c1) + (varyings2[slot] * c2);
            varyings[slot] = accumulated * recip;
          }
        }

        rasterise(Point2I{x, y}, varyings, window_z, inv_w);
      }

      w0 -= edge_a.y();
      w1 -= edge_b.y();
      w2 -= edge_c.y();
      window_z_acc += dz_dx;
    }

    w0_init += edge_a.x();
    w1_init += edge_b.x();
    w2_init += edge_c.x();
    window_z_row += dz_dy;
  }
}

/// Integer line-walk algorithm used by draw_line_varyings.
enum class LineRaster : std::uint8_t
{
  BRESENHAM = 0U, ///< Integer-exact walk; matches the fixed pipeline's draw_line.
  DDA,            ///< Floating-point walk; benchmarked against Bresenham, kept for the equivalence test.
};

namespace details
{

constexpr Point2I window_to_pixel(const Vector4F& window_position) noexcept
{
  return Point2I{static_cast<std::int32_t>(window_position.x()), static_cast<std::int32_t>(window_position.y())};
}

constexpr bool inside_pixel_bounds(const Point2I& p, const math::BoundingBoxI& bounds) noexcept
{
  return (p.x() >= bounds.min_x) && (p.x() <= bounds.max_x) && (p.y() >= bounds.min_y) && (p.y() <= bounds.max_y);
}

} // namespace details

/// Rasterises the edge p0->p1 with perspective-correct varying interpolation, mirroring fill_triangle_bbox for the
/// PolygonMode::LINE path. The two endpoints carry window-space x/y in xy, screen-space depth in z, and 1/w in w
/// (the layout produced by clip_to_window). Each covered pixel invokes `rasterise(pixel, varyings, window_z, inv_w)`
/// with the same signature the fill path uses, so callers share a single fragment-shading callback.
/// Pixels outside are skipped (the near-plane / out-of-bounds guard the fill path applies via its scan-box clamp).
/// The default Bresenham walk matches the fixed pipeline; DDA is selectable for benchmarking.
template <std::uint16_t N, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_VARYING_RASTERISE_CALLBACK_V<N, RasteriseCallbackT>>>
constexpr void draw_line_varyings(const Vector4F& p0, const Vector4F& p1,
                                  const RegisterFile<single_precision, N>& varyings0,
                                  const RegisterFile<single_precision, N>& varyings1, const math::BoundingBoxI& bounds,
                                  RasteriseCallbackT rasterise, const LineRaster algorithm = LineRaster::BRESENHAM)
{
  const auto ip0 = details::window_to_pixel(p0);
  const auto ip1 = details::window_to_pixel(p1);

  // Parameterise the line by its major (longer) axis. Both the DDA and Bresenham walkers advance the major axis by
  // exactly one pixel per step and visit every integer coordinate between the endpoints, so deriving t from the
  // major-axis position -- rather than from a step counter -- yields identical varyings for either walker.
  const std::int32_t dx = std::abs(ip1.x() - ip0.x());
  const std::int32_t dy = std::abs(ip1.y() - ip0.y());
  const bool x_major = dx >= dy;
  const auto major0 = static_cast<single_precision>(x_major ? ip0.x() : ip0.y());
  const auto major1 = static_cast<single_precision>(x_major ? ip1.x() : ip1.y());
  const auto span = major1 - major0;

  const auto z0 = p0.z();
  const auto z1 = p1.z();
  const auto inv_w0 = p0.w();
  const auto inv_w1 = p1.w();

  // Skip varying slots that are zero on both endpoints; they interpolate to exactly zero (see fill_triangle_bbox).
  std::uint16_t active_count = 0U;
  constexpr math::Vector4<single_precision> ZERO_VARYING{};
  for (std::uint16_t slot = 0U; slot < N; ++slot)
  {
    if ((varyings0[slot] != ZERO_VARYING) || (varyings1[slot] != ZERO_VARYING))
    {
      active_count = static_cast<std::uint16_t>(slot + 1U);
    }
  }

  RegisterFile<single_precision, N> varyings;
  const auto per_pixel = [&](const Point2I& p)
  {
    if (!details::inside_pixel_bounds(p, bounds))
    {
      return;
    }

    const auto major = static_cast<single_precision>(x_major ? p.x() : p.y());
    const auto t = (span != single_precision{0}) ? ((major - major0) / span) : single_precision{0};
    const auto window_z = z0 + ((z1 - z0) * t);

    const auto c0 = (single_precision{1} - t) * inv_w0;
    const auto c1 = t * inv_w1;
    const auto inv_w = c0 + c1;

    if (active_count > 0U)
    {
      const auto recip = single_precision{1} / inv_w;
      for (std::uint16_t slot = 0U; slot < active_count; ++slot)
      {
        const auto accumulated = (varyings0[slot] * c0) + (varyings1[slot] * c1);
        varyings[slot] = accumulated * recip;
      }
    }

    rasterise(p, varyings, window_z, inv_w);
  };

  if (algorithm == LineRaster::DDA)
  {
    draw_line_dda(ip0, ip1, per_pixel);
  }
  else
  {
    draw_line_bresenham(ip0, ip1, per_pixel);
  }
}

/// Rasterises a vertex as a square point sprite for the PolygonMode::POINT path. At a vertex the
/// perspective-correct interpolation of fill_triangle_bbox collapses to the raw vertex varyings, so each pixel of the
/// sprite shades with the same varyings, depth and inv_w as the centre. Even sizes bias top-left around the centre
/// pixel, matching the integer pixel-centre convention used elsewhere.
template <std::uint16_t N, typename RasteriseCallbackT,
          typename = std::enable_if_t<details::IS_VARYING_RASTERISE_CALLBACK_V<N, RasteriseCallbackT>>>
constexpr void draw_point_varyings(const Vector4F& p, const RegisterFile<single_precision, N>& varyings,
                                   const math::BoundingBoxI& bounds, RasteriseCallbackT rasterise,
                                   const single_precision point_size = single_precision{1})
{
  const auto centre = details::window_to_pixel(p);
  const auto size = std::max(std::int32_t{1}, static_cast<std::int32_t>(point_size));
  const auto half_low = (size - 1) / 2;
  const auto half_high = size / 2;

  const auto min_x = std::max(centre.x() - half_low, bounds.min_x);
  const auto min_y = std::max(centre.y() - half_low, bounds.min_y);
  const auto max_x = std::min(centre.x() + half_high, bounds.max_x);
  const auto max_y = std::min(centre.y() + half_high, bounds.max_y);

  for (std::int32_t y = min_y; y <= max_y; ++y)
  {
    for (std::int32_t x = min_x; x <= max_x; ++x)
    {
      rasterise(Point2I{x, y}, varyings, p.z(), p.w());
    }
  }
}

} // namespace rtw::sw_renderer
