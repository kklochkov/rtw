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

TEST(PipelineRasterisation, draw_line_varyings_covers_endpoints)
{
  std::vector<Point2I> pixels;
  const Vector4F p0{10.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p1{40.0F, 25.0F, 1.0F, 1.0F};
  const RegisterFile<single_precision, 1U> varyings0;
  const RegisterFile<single_precision, 1U> varyings1;

  draw_line_varyings(p0, p1, varyings0, varyings1, math::BoundingBoxI{0, 0, 1'023, 1'023},
                     [&pixels](const Point2I& p, const RegisterFile<single_precision, 1U>&, single_precision,
                               single_precision) { pixels.push_back(p); });

  ASSERT_FALSE(pixels.empty());
  EXPECT_EQ(pixels.front().x(), 10);
  EXPECT_EQ(pixels.front().y(), 10);
  EXPECT_EQ(pixels.back().x(), 40);
  EXPECT_EQ(pixels.back().y(), 25);
}

TEST(PipelineRasterisation, draw_line_varyings_perspective_correct_constant)
{
  const Vector4F p0{10.0F, 20.0F, 1.0F, 1.0F};
  const Vector4F p1{50.0F, 20.0F, 1.0F, 0.5F};

  constexpr single_precision CONSTANT{7.0F};
  RegisterFile<single_precision, 1U> varyings0;
  RegisterFile<single_precision, 1U> varyings1;
  varyings0[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};
  varyings1[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};

  std::size_t pixel_count = 0;
  draw_line_varyings(
      p0, p1, varyings0, varyings1, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&](const Point2I&, const RegisterFile<single_precision, 1U>& varyings, single_precision, single_precision inv_w)
      {
        ++pixel_count;
        EXPECT_NEAR(varyings[0U].x(), CONSTANT, 1e-3F);
        EXPECT_GT(inv_w, 0.0F);
      });

  EXPECT_GT(pixel_count, 0U);
}

TEST(PipelineRasterisation, draw_line_varyings_bresenham_matches_dda)
{
  const Vector4F p0{10.0F, 20.0F, 0.2F, 1.0F};
  const Vector4F p1{50.0F, 20.0F, 0.8F, 0.5F};

  RegisterFile<single_precision, 1U> varyings0;
  RegisterFile<single_precision, 1U> varyings1;
  varyings0[0U] = Vector4F{1.0F, 0.0F, 0.0F, 0.0F};
  varyings1[0U] = Vector4F{0.0F, 1.0F, 0.0F, 0.0F};

  struct Sample
  {
    Point2I pixel;
    Vector4F varying;
    single_precision window_z;
    single_precision inv_w;
  };
  std::vector<Sample> bresenham;
  std::vector<Sample> dda;

  draw_line_varyings(
      p0, p1, varyings0, varyings1, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&bresenham](const Point2I& p, const RegisterFile<single_precision, 1U>& varyings, single_precision window_z,
                   single_precision inv_w) { bresenham.push_back({p, varyings[0U], window_z, inv_w}); },
      LineRaster::BRESENHAM);
  draw_line_varyings(
      p0, p1, varyings0, varyings1, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&dda](const Point2I& p, const RegisterFile<single_precision, 1U>& varyings, single_precision window_z,
             single_precision inv_w) { dda.push_back({p, varyings[0U], window_z, inv_w}); },
      LineRaster::DDA);

  ASSERT_EQ(bresenham.size(), dda.size());
  ASSERT_FALSE(bresenham.empty());
  for (std::size_t i = 0U; i < bresenham.size(); ++i)
  {
    EXPECT_EQ(bresenham[i].pixel.x(), dda[i].pixel.x());
    EXPECT_EQ(bresenham[i].pixel.y(), dda[i].pixel.y());
    EXPECT_NEAR(bresenham[i].varying.x(), dda[i].varying.x(), 1e-4F);
    EXPECT_NEAR(bresenham[i].varying.y(), dda[i].varying.y(), 1e-4F);
    EXPECT_NEAR(bresenham[i].window_z, dda[i].window_z, 1e-4F);
    EXPECT_NEAR(bresenham[i].inv_w, dda[i].inv_w, 1e-4F);
  }
}

TEST(PipelineRasterisation, draw_line_varyings_clips_to_bounds)
{
  std::vector<Point2I> pixels;
  const Vector4F p0{-5.0F, 5.0F, 1.0F, 1.0F};
  const Vector4F p1{5.0F, 5.0F, 1.0F, 1.0F};
  const RegisterFile<single_precision, 1U> varyings0;
  const RegisterFile<single_precision, 1U> varyings1;

  draw_line_varyings(p0, p1, varyings0, varyings1, math::BoundingBoxI{0, 0, 63, 63},
                     [&pixels](const Point2I& p, const RegisterFile<single_precision, 1U>&, single_precision,
                               single_precision) { pixels.push_back(p); });

  ASSERT_FALSE(pixels.empty());
  for (const auto& p : pixels)
  {
    EXPECT_GE(p.x(), 0);
    EXPECT_LE(p.x(), 63);
  }
}

TEST(PipelineRasterisation, draw_point_varyings_covers_vertex)
{
  std::vector<Point2I> pixels;
  const Vector4F p{15.0F, 25.0F, 0.5F, 0.5F};
  RegisterFile<single_precision, 2U> varyings;
  varyings[1U] = Vector4F{1.0F, 2.0F, 3.0F, 4.0F};

  single_precision seen_window_z{0};
  single_precision seen_inv_w{0};
  Vector4F seen_varying{};
  draw_point_varyings(p, varyings, math::BoundingBoxI{0, 0, 1'023, 1'023},
                      [&](const Point2I& pixel, const RegisterFile<single_precision, 2U>& v, single_precision window_z,
                          single_precision inv_w)
                      {
                        pixels.push_back(pixel);
                        seen_window_z = window_z;
                        seen_inv_w = inv_w;
                        seen_varying = v[1U];
                      });

  ASSERT_EQ(pixels.size(), 1U);
  EXPECT_EQ(pixels.front().x(), 15);
  EXPECT_EQ(pixels.front().y(), 25);
  EXPECT_NEAR(seen_window_z, 0.5F, 1e-5F);
  EXPECT_NEAR(seen_inv_w, 0.5F, 1e-5F);
  EXPECT_NEAR(seen_varying.x(), 1.0F, 1e-5F);
  EXPECT_NEAR(seen_varying.w(), 4.0F, 1e-5F);
}

TEST(PipelineRasterisation, draw_point_varyings_out_of_bounds_skipped)
{
  std::size_t count = 0;
  const Vector4F p{-1.0F, 5.0F, 0.5F, 1.0F};
  const RegisterFile<single_precision, 1U> varyings;

  draw_point_varyings(p, varyings, math::BoundingBoxI{0, 0, 63, 63},
                      [&count](const Point2I&, const RegisterFile<single_precision, 1U>&, single_precision,
                               single_precision) { ++count; });

  EXPECT_EQ(count, 0U);
}

TEST(PipelineRasterisation, draw_point_varyings_emits_square_block_for_point_size)
{
  std::vector<Point2I> pixels;
  const Vector4F p{15.0F, 25.0F, 0.5F, 0.5F};
  const RegisterFile<single_precision, 1U> varyings;

  draw_point_varyings(
      p, varyings, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&pixels](const Point2I& pixel, const RegisterFile<single_precision, 1U>&, single_precision, single_precision)
      { pixels.push_back(pixel); }, single_precision{3});

  ASSERT_EQ(pixels.size(), 9U);
  std::int32_t min_x = 1'023;
  std::int32_t max_x = 0;
  std::int32_t min_y = 1'023;
  std::int32_t max_y = 0;
  for (const auto& pixel : pixels)
  {
    min_x = std::min(min_x, pixel.x());
    max_x = std::max(max_x, pixel.x());
    min_y = std::min(min_y, pixel.y());
    max_y = std::max(max_y, pixel.y());
  }
  EXPECT_EQ(min_x, 14);
  EXPECT_EQ(max_x, 16);
  EXPECT_EQ(min_y, 24);
  EXPECT_EQ(max_y, 26);
}

TEST(PipelineRasterisation, draw_point_varyings_square_block_clamps_to_bounds)
{
  std::vector<Point2I> pixels;
  const Vector4F p{0.0F, 0.0F, 0.5F, 1.0F};
  const RegisterFile<single_precision, 1U> varyings;

  draw_point_varyings(
      p, varyings, math::BoundingBoxI{0, 0, 63, 63},
      [&pixels](const Point2I& pixel, const RegisterFile<single_precision, 1U>&, single_precision, single_precision)
      { pixels.push_back(pixel); }, single_precision{3});

  ASSERT_EQ(pixels.size(), 4U);
  for (const auto& pixel : pixels)
  {
    EXPECT_GE(pixel.x(), 0);
    EXPECT_LE(pixel.x(), 1);
    EXPECT_GE(pixel.y(), 0);
    EXPECT_LE(pixel.y(), 1);
  }
}

} // namespace
} // namespace rtw::sw_renderer
