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

} // namespace
} // namespace rtw::sw_renderer
