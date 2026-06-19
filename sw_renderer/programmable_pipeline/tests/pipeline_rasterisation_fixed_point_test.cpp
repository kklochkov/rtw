#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/pipeline_rasterisation.h"
#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/types.h"

#include "multiprecision/fixed_point.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace rtw::sw_renderer
{
namespace
{

static_assert(multiprecision::IS_FIXED_POINT_V<single_precision>,
              "This test must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

TEST(PipelineRasterisationFixedPoint, large_triangle_preserves_constant_varying)
{
  const Vector4F p0{10.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p1{260.0F, 10.0F, 1.0F, 1.0F};
  const Vector4F p2{10.0F, 260.0F, 1.0F, 1.0F};

  constexpr single_precision CONSTANT{0.25F};
  RegisterFile<single_precision, 1U> varyings0;
  RegisterFile<single_precision, 1U> varyings1;
  RegisterFile<single_precision, 1U> varyings2;
  varyings0[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};
  varyings1[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};
  varyings2[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};

  std::size_t pixel_count = 0;
  double max_constant_error = 0.0;
  bool inv_w_positive = true;

  fill_triangle_bbox(
      p0, p1, p2, varyings0, varyings1, varyings2, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&](const Point2I&, const RegisterFile<single_precision, 1U>& varyings, single_precision, single_precision inv_w)
      {
        ++pixel_count;
        inv_w_positive = inv_w_positive && (inv_w > single_precision{0});
        const auto& v = varyings[0U];
        const auto expected = static_cast<double>(CONSTANT);
        max_constant_error =
            std::max({max_constant_error, std::abs(static_cast<double>(v.x()) - expected),
                      std::abs(static_cast<double>(v.y()) - expected), std::abs(static_cast<double>(v.z()) - expected),
                      std::abs(static_cast<double>(v.w()) - expected)});
      });

  EXPECT_GT(pixel_count, 30'000U);
  EXPECT_LT(pixel_count, 32'000U);
  EXPECT_TRUE(inv_w_positive);
  EXPECT_LT(max_constant_error, 0.01);
}

TEST(PipelineRasterisationFixedPoint, draw_line_varyings_preserves_constant_varying)
{
  const Vector4F p0{10.0F, 20.0F, 1.0F, 1.0F};
  const Vector4F p1{200.0F, 20.0F, 1.0F, 0.5F};

  constexpr single_precision CONSTANT{0.25F};
  RegisterFile<single_precision, 1U> varyings0;
  RegisterFile<single_precision, 1U> varyings1;
  varyings0[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};
  varyings1[0U] = Vector4F{CONSTANT, CONSTANT, CONSTANT, CONSTANT};

  std::size_t pixel_count = 0;
  double max_constant_error = 0.0;
  bool inv_w_positive = true;

  draw_line_varyings(
      p0, p1, varyings0, varyings1, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&](const Point2I&, const RegisterFile<single_precision, 1U>& varyings, single_precision, single_precision inv_w)
      {
        ++pixel_count;
        inv_w_positive = inv_w_positive && (inv_w > single_precision{0});
        const auto& v = varyings[0U];
        const auto expected = static_cast<double>(CONSTANT);
        max_constant_error = std::max({max_constant_error, std::abs(static_cast<double>(v.x()) - expected),
                                       std::abs(static_cast<double>(v.w()) - expected)});
      });

  EXPECT_GT(pixel_count, 100U);
  EXPECT_TRUE(inv_w_positive);
  EXPECT_LT(max_constant_error, 0.01);
}

TEST(PipelineRasterisationFixedPoint, draw_point_varyings_forwards_vertex_varying)
{
  const Vector4F p{15.0F, 25.0F, 0.5F, 0.5F};
  RegisterFile<single_precision, 2U> varyings;
  varyings[1U] = Vector4F{0.25F, 0.5F, 0.75F, 1.0F};

  std::size_t count = 0;
  Point2I seen_pixel{};
  single_precision seen_window_z{0};
  single_precision seen_inv_w{0};
  Vector4F seen_varying{};

  draw_point_varyings(p, varyings, math::BoundingBoxI{0, 0, 1'023, 1'023},
                      [&](const Point2I& pixel, const RegisterFile<single_precision, 2U>& v, single_precision window_z,
                          single_precision inv_w)
                      {
                        ++count;
                        seen_pixel = pixel;
                        seen_window_z = window_z;
                        seen_inv_w = inv_w;
                        seen_varying = v[1U];
                      });

  ASSERT_EQ(count, 1U);
  EXPECT_EQ(seen_pixel.x(), 15);
  EXPECT_EQ(seen_pixel.y(), 25);
  EXPECT_NEAR(static_cast<double>(seen_window_z), 0.5, 1e-3);
  EXPECT_NEAR(static_cast<double>(seen_inv_w), 0.5, 1e-3);
  EXPECT_NEAR(static_cast<double>(seen_varying.x()), 0.25, 1e-3);
  EXPECT_NEAR(static_cast<double>(seen_varying.w()), 1.0, 1e-3);
}

TEST(PipelineRasterisationFixedPoint, draw_point_varyings_emits_square_block_for_point_size)
{
  const Vector4F p{15.0F, 25.0F, 0.5F, 0.5F};
  const RegisterFile<single_precision, 1U> varyings;

  std::size_t count = 0;
  draw_point_varyings(
      p, varyings, math::BoundingBoxI{0, 0, 1'023, 1'023},
      [&count](const Point2I&, const RegisterFile<single_precision, 1U>&, single_precision, single_precision)
      { ++count; }, single_precision{3});

  EXPECT_EQ(count, 9U);
}

} // namespace
} // namespace rtw::sw_renderer
