#include "sw_renderer/rasterisation_routines.h"

#include <benchmark/benchmark.h>

namespace
{

void bm_draw_line_dda(benchmark::State& state)
{
  const rtw::math::Point2I p0{80, 40};
  const rtw::math::Point2I p1{140, 100};

  for (auto _ : state)
  {
    rtw::sw_renderer::draw_line_dda(p0, p1,
                                    [](const rtw::math::Point2I& p)
                                    {
                                      auto pp = p;
                                      benchmark::DoNotOptimize(pp);
                                    });
  }
}

void bm_draw_line_bresenham(benchmark::State& state)
{
  const rtw::math::Point2I p0{80, 40};
  const rtw::math::Point2I p1{140, 100};

  for (auto _ : state)
  {
    rtw::sw_renderer::draw_line_bresenham(p0, p1,
                                          [](const rtw::math::Point2I& p)
                                          {
                                            auto pp = p;
                                            benchmark::DoNotOptimize(pp);
                                          });
  }
}

void bm_fill_triangle_scanline(benchmark::State& state)
{
  const rtw::math::Point4F p0{80.0F, 40.0F, 0.0F};
  const rtw::math::Point4F p1{140.0F, 40.0F, 0.0F};
  const rtw::math::Point4F p2{140.0F, 100.0F, 0.0F};

  const rtw::sw_renderer::Vertex4F v0{p0};
  const rtw::sw_renderer::Vertex4F v1{p1};
  const rtw::sw_renderer::Vertex4F v2{p2};

  for (auto _ : state)
  {
    rtw::sw_renderer::fill_triangle_scanline(
        v0, v1, v2,
        [](const rtw::sw_renderer::Vertex4F& v0, const rtw::sw_renderer::Vertex4F& v1,
           const rtw::sw_renderer::Vertex4F& v2, const rtw::math::Point2I& p)
        {
          auto vv0 = v0;
          auto vv1 = v1;
          auto vv2 = v2;
          auto pp = p;
          auto b = rtw::sw_renderer::make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<float>());
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
  const rtw::math::Point4F p0{80.0F, 40.0F, 0.0F};
  const rtw::math::Point4F p1{140.0F, 40.0F, 0.0F};
  const rtw::math::Point4F p2{140.0F, 100.0F, 0.0F};

  const rtw::sw_renderer::Vertex4F v0{p0};
  const rtw::sw_renderer::Vertex4F v1{p1};
  const rtw::sw_renderer::Vertex4F v2{p2};

  for (auto _ : state)
  {
    rtw::sw_renderer::fill_triangle_bbox(v0, v1, v2,
                                         [](const rtw::sw_renderer::Vertex4F& v0, const rtw::sw_renderer::Vertex4F& v1,
                                            const rtw::sw_renderer::Vertex4F& v2, const rtw::math::Point2I& p,
                                            const rtw::sw_renderer::Barycentric3F& b)
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

} // namespace

BENCHMARK(bm_draw_line_dda);
BENCHMARK(bm_draw_line_bresenham);
BENCHMARK(bm_fill_triangle_scanline);
BENCHMARK(bm_fill_triangle_bbox);

BENCHMARK_MAIN();
