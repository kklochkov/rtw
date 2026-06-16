#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/clip_space.h"
#include "sw_renderer/programmable_pipeline/register_file.h"

#include "math/vector.h"

#include <gtest/gtest.h>

namespace rtw::sw_renderer
{
namespace
{

ClipVertex<single_precision> make_clip_vertex(const float x, const float y, const float z, const float w,
                                              const float varying = 0.0F)
{
  ClipVertex<single_precision> vertex;
  vertex.position = math::Vector4<single_precision>{x, y, z, w};
  vertex.varyings[0] = math::Vector4<single_precision>{varying, 0.0F, 0.0F, 0.0F};
  return vertex;
}

TEST(ClipSpace, triangle_inside)
{
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(0.5F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, 0.0F, 1.0F);

  const auto result = clip(v0, v1, v2);
  EXPECT_EQ(result.size(), 3U);
}

TEST(ClipSpace, triangle_outside)
{
  const auto v0 = make_clip_vertex(2.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(3.0F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(2.0F, 1.0F, 0.0F, 1.0F);

  const auto result = clip(v0, v1, v2);
  EXPECT_EQ(result.size(), 0U);
}

TEST(ClipSpace, triangle_straddling_near)
{
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(0.5F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, -2.0F, 1.0F);

  const auto result = clip(v0, v1, v2);
  EXPECT_EQ(result.size(), 4U);
}

TEST(ClipSpace, interpolates_varyings)
{
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F, 10.0F);
  const auto v1 = make_clip_vertex(2.0F, 0.0F, 0.0F, 1.0F, 20.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, 0.0F, 1.0F, 30.0F);

  const auto result = clip(v0, v1, v2);

  ASSERT_EQ(result.size(), 4U);
  EXPECT_FLOAT_EQ(result[1].position[0], 1.0F);
  EXPECT_FLOAT_EQ(result[1].varyings[0][0], 15.0F);
}

} // namespace
} // namespace rtw::sw_renderer
