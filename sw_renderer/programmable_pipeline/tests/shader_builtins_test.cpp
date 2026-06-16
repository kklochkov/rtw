#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/programmable_pipeline/shader_builtins.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include "math/vector_operations.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

namespace
{
namespace sw = rtw::sw_renderer;
} // namespace

// --- mix --------------------------------------------------------------------

TEST(ShaderBuiltins, mix_scalar)
{
  EXPECT_FLOAT_EQ(sw::mix(0.0F, 10.0F, 0.5F), 5.0F);
  EXPECT_FLOAT_EQ(sw::mix(0.0F, 10.0F, 0.0F), 0.0F);
  EXPECT_FLOAT_EQ(sw::mix(0.0F, 10.0F, 1.0F), 10.0F);
}

TEST(ShaderBuiltins, mix_vector_scalar_factor)
{
  const auto result = sw::mix(sw::Vector3F{0.0F, 0.0F, 0.0F}, sw::Vector3F{2.0F, 4.0F, 6.0F}, 0.5F);
  EXPECT_THAT(result, ::testing::ElementsAre(1, 2, 3));
}

TEST(ShaderBuiltins, mix_vector_componentwise_factor)
{
  const auto result =
      sw::mix(sw::Vector3F{0.0F, 0.0F, 0.0F}, sw::Vector3F{10.0F, 10.0F, 10.0F}, sw::Vector3F{0.1F, 0.5F, 0.9F});
  EXPECT_THAT(result,
              ::testing::ElementsAre(::testing::FloatEq(1.0F), ::testing::FloatEq(5.0F), ::testing::FloatEq(9.0F)));
}

// --- saturate ---------------------------------------------------------------

TEST(ShaderBuiltins, saturate_scalar)
{
  EXPECT_FLOAT_EQ(sw::saturate(-1.0F), 0.0F);
  EXPECT_FLOAT_EQ(sw::saturate(0.5F), 0.5F);
  EXPECT_FLOAT_EQ(sw::saturate(2.0F), 1.0F);
}

TEST(ShaderBuiltins, saturate_vector)
{
  const auto result = sw::saturate(sw::Vector3F{-1.0F, 0.5F, 2.0F});
  EXPECT_THAT(result, ::testing::ElementsAre(0, 0.5F, 1));
}

// --- step -------------------------------------------------------------------

TEST(ShaderBuiltins, step_scalar)
{
  EXPECT_FLOAT_EQ(sw::step(0.5F, 0.25F), 0.0F); // x < edge
  EXPECT_FLOAT_EQ(sw::step(0.5F, 0.75F), 1.0F); // x >= edge
  EXPECT_FLOAT_EQ(sw::step(0.5F, 0.5F), 1.0F);  // x == edge
}

TEST(ShaderBuiltins, step_vector)
{
  const auto result = sw::step(sw::Vector3F{0.5F, 0.5F, 0.5F}, sw::Vector3F{0.25F, 0.5F, 0.75F});
  EXPECT_THAT(result, ::testing::ElementsAre(0, 1, 1));
}

TEST(ShaderBuiltins, step_scalar_edge_broadcast)
{
  const auto result = sw::step(0.5F, sw::Vector3F{0.25F, 0.5F, 0.75F});
  EXPECT_THAT(result, ::testing::ElementsAre(0, 1, 1));
}

// --- smoothstep -------------------------------------------------------------

TEST(ShaderBuiltins, smoothstep_scalar)
{
  EXPECT_FLOAT_EQ(sw::smoothstep(0.0F, 1.0F, -1.0F), 0.0F); // below lower edge
  EXPECT_FLOAT_EQ(sw::smoothstep(0.0F, 1.0F, 0.5F), 0.5F);  // midpoint
  EXPECT_FLOAT_EQ(sw::smoothstep(0.0F, 1.0F, 2.0F), 1.0F);  // above upper edge
}

TEST(ShaderBuiltins, smoothstep_vector_scalar_edges)
{
  const auto result = sw::smoothstep(0.0F, 1.0F, sw::Vector3F{-1.0F, 0.5F, 2.0F});
  EXPECT_THAT(result, ::testing::ElementsAre(0, 0.5F, 1));
}

// --- fract ------------------------------------------------------------------

TEST(ShaderBuiltins, fract_scalar)
{
  EXPECT_FLOAT_EQ(sw::fract(1.25F), 0.25F);
  EXPECT_FLOAT_EQ(sw::fract(-0.25F), 0.75F); // -0.25 - floor(-0.25) = -0.25 - (-1) = 0.75
}

TEST(ShaderBuiltins, fract_vector)
{
  const auto result = sw::fract(sw::Vector3F{1.25F, 2.5F, -0.25F});
  EXPECT_THAT(result,
              ::testing::ElementsAre(::testing::FloatEq(0.25F), ::testing::FloatEq(0.5F), ::testing::FloatEq(0.75F)));
}

// --- reflect / refract ------------------------------------------------------

TEST(ShaderBuiltins, reflect)
{
  // Incident going down-right, reflecting off a floor with an up normal.
  const auto result = sw::reflect(sw::Vector3F{1.0F, -1.0F, 0.0F}, sw::Vector3F{0.0F, 1.0F, 0.0F});
  EXPECT_THAT(result, ::testing::ElementsAre(1, 1, 0));
}

TEST(ShaderBuiltins, refract_straight_through_when_eta_is_one)
{
  // With eta == 1 the ray is undeviated.
  const auto result = sw::refract(sw::Vector3F{0.0F, -1.0F, 0.0F}, sw::Vector3F{0.0F, 1.0F, 0.0F}, 1.0F);
  EXPECT_THAT(result, ::testing::ElementsAre(0, -1, 0));
}

TEST(ShaderBuiltins, refract_total_internal_reflection_returns_zero)
{
  const auto incident = rtw::math::normalize(sw::Vector3F{1.0F, -1.0F, 0.0F});
  const auto result = sw::refract(incident, sw::Vector3F{0.0F, 1.0F, 0.0F}, 2.0F);
  EXPECT_THAT(result, ::testing::ElementsAre(0, 0, 0));
}

// --- texture ----------------------------------------------------------------

TEST(ShaderBuiltins, texture_samples_through_sampler)
{
  std::array<std::uint32_t, 1> texels{0xFF'00'00'FFU}; // single red texel
  const sw::Texture texture{texels.data(), 1, 1};
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::NEAREST};

  EXPECT_THAT(sw::texture(sampler, sw::Vector2F{0.5F, 0.5F}), ::testing::ElementsAre(1, 0, 0, 1));
}
