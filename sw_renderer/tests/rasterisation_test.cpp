#include "sw_renderer/rasterisation_routines.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace rtw::sw_renderer
{
namespace
{

TEST(Rasterisation, draw_line_dda_horizontal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 5};
  const Point2I p1{10, 5};

  draw_line_dda(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U); // 0 to 10 inclusive
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.y(), 5); // All pixels on same y
  }
}

TEST(Rasterisation, draw_line_dda_vertical)
{
  std::vector<Point2I> pixels;
  const Point2I p0{5, 0};
  const Point2I p1{5, 10};

  draw_line_dda(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.x(), 5); // All pixels on same x
  }
}

TEST(Rasterisation, draw_line_dda_diagonal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 0};
  const Point2I p1{10, 10};

  draw_line_dda(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  EXPECT_EQ(pixels.front().x(), p0.x());
  EXPECT_EQ(pixels.front().y(), p0.y());
  EXPECT_EQ(pixels.back().x(), p1.x());
  EXPECT_EQ(pixels.back().y(), p1.y());
}

TEST(Rasterisation, draw_line_bresenham_horizontal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 5};
  const Point2I p1{10, 5};

  draw_line_bresenham(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.y(), 5);
  }
}

TEST(Rasterisation, draw_line_bresenham_vertical)
{
  std::vector<Point2I> pixels;
  const Point2I p0{5, 0};
  const Point2I p1{5, 10};

  draw_line_bresenham(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.x(), 5);
  }
}

TEST(Rasterisation, draw_line_bresenham_diagonal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 0};
  const Point2I p1{10, 10};

  draw_line_bresenham(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  EXPECT_EQ(pixels.front().x(), p0.x());
  EXPECT_EQ(pixels.front().y(), p0.y());
  EXPECT_EQ(pixels.back().x(), p1.x());
  EXPECT_EQ(pixels.back().y(), p1.y());
}

TEST(Rasterisation, draw_line_bresenham_matches_dda_count)
{
  std::vector<Point2I> dda_pixels;
  std::vector<Point2I> bresenham_pixels;
  const Point2I p0{3, 7};
  const Point2I p1{15, 22};

  draw_line_dda(p0, p1, [&dda_pixels](const Point2I& p) { dda_pixels.push_back(p); });
  draw_line_bresenham(p0, p1, [&bresenham_pixels](const Point2I& p) { bresenham_pixels.push_back(p); });

  // Both algorithms should produce similar number of pixels
  // (may differ by 1 due to rounding)
  const auto size_diff =
      std::abs(static_cast<std::int32_t>(dda_pixels.size()) - static_cast<std::int32_t>(bresenham_pixels.size()));
  EXPECT_LE(size_diff, 1);
}

TEST(Rasterisation, is_top_left_top_edge)
{
  // Top edge: y == 0 and x < 0
  const math::Vector2F top_edge{-1.0F, 0.0F};
  EXPECT_TRUE(is_top_left(top_edge));
}

TEST(Rasterisation, is_top_left_left_edge)
{
  // Left edge: y > 0
  const math::Vector2F left_edge{0.5F, 1.0F};
  EXPECT_TRUE(is_top_left(left_edge));
}

TEST(Rasterisation, is_top_left_right_edge)
{
  // Right edge: y < 0
  const math::Vector2F right_edge{0.5F, -1.0F};
  EXPECT_FALSE(is_top_left(right_edge));
}

TEST(Rasterisation, is_top_left_bottom_edge)
{
  // Bottom edge: y == 0 and x > 0
  const math::Vector2F bottom_edge{1.0F, 0.0F};
  EXPECT_FALSE(is_top_left(bottom_edge));
}

TEST(Rasterisation, fill_triangle_bbox_basic)
{
  std::size_t pixel_count = 0;

  // Small triangle
  VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{15.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_bbox(v0, v1, v2,
                     [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                    const Point2I& /*p*/, const BarycentricF& /*b*/) { ++pixel_count; });

  // Triangle should have some pixels filled
  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_basic)
{
  std::size_t pixel_count = 0;

  // Small triangle
  VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{15.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2,
                         [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                        const Point2I& /*p*/) { ++pixel_count; });

  // Triangle should have some pixels filled
  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_flat_bottom)
{
  std::size_t pixel_count = 0;

  // Flat-bottom triangle (v0 and v1 have same y)
  VertexF v0{Point4F{10.0F, 20.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{20.0F, 20.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{15.0F, 10.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2,
                         [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                        const Point2I& /*p*/) { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_flat_top)
{
  std::size_t pixel_count = 0;

  // Flat-top triangle (v1 and v2 have same y)
  VertexF v0{Point4F{15.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{10.0F, 20.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{20.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2,
                         [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                        const Point2I& /*p*/) { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, fill_triangle_scanline_degenerate)
{
  std::size_t pixel_count = 0;

  // Degenerate triangle (all vertices collinear horizontally - zero height)
  VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v1{Point4F{15.0F, 10.0F, 1.0F, 1.0F}};
  VertexF v2{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};

  fill_triangle_scanline(v0, v1, v2,
                         [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                        const Point2I& /*p*/) { ++pixel_count; });

  // Degenerate triangle with zero height should have no pixels
  EXPECT_EQ(pixel_count, 0U);
}

TEST(Rasterisation, programmable_fill_triangle_bbox_basic)
{
  std::size_t pixel_count = 0;

  // Window-space positions (x, y in pixels, z window-depth, w = 1/w_clip). Same winding as the
  // legacy fill_triangle_bbox_basic test, so the signed area is positive and the triangle fills.
  const Vector4F p0{10.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p1{20.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p2{15.0F, 20.0F, 1.0F, 1.0F};

  const RegisterFile<single_precision, 1U> varyings0;
  const RegisterFile<single_precision, 1U> varyings1;
  const RegisterFile<single_precision, 1U> varyings2;

  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2,
                     [&pixel_count](const Point2I& /*p*/, const RegisterFile<single_precision, 1U>& /*varyings*/,
                                    single_precision /*window_z*/, single_precision /*inv_w*/) { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(Rasterisation, programmable_fill_triangle_bbox_affine_interpolation)
{
  // With every vertex at w = 1 (1/w_clip = 1) perspective-correct interpolation reduces to affine
  // interpolation. Feeding each vertex its own screen position as a varying must reconstruct the
  // sample point (pixel centre) at every covered pixel, and the affine window-z (here z = vertex.x)
  // must match the interpolated x. This pins both the barycentric sampling and the interpolation math.
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
  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2,
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

TEST(Rasterisation, programmable_fill_triangle_bbox_perspective_correct_constant)
{
  // Perspective-correct interpolation of a constant attribute must return that constant exactly,
  // independent of the per-vertex 1/w. Use distinct, non-trivial 1/w values so the weights c_i = b_i/w_i
  // differ from the plain barycentrics, then assert the constant survives and 1/w stays positive.
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
  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2,
                     [&pixel_count](const Point2I& /*p*/, const RegisterFile<single_precision, 1U>& varyings,
                                    single_precision /*window_z*/, single_precision inv_w)
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

} // namespace
} // namespace rtw::sw_renderer
