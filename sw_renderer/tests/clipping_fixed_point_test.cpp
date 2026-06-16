#include "sw_renderer/clip_space.h"

#include "sw_renderer/precision.h"
#include "sw_renderer/register_file.h"
#include "sw_renderer/types.h"

#include "multiprecision/fixed_point.h"

#include <gtest/gtest.h>

// Fixed-point coverage for the clip-space clipper. Built via cc_test_with_fixed_point (see
// BUILD), whose config transition compiles this target with RTW_USE_FIXED_POINT, so single_precision is
// FixedPoint16 and double_precision is FixedPoint32. It exercises the homogeneous clipper in fixed-point
// mode -- the FixedPoint32 plane-distance widening and the narrowing back to FixedPoint16 -- and checks
// the clipped vertex count and the interpolated position / varying values.
namespace
{

namespace sw = rtw::sw_renderer;

static_assert(rtw::multiprecision::IS_FIXED_POINT_V<sw::single_precision>,
              "This target must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

constexpr double TOLERANCE = 1.0e-3;

sw::ClipVertex<sw::single_precision> make_clip_vertex(const float x, const float y, const float z, const float w,
                                                      const float varying = 0.0F)
{
  sw::ClipVertex<sw::single_precision> vertex;
  vertex.position =
      sw::Vector4F{sw::single_precision{x}, sw::single_precision{y}, sw::single_precision{z}, sw::single_precision{w}};
  vertex.varyings[0U] = sw::Vector4F{sw::single_precision{varying}, sw::single_precision{0}, sw::single_precision{0},
                                     sw::single_precision{0}};
  return vertex;
}

TEST(ClippingFixedPoint, clip_space_triangle_inside_runs)
{
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(0.5F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, 0.0F, 1.0F);

  const auto result = sw::clip(v0, v1, v2);

  EXPECT_EQ(result.size(), 3U);
}

TEST(ClippingFixedPoint, clip_space_triangle_outside_runs)
{
  const auto v0 = make_clip_vertex(2.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(3.0F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(2.0F, 1.0F, 0.0F, 1.0F);

  const auto result = sw::clip(v0, v1, v2);

  EXPECT_EQ(result.size(), 0U);
}

TEST(ClippingFixedPoint, clip_space_straddling_interpolates_varyings)
{
  // v0->v1 crosses the right plane at t = 0.5; the slot-0 varying lerps 10 -> 20 = 15.
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F, 10.0F);
  const auto v1 = make_clip_vertex(2.0F, 0.0F, 0.0F, 1.0F, 20.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, 0.0F, 1.0F, 30.0F);

  const auto result = sw::clip(v0, v1, v2);

  ASSERT_EQ(result.size(), 4U);
  EXPECT_NEAR(static_cast<double>(result[1U].position[0U]), 1.0, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(result[1U].varyings[0U][0U]), 15.0, TOLERANCE);
}

} // namespace
