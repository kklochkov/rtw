#include "sw_renderer/programmable_pipeline/pipeline_rasterisation.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{
namespace
{

TEST(PipelineRasterisation, fill_triangle_bbox_basic)
{
  std::size_t pixel_count = 0;

  const Vector4F p0{10.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p1{20.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p2{15.0F, 20.0F, 1.0F, 1.0F};

  const RegisterFile<single_precision, 1U> varyings0;
  const RegisterFile<single_precision, 1U> varyings1;
  const RegisterFile<single_precision, 1U> varyings2;

  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2, math::BoundingBoxI{0, 0, 1'023, 1'023},
                     [&pixel_count](const Point2I&, const RegisterFile<single_precision, 1U>&, single_precision,
                                    single_precision) { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(PipelineRasterisation, fill_triangle_bbox_affine_interpolation)
{
  const Vector4F p0{10.0F, 10.0F, 10.0F, 1.0F};
  const Vector4F p1{20.0F, 10.0F, 20.0F, 1.0F};
  const Vector4F p2{15.0F, 20.0F, 15.0F, 1.0F};

  RegisterFile<single_precision, 1U> varyings0;
  RegisterFile<single_precision, 1U> varyings1;
  RegisterFile<single_precision, 1U> varyings2;
  varyings0[0U] = Vector4F{10.0F, 10.0F, 0.0F, 0.0F};
  varyings1[0U] = Vector4F{20.0F, 10.0F, 0.0F, 0.0F};
  varyings2[0U] = Vector4F{15.0F, 20.0F, 0.0F, 0.0F};

  std::size_t pixel_count = 0;
  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2, math::BoundingBoxI{0, 0, 1'023, 1'023},
                     [&pixel_count](const Point2I& p, const RegisterFile<single_precision, 1U>& varyings,
                                    single_precision window_z, single_precision inv_w)
                     {
                       ++pixel_count;
                       const auto expected_x = static_cast<single_precision>(p.x()) + 0.5F;
                       const auto expected_y = static_cast<single_precision>(p.y()) + 0.5F;
                       EXPECT_NEAR(varyings[0U].x(), expected_x, 0.05F);
                       EXPECT_NEAR(varyings[0U].y(), expected_y, 0.05F);
                       EXPECT_NEAR(window_z, expected_x, 0.05F);
                       EXPECT_NEAR(inv_w, 1.0F, 0.05F);
                     });

  EXPECT_GT(pixel_count, 0U);
}

TEST(PipelineRasterisation, fill_triangle_bbox_perspective_correct_constant)
{
  const Vector4F p0{10.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p1{20.0F, 10.0F, 1.0F, 0.5F};
  const Vector4F p2{15.0F, 20.0F, 1.0F, 0.25F};

  constexpr single_precision CONSTANT{7.0F};
  RegisterFile<single_precision, 1U> varyings0;
  RegisterFile<single_precision, 1U> varyings1;
  RegisterFile<single_precision, 1U> varyings2;
  varyings0[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};
  varyings1[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};
  varyings2[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};

  std::size_t pixel_count = 0;
  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2, math::BoundingBoxI{0, 0, 1'023, 1'023},
                     [&pixel_count](const Point2I&, const RegisterFile<single_precision, 1U>& varyings,
                                    single_precision, single_precision inv_w)
                     {
                       ++pixel_count;
                       EXPECT_NEAR(varyings[0U].x(), CONSTANT, 1e-3F);
                       EXPECT_NEAR(varyings[0U].y(), CONSTANT, 1e-3F);
                       EXPECT_NEAR(varyings[0U].z(), CONSTANT, 1e-3F);
                       EXPECT_NEAR(varyings[0U].w(), CONSTANT, 1e-3F);
                       EXPECT_GT(inv_w, 0.0F);
                     });

  EXPECT_GT(pixel_count, 0U);
}

TEST(PipelineRasterisation, fill_triangle_bbox_shared_edge_back_facing_single_cover)
{
  constexpr std::int32_t GRID = 64;
  std::vector<int> coverage(static_cast<std::size_t>(GRID * GRID), 0);

  const Vector4F tl{10.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F tr{30.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F br{30.0F, 30.0F, 1.0F, 1.0F};
  const Vector4F bl{10.0F, 30.0F, 1.0F, 1.0F};

  const RegisterFile<single_precision, 1U> varyings0;
  const RegisterFile<single_precision, 1U> varyings1;
  const RegisterFile<single_precision, 1U> varyings2;

  const auto accumulate =
      [&coverage](const Point2I& p, const RegisterFile<single_precision, 1U>&, single_precision, single_precision)
  { ++coverage[static_cast<std::size_t>((p.y() * GRID) + p.x())]; };

  fill_triangle_bbox(tl, br, tr, varyings0, varyings1, varyings2, math::BoundingBoxI{0, 0, 1'023, 1'023}, accumulate);
  fill_triangle_bbox(tl, bl, br, varyings0, varyings1, varyings2, math::BoundingBoxI{0, 0, 1'023, 1'023}, accumulate);

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
