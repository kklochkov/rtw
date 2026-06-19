#include "math/barycentric.h"
#include "math/barycentric_operations.h"
#include "math/vector_operations.h"
#include "sw_renderer/fixed_pipeline/rasterisation_routines.h"
#include "sw_renderer/programmable_pipeline/pipeline_rasterisation.h"
#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/raster_common.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex.h"

#include <benchmark/benchmark.h>

namespace
{

using Varyings =
    rtw::sw_renderer::RegisterFile<rtw::sw_renderer::single_precision, rtw::sw_renderer::MAX_VARYING_COUNT>;

void bm_draw_line_dda(benchmark::State& state)
{
  const rtw::sw_renderer::Point2I p0{80, 40};
  const rtw::sw_renderer::Point2I p1{140, 100};

  for (auto _ : state)
  {
    rtw::sw_renderer::draw_line_dda(p0, p1,
                                    [](const rtw::sw_renderer::Point2I& p)
                                    {
                                      auto pp = p;
                                      benchmark::DoNotOptimize(pp);
                                    });
  }
}

void bm_draw_line_bresenham(benchmark::State& state)
{
  const rtw::sw_renderer::Point2I p0{80, 40};
  const rtw::sw_renderer::Point2I p1{140, 100};

  for (auto _ : state)
  {
    rtw::sw_renderer::draw_line_bresenham(p0, p1,
                                          [](const rtw::sw_renderer::Point2I& p)
                                          {
                                            auto pp = p;
                                            benchmark::DoNotOptimize(pp);
                                          });
  }
}

void bm_fill_triangle_scanline(benchmark::State& state)
{
  const rtw::sw_renderer::Point4F p0{80.0F, 40.0F, 0.0F};
  const rtw::sw_renderer::Point4F p1{140.0F, 40.0F, 0.0F};
  const rtw::sw_renderer::Point4F p2{140.0F, 100.0F, 0.0F};

  const rtw::sw_renderer::VertexF v0{p0};
  const rtw::sw_renderer::VertexF v1{p1};
  const rtw::sw_renderer::VertexF v2{p2};

  for (auto _ : state)
  {
    rtw::sw_renderer::fill_triangle_scanline(
        v0, v1, v2,
        [](const rtw::sw_renderer::VertexF& v0, const rtw::sw_renderer::VertexF& v1,
           const rtw::sw_renderer::VertexF& v2, const rtw::sw_renderer::Point2I& p)
        {
          auto vv0 = v0;
          auto vv1 = v1;
          auto vv2 = v2;
          auto pp = p;
          auto b = rtw::math::make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(),
                                               p.cast<rtw::sw_renderer::single_precision>());
          benchmark::DoNotOptimize(vv0);
          benchmark::DoNotOptimize(vv1);
          benchmark::DoNotOptimize(vv2);
          benchmark::DoNotOptimize(pp);
          benchmark::DoNotOptimize(b);
        });
  }
}

void bm_fill_triangle_bbox(benchmark::State& state)
{
  const rtw::sw_renderer::Point4F p0{80.0F, 40.0F, 0.0F};
  const rtw::sw_renderer::Point4F p1{140.0F, 40.0F, 0.0F};
  const rtw::sw_renderer::Point4F p2{140.0F, 100.0F, 0.0F};

  const rtw::sw_renderer::VertexF v0{p0};
  const rtw::sw_renderer::VertexF v1{p1};
  const rtw::sw_renderer::VertexF v2{p2};

  for (auto _ : state)
  {
    rtw::sw_renderer::fill_triangle_bbox(v0, v1, v2,
                                         [](const rtw::sw_renderer::VertexF& v0, const rtw::sw_renderer::VertexF& v1,
                                            const rtw::sw_renderer::VertexF& v2, const rtw::sw_renderer::Point2I& p,
                                            const rtw::sw_renderer::BarycentricF& b)
                                         {
                                           auto vv0 = v0;
                                           auto vv1 = v1;
                                           auto vv2 = v2;
                                           auto pp = p;
                                           auto bb = b;
                                           benchmark::DoNotOptimize(vv0);
                                           benchmark::DoNotOptimize(vv1);
                                           benchmark::DoNotOptimize(vv2);
                                           benchmark::DoNotOptimize(pp);
                                           benchmark::DoNotOptimize(bb);
                                         });
  }
}

void bm_draw_line_varyings_dda(benchmark::State& state)
{
  const rtw::sw_renderer::Vector4F p0{80.0F, 40.0F, 0.2F, 1.0F};
  const rtw::sw_renderer::Vector4F p1{140.0F, 100.0F, 0.8F, 0.5F};

  Varyings v0;
  Varyings v1;
  v0[0U] = rtw::sw_renderer::Vector4F{1.0F, 0.0F, 0.0F, 1.0F};
  v1[0U] = rtw::sw_renderer::Vector4F{0.0F, 1.0F, 0.0F, 1.0F};
  v0[1U] = rtw::sw_renderer::Vector4F{0.0F, 0.0F, 0.0F, 0.0F};
  v1[1U] = rtw::sw_renderer::Vector4F{1.0F, 1.0F, 0.0F, 0.0F};

  const rtw::math::BoundingBoxI bounds{0, 0, 1'023, 1'023};

  for (auto _ : state)
  {
    rtw::sw_renderer::draw_line_varyings(
        p0, p1, v0, v1, bounds,
        [](const rtw::sw_renderer::Point2I& p, const Varyings& varyings, rtw::sw_renderer::single_precision window_z,
           rtw::sw_renderer::single_precision inv_w)
        {
          auto pp = p;
          auto vv = varyings[0U];
          benchmark::DoNotOptimize(pp);
          benchmark::DoNotOptimize(vv);
          benchmark::DoNotOptimize(window_z);
          benchmark::DoNotOptimize(inv_w);
        },
        rtw::sw_renderer::LineRaster::DDA);
  }
}

void bm_draw_line_varyings_bresenham(benchmark::State& state)
{
  const rtw::sw_renderer::Vector4F p0{80.0F, 40.0F, 0.2F, 1.0F};
  const rtw::sw_renderer::Vector4F p1{140.0F, 100.0F, 0.8F, 0.5F};

  Varyings v0;
  Varyings v1;
  v0[0U] = rtw::sw_renderer::Vector4F{1.0F, 0.0F, 0.0F, 1.0F};
  v1[0U] = rtw::sw_renderer::Vector4F{0.0F, 1.0F, 0.0F, 1.0F};
  v0[1U] = rtw::sw_renderer::Vector4F{0.0F, 0.0F, 0.0F, 0.0F};
  v1[1U] = rtw::sw_renderer::Vector4F{1.0F, 1.0F, 0.0F, 0.0F};

  const rtw::math::BoundingBoxI bounds{0, 0, 1'023, 1'023};

  for (auto _ : state)
  {
    rtw::sw_renderer::draw_line_varyings(
        p0, p1, v0, v1, bounds,
        [](const rtw::sw_renderer::Point2I& p, const Varyings& varyings, rtw::sw_renderer::single_precision window_z,
           rtw::sw_renderer::single_precision inv_w)
        {
          auto pp = p;
          auto vv = varyings[0U];
          benchmark::DoNotOptimize(pp);
          benchmark::DoNotOptimize(vv);
          benchmark::DoNotOptimize(window_z);
          benchmark::DoNotOptimize(inv_w);
        },
        rtw::sw_renderer::LineRaster::BRESENHAM);
  }
}

} // namespace

BENCHMARK(bm_draw_line_dda);
BENCHMARK(bm_draw_line_bresenham);
BENCHMARK(bm_draw_line_varyings_dda);
BENCHMARK(bm_draw_line_varyings_bresenham);
BENCHMARK(bm_fill_triangle_scanline);
BENCHMARK(bm_fill_triangle_bbox);

BENCHMARK_MAIN();
