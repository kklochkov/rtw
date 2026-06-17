#pragma once

#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/raster_common.h"
#include "sw_renderer/types.h"

#include "math/point.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include <algorithm>
#include <cstdint>
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
void fill_triangle_bbox(const Vector4F& p0, const Vector4F& p1, const Vector4F& p2,
                        const RegisterFile<single_precision, N>& varyings0,
                        const RegisterFile<single_precision, N>& varyings1,
                        const RegisterFile<single_precision, N>& varyings2, RasteriseCallbackT rasterise)
{
  // Bounding-box rasterization via incremental edge functions in the style of
  // Juan Pineda's "A Parallel Algorithm for Polygon Rasterization".
  using multiprecision::math::ceil;
  using multiprecision::math::floor;
  using std::ceil;
  using std::floor;

  const auto min_x = static_cast<std::int32_t>(floor(std::min({p0.x(), p1.x(), p2.x()})));
  const auto min_y = static_cast<std::int32_t>(floor(std::min({p0.y(), p1.y(), p2.y()})));
  const auto max_x = static_cast<std::int32_t>(ceil(std::max({p0.x(), p1.x(), p2.x()})));
  const auto max_y = static_cast<std::int32_t>(ceil(std::max({p0.y(), p1.y(), p2.y()})));

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

  for (std::int32_t y = min_y; y <= max_y; ++y)
  {
    auto w0 = w0_init;
    auto w1 = w1_init;
    auto w2 = w2_init;

    for (std::int32_t x = min_x; x <= max_x; ++x)
    {
      if ((w0 >= 0) && (w1 >= 0) && (w2 >= 0))
      {
        const auto window_z = static_cast<single_precision>((w0 * z0) + (w1 * z1) + (w2 * z2));

        const auto sw0 = static_cast<single_precision>(w0);
        const auto sw1 = static_cast<single_precision>(w1);
        const auto sw2 = static_cast<single_precision>(w2);
        const auto c0 = sw0 * inv_w0;
        const auto c1 = sw1 * inv_w1;
        const auto c2 = sw2 * inv_w2;
        const auto inv_w = c0 + c1 + c2;

        RegisterFile<single_precision, N> varyings;
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
    }

    w0_init += edge_a.x();
    w1_init += edge_b.x();
    w2_init += edge_c.x();
  }
}

} // namespace rtw::sw_renderer
