#include "sw_renderer/color.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/sampler.h"
#include "sw_renderer/shader_builtins.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include "multiprecision/fixed_point.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

// Fixed-point coverage for the shader-support primitives (Color <-> Vector4 conversions,
// Sampler2D, and the GLSL-style built-ins). Built via cc_test_with_fixed_point (see BUILD), whose config
// transition compiles this target with RTW_USE_FIXED_POINT so that single_precision is FixedPoint16
// (Q15.16). It runs these header-only templates in fixed-point mode and checks their numeric results;
// the float test files (color_test/sampler_test/shader_builtins_test) carry the exhaustive assertions.
// Tolerances here sit comfortably above the FixedPoint16 resolution (~2^-16).
namespace
{
namespace sw = rtw::sw_renderer;

static_assert(rtw::multiprecision::IS_FIXED_POINT_V<sw::single_precision>,
              "This target must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

constexpr double TOLERANCE = 1.0e-3;

/// Construct a fixed-point scalar from a float literal.
sw::single_precision scalar(const float value) { return sw::single_precision{value}; }

/// Assert that @p actual matches @p expected component-wise within @p tolerance.
template <typename VectorT>
void expect_vector_near(const VectorT& actual, std::initializer_list<double> expected, const double tolerance)
{
  ASSERT_EQ(static_cast<std::size_t>(actual.size()), expected.size());
  std::uint32_t index = 0U;
  for (const double value : expected)
  {
    EXPECT_NEAR(static_cast<double>(actual[index]), value, tolerance) << "component " << index;
    ++index;
  }
}

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

// --- color <-> vector4 ------------------------------------------------------

TEST(ShaderSupportFixedPoint, color_casts_to_vector4)
{
  const sw::Color color{0xFF'80'00'FFU}; // R=255 G=128 B=0 A=255
  expect_vector_near(static_cast<sw::Vector4F>(color), {1.0, 128.0 / 255.0, 0.0, 1.0}, TOLERANCE);
}

TEST(ShaderSupportFixedPoint, color_constructs_from_vector4)
{
  const sw::Color color{sw::Vector4F{1.0F, 0.0F, 0.0F, 1.0F}};
  EXPECT_EQ(color, sw::Color{0xFF'00'00'FFU});
}

TEST(ShaderSupportFixedPoint, color_round_trip_truncates)
{
  // 0.5 -> 127.5 -> 127 (truncated), so the round trip is not the identity.
  const sw::Color color{sw::Vector4F{0.5F, 0.5F, 0.5F, 1.0F}};
  EXPECT_EQ(color, sw::Color{0x7F'7F'7F'FFU});
}

// --- sampler ----------------------------------------------------------------

TEST(ShaderSupportFixedPoint, sampler_nearest_samples_texels)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::NEAREST};

  expect_vector_near(sampler.sample(sw::Vector2F{0.25F, 0.25F}), {1.0, 0.0, 0.0, 1.0}, TOLERANCE); // red
  expect_vector_near(sampler.sample(sw::Vector2F{0.75F, 0.75F}), {1.0, 1.0, 1.0, 1.0}, TOLERANCE); // white
}

TEST(ShaderSupportFixedPoint, sampler_linear_blends_center)
{
  const auto texture = make_2x2_texture();
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::LINEAR};

  expect_vector_near(sampler.sample(sw::Vector2F{0.5F, 0.5F}), {0.5, 0.5, 0.5, 1.0}, TOLERANCE);
}

TEST(ShaderSupportFixedPoint, sampler_wrap_modes_map_out_of_range)
{
  const auto texture = make_2x2_texture();

  const sw::Sampler2D repeat{texture, sw::WrapMode::REPEAT, sw::FilterMode::NEAREST};
  expect_vector_near(repeat.sample(sw::Vector2F{1.25F, 0.25F}), {1.0, 0.0, 0.0, 1.0}, TOLERANCE); // -> red

  const sw::Sampler2D clamp{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::NEAREST};
  expect_vector_near(clamp.sample(sw::Vector2F{1.5F, -0.5F}), {0.0, 1.0, 0.0, 1.0}, TOLERANCE); // -> green

  const sw::Sampler2D mirror{texture, sw::WrapMode::MIRRORED_REPEAT, sw::FilterMode::NEAREST};
  expect_vector_near(mirror.sample(sw::Vector2F{1.25F, 0.25F}), {0.0, 1.0, 0.0, 1.0}, TOLERANCE); // -> green
}

// --- built-ins (scalar) -----------------------------------------------------

TEST(ShaderSupportFixedPoint, builtins_scalar)
{
  EXPECT_NEAR(static_cast<double>(sw::mix(scalar(0.0F), scalar(10.0F), scalar(0.5F))), 5.0, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(sw::saturate(scalar(2.0F))), 1.0, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(sw::saturate(scalar(-1.0F))), 0.0, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(sw::step(scalar(0.5F), scalar(0.75F))), 1.0, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(sw::smoothstep(scalar(0.0F), scalar(1.0F), scalar(0.5F))), 0.5, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(sw::fract(scalar(1.25F))), 0.25, TOLERANCE);
}

// --- built-ins (vector) -----------------------------------------------------

TEST(ShaderSupportFixedPoint, builtins_vector)
{
  expect_vector_near(sw::mix(sw::Vector3F{0.0F, 0.0F, 0.0F}, sw::Vector3F{2.0F, 4.0F, 6.0F}, scalar(0.5F)),
                     {1.0, 2.0, 3.0}, TOLERANCE);
  expect_vector_near(sw::saturate(sw::Vector3F{-1.0F, 0.5F, 2.0F}), {0.0, 0.5, 1.0}, TOLERANCE);
  expect_vector_near(sw::step(sw::Vector3F{0.5F, 0.5F, 0.5F}, sw::Vector3F{0.25F, 0.5F, 0.75F}), {0.0, 1.0, 1.0},
                     TOLERANCE);
  expect_vector_near(sw::smoothstep(scalar(0.0F), scalar(1.0F), sw::Vector3F{-1.0F, 0.5F, 2.0F}), {0.0, 0.5, 1.0},
                     TOLERANCE);
  expect_vector_near(sw::fract(sw::Vector3F{1.25F, 2.5F, -0.25F}), {0.25, 0.5, 0.75}, TOLERANCE);
}

// --- built-ins (reflect / refract) ------------------------------------------

TEST(ShaderSupportFixedPoint, builtins_reflect_refract)
{
  expect_vector_near(sw::reflect(sw::Vector3F{1.0F, -1.0F, 0.0F}, sw::Vector3F{0.0F, 1.0F, 0.0F}), {1.0, 1.0, 0.0},
                     TOLERANCE);
  // eta == 1 leaves the ray undeviated.
  expect_vector_near(sw::refract(sw::Vector3F{0.0F, -1.0F, 0.0F}, sw::Vector3F{0.0F, 1.0F, 0.0F}, scalar(1.0F)),
                     {0.0, -1.0, 0.0}, TOLERANCE);
  // Total internal reflection returns the zero vector.
  expect_vector_near(sw::refract(sw::Vector3F{1.0F, 0.0F, 0.0F}, sw::Vector3F{0.0F, 1.0F, 0.0F}, scalar(2.0F)),
                     {0.0, 0.0, 0.0}, TOLERANCE);
}

// --- built-ins (texture) ----------------------------------------------------

TEST(ShaderSupportFixedPoint, builtins_texture)
{
  std::array<std::uint32_t, 1> texels{0xFF'00'00'FFU}; // single red texel
  const sw::Texture texture{texels.data(), 1, 1};
  const sw::Sampler2D sampler{texture, sw::WrapMode::CLAMP_TO_EDGE, sw::FilterMode::NEAREST};

  expect_vector_near(sw::texture(sampler, sw::Vector2F{0.5F, 0.5F}), {1.0, 0.0, 0.0, 1.0}, TOLERANCE);
}
