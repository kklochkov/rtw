#include "sw_renderer/fixed_pipeline/rasterisation_routines.h"
#include "sw_renderer/types.h"

#include "multiprecision/fixed_point.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{
namespace
{

static_assert(multiprecision::IS_FIXED_POINT_V<single_precision>,
              "This test must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

TEST(RasterisationFixedPoint, small_triangle_fills_pixels)
{
  std::size_t pixel_count = 0;
  const VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v1{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v2{Point4F{15.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_bbox(v0, v1, v2,
                     [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&, const BarycentricF&)
                     { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(RasterisationFixedPoint, large_triangle_does_not_overflow)
{
  std::size_t pixel_count = 0;
  const VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v1{Point4F{260.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v2{Point4F{10.0F, 260.0F, 1.0F, 1.0F}};

  fill_triangle_bbox(v0, v1, v2,
                     [&pixel_count](const VertexF&, const VertexF&, const VertexF&, const Point2I&, const BarycentricF&)
                     { ++pixel_count; });

  EXPECT_GT(pixel_count, 30'000U);
  EXPECT_LT(pixel_count, 32'000U);
}

TEST(RasterisationFixedPoint, barycentric_coordinates_are_valid)
{
  const VertexF v0{Point4F{20.0F, 20.0F, 1.0F, 1.0F}};
  const VertexF v1{Point4F{220.0F, 40.0F, 1.0F, 1.0F}};
  const VertexF v2{Point4F{60.0F, 240.0F, 1.0F, 1.0F}};

  double max_sum_error = 0.0;
  bool all_non_negative = true;
  std::size_t pixel_count = 0;

  fill_triangle_bbox(v0, v1, v2,
                     [&](const VertexF&, const VertexF&, const VertexF&, const Point2I&, const BarycentricF& b)
                     {
                       ++pixel_count;
                       all_non_negative = all_non_negative && (b.w0() >= single_precision{0})
                                       && (b.w1() >= single_precision{0}) && (b.w2() >= single_precision{0});
                       const double sum =
                           static_cast<double>(b.w0()) + static_cast<double>(b.w1()) + static_cast<double>(b.w2());
                       max_sum_error = std::max(max_sum_error, std::abs(sum - 1.0));
                     });

  EXPECT_GT(pixel_count, 0U);
  EXPECT_TRUE(all_non_negative);
  EXPECT_LT(max_sum_error, 0.01);
}

TEST(RasterisationFixedPoint, shared_edge_back_facing_single_cover)
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

} // namespace
} // namespace rtw::sw_renderer
