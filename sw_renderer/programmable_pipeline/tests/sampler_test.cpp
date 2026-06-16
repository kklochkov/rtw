#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

namespace
{
namespace sw = rtw::sw_renderer;

/// Build a 2x2 RGBA texture:
///   (0,0) red    (1,0) green
///   (0,1) blue   (1,1) white
sw::Texture make_2x2_texture()
{
  std::array<std::uint32_t, 4> texels{
      0xFF'00'00'FFU, // (0,0) red
      0x00'FF'00'FFU, // (1,0) green
      0x00'00'FF'FFU, // (0,1) blue
      0xFF'FF'FF'FFU, // (1,1) white
  };
  return sw::Texture{texels.data(), 2, 2};
}
} // namespace

TEST(Sampler2D, nearest_samples_each_texel)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::NEAREST};

  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(0.25F, 0.25F)), ::testing::ElementsAre(1, 0, 0, 1)); // red
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(0.75F, 0.25F)), ::testing::ElementsAre(0, 1, 0, 1)); // green
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(0.25F, 0.75F)), ::testing::ElementsAre(0, 0, 1, 1)); // blue
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(0.75F, 0.75F)), ::testing::ElementsAre(1, 1, 1, 1)); // white
}

TEST(Sampler2D, linear_blends_four_texels_at_center)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::LINEAR};

  // The texel centers sit at (0.25, 0.25) ... (0.75, 0.75); sampling the middle averages all four.
  const auto color = sampler.sample(0.5F, 0.5F);
  EXPECT_FLOAT_EQ(color.x(), 0.5F); // r
  EXPECT_FLOAT_EQ(color.y(), 0.5F); // g
  EXPECT_FLOAT_EQ(color.z(), 0.5F); // b
  EXPECT_FLOAT_EQ(color.w(), 1.0F); // a

  const sw::Color c{color};
  EXPECT_EQ(c.r(), 127); // r
  EXPECT_EQ(c.g(), 127); // g
  EXPECT_EQ(c.b(), 127); // b
  EXPECT_EQ(c.a(), 255); // a
}

TEST(Sampler2D, linear_at_texel_center_returns_that_texel)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::LINEAR};

  // At the exact center of texel (0,0) the bilinear weights collapse onto a single texel.
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(0.25F, 0.25F)), ::testing::ElementsAre(1, 0, 0, 1));
}

TEST(Sampler2D, repeat_wraps_out_of_range)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::REPEAT, sw::FilterMode::NEAREST};

  // u = 1.25 wraps to column 0 (same as u = 0.25).
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(1.25F, 0.25F)), ::testing::ElementsAre(1, 0, 0, 1)); // red
  // u = -0.25 -> floor(-0.5) = -1 -> wraps to column 1.
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(-0.25F, 0.25F)), ::testing::ElementsAre(0, 1, 0, 1)); // green
}

TEST(Sampler2D, clamp_to_edge_clamps_out_of_range)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::NEAREST};

  // u = 1.5 -> column 3 -> clamped to 1; v = -0.5 -> row -1 -> clamped to 0 => texel(1,0) green.
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(1.5F, -0.5F)), ::testing::ElementsAre(0, 1, 0, 1));
}

TEST(Sampler2D, mirrored_repeat_mirrors_out_of_range)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::MIRRORED_REPEAT, sw::FilterMode::NEAREST};

  // u = 1.25 -> floor(2.5) = 2 -> mirrored back to column 1 => texel(1,0) green.
  EXPECT_THAT(static_cast<sw::Vector4F>(sampler.sample(1.25F, 0.25F)), ::testing::ElementsAre(0, 1, 0, 1));
}
