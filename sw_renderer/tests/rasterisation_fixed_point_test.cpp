#include "sw_renderer/rasterisation_routines.h"
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

// This target is built via cc_test_with_fixed_point (see BUILD), whose config transition compiles it
// with RTW_USE_FIXED_POINT so that single_precision is FixedPoint16 (Q15.16) and double_precision is
// FixedPoint32 (Q31.32). It guards the fixed-point rasterisation path: fill_triangle_bbox computes edge
// functions in double_precision, so the screen-space cross products do not overflow for large triangles.
// With FixedPoint16 alone (max ~32768) the area/barycentric cross products saturate once a triangle
// spans more than ~181 pixels, corrupting the fill.
static_assert(multiprecision::IS_FIXED_POINT_V<single_precision>,
              "This test must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

TEST(RasterisationFixedPoint, small_triangle_fills_pixels)
{
  std::size_t pixel_count = 0;
  const VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v1{Point4F{20.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v2{Point4F{15.0F, 20.0F, 1.0F, 1.0F}};

  fill_triangle_bbox(v0, v1, v2,
                     [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                    const Point2I& /*p*/, const BarycentricF& /*b*/) { ++pixel_count; });

  EXPECT_GT(pixel_count, 0U);
}

TEST(RasterisationFixedPoint, large_triangle_does_not_overflow)
{
  // Right triangle with 250-pixel legs: idealised coverage is ~sum_{k=1}^{250} k = 31375 px. Each leg
  // (250) exceeds the ~181px FixedPoint16 overflow threshold, so before widening the edge functions to
  // FixedPoint32 the area/barycentric cross products saturated and the fill produced a wildly wrong
  // pixel count (regression guard for the cross-product overflow fix).
  std::size_t pixel_count = 0;
  const VertexF v0{Point4F{10.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v1{Point4F{260.0F, 10.0F, 1.0F, 1.0F}};
  const VertexF v2{Point4F{10.0F, 260.0F, 1.0F, 1.0F}};

  fill_triangle_bbox(v0, v1, v2,
                     [&pixel_count](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/,
                                    const Point2I& /*p*/, const BarycentricF& /*b*/) { ++pixel_count; });

  // Expect approximately the geometric coverage, far from the near-zero / garbage counts a saturated
  // cross product would yield.
  EXPECT_GT(pixel_count, 30'000U);
  EXPECT_LT(pixel_count, 32'000U);
}

TEST(RasterisationFixedPoint, barycentric_coordinates_are_valid)
{
  // For every covered pixel of a large triangle the barycentric weights must be non-negative and sum
  // to ~1. A saturated area would break this invariant.
  const VertexF v0{Point4F{20.0F, 20.0F, 1.0F, 1.0F}};
  const VertexF v1{Point4F{220.0F, 40.0F, 1.0F, 1.0F}};
  const VertexF v2{Point4F{60.0F, 240.0F, 1.0F, 1.0F}};

  double max_sum_error = 0.0;
  bool all_non_negative = true;
  std::size_t pixel_count = 0;

  fill_triangle_bbox(v0, v1, v2,
                     [&](const VertexF& /*v0*/, const VertexF& /*v1*/, const VertexF& /*v2*/, const Point2I& /*p*/,
                         const BarycentricF& b)
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
  EXPECT_LT(max_sum_error, 0.01); // Well above FixedPoint16 barycentric resolution (~2^-16).
}

TEST(RasterisationFixedPoint, programmable_large_triangle_preserves_constant_varying)
{
  // Right triangle with 250-pixel legs (same geometry as large_triangle_does_not_overflow) exercising
  // the programmable overload, whose perspective-correct varying combine and affine window-z also run
  // in double_precision (FixedPoint32). A constant varying must survive perspective-correct interpolation unchanged,
  // and the coverage must match the geometric area (~31375 px); a saturated FixedPoint16 cross product would corrupt
  // both.
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

  fill_triangle_bbox(p0, p1, p2, varyings0, varyings1, varyings2,
                     [&](const Point2I& /*p*/, const RegisterFile<single_precision, 1U>& varyings,
                         single_precision /*window_z*/, single_precision inv_w)
                     {
                       ++pixel_count;
                       inv_w_positive = inv_w_positive && (inv_w > single_precision{0});
                       const auto& v = varyings[0U];
                       const auto expected = static_cast<double>(CONSTANT);
                       max_constant_error =
                           std::max({max_constant_error, std::abs(static_cast<double>(v.x()) - expected),
                                     std::abs(static_cast<double>(v.y()) - expected),
                                     std::abs(static_cast<double>(v.z()) - expected),
                                     std::abs(static_cast<double>(v.w()) - expected)});
                     });

  EXPECT_GT(pixel_count, 30'000U);
  EXPECT_LT(pixel_count, 32'000U);
  EXPECT_TRUE(inv_w_positive);
  EXPECT_LT(max_constant_error, 0.01); // Well above FixedPoint16 resolution (~2^-16).
}

} // namespace
} // namespace rtw::sw_renderer
