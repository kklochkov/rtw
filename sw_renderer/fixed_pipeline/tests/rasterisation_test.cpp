#include "sw_renderer/fixed_pipeline/rasterisation_routines.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{
namespace
{

TEST(Rasterisation, fill_triangle_bbox_basic)
{
  std::size_t pixel_count = 0;

  VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{15.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_bbox(v0, v1, v2,
                     [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&, const BarycentricF&)
                     { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_bbox_shared_edge_single_cover)
{
  constexpr std::int32_t GRID = 64;
  std::vector<int> coverage(static_cast<std::size_t>(GRID * GRID), 0);

  const VertexF tl{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF tr{Point4F{30.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF br{Point4F{30.0F, 30.0F, 1.0F, 1.0F}};
  const VertexF bl{Point4F{10.0F, 30.0F, 1.0F, 1.0F}};

  const auto accumulate =
      [&coverage](const VertexF&, const VertexF&, const VertexF&, const Point2I& p, const BarycentricF&)
  { ++coverage[static_cast<std::size_t>((p.y() * GRID) + p.x())]; };

  fill_triangle_bbox(tl, tr, br, accumulate);
  fill_triangle_bbox(tl, br, bl, accumulate);

  int max_coverage = 0;
  int total_coverage = 0;
  for (const int count : coverage)
  {
    max_coverage = std::max(max_coverage, count);
    total_coverage += count;
  }
  EXPECT_EQ(max_coverage, 1);
  EXPECT_GT(total_coverage, 0);
}

TEST(Rasterisation, fill_triangle_bbox_shared_edge_back_facing_single_cover)
{
  constexpr std::int32_t GRID = 64;
  std::vector<int> coverage(static_cast<std::size_t>(GRID * GRID), 0);

  const VertexF tl{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF tr{Point4F{30.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF br{Point4F{30.0F, 30.0F, 1.0F, 1.0F}};
  const VertexF bl{Point4F{10.0F, 30.0F, 1.0F, 1.0F}};

  const auto accumulate =
      [&coverage](const VertexF&, const VertexF&, const VertexF&, const Point2I& p, const BarycentricF&)
  { ++coverage[static_cast<std::size_t>((p.y() * GRID) + p.x())]; };

  fill_triangle_bbox(tl, br, tr, accumulate);
  fill_triangle_bbox(tl, bl, br, accumulate);

  int max_coverage = 0;
  int total_coverage = 0;
  for (const int count : coverage)
  {
    max_coverage = std::max(max_coverage, count);
    total_coverage += count;
  }
  EXPECT_EQ(max_coverage, 1);
  EXPECT_GT(total_coverage, 0);
}

TEST(Rasterisation, fill_triangle_scanline_basic)
{
  std::size_t pixel_count = 0;

  VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{15.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2, [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&)
                         { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_flat_bottom)
{
  std::size_t pixel_count = 0;

  VertexF v0{Point4F{10.0F, 20.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{20.0F, 20.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{15.0F, 10.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2, [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&)
                         { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_flat_top)
{
  std::size_t pixel_count = 0;

  VertexF v0{Point4F{15.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{10.0F, 20.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{20.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2, [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&)
                         { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_degenerate)
{
  std::size_t pixel_count = 0;

  VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{15.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2, [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&)
                         { ++pixel_count; });

  EXPECT_EQ(pixel_count, 0U);
}

} // namespace
} // namespace rtw::sw_renderer
