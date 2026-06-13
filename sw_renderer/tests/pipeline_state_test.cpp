#include "sw_renderer/pipeline_state.h"
#include "sw_renderer/types.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>

namespace
{
namespace sw = rtw::sw_renderer;
} // namespace

TEST(Viewport, defaults)
{
  constexpr sw::Viewport VIEWPORT{};
  EXPECT_EQ(VIEWPORT.x, 0);
  EXPECT_EQ(VIEWPORT.y, 0);
  EXPECT_EQ(VIEWPORT.width, 0);
  EXPECT_EQ(VIEWPORT.height, 0);
}

TEST(DepthRange, defaults)
{
  constexpr sw::DepthRange RANGE{};
  EXPECT_EQ(RANGE.z_near, sw::single_precision{0});
  EXPECT_EQ(RANGE.z_far, sw::single_precision{1});
}

TEST(Scissor, defaults)
{
  constexpr sw::Scissor SCISSOR{};
  EXPECT_FALSE(SCISSOR.enabled);
  EXPECT_EQ(SCISSOR.x, 0);
  EXPECT_EQ(SCISSOR.y, 0);
  EXPECT_EQ(SCISSOR.width, 0);
  EXPECT_EQ(SCISSOR.height, 0);
}

TEST(ColorMask, defaults_all_enabled)
{
  constexpr sw::ColorMask MASK{};
  EXPECT_TRUE(MASK.red);
  EXPECT_TRUE(MASK.green);
  EXPECT_TRUE(MASK.blue);
  EXPECT_TRUE(MASK.alpha);
}

TEST(BlendState, defaults_match_disabled_opaque)
{
  constexpr sw::BlendState BLEND{};
  EXPECT_FALSE(BLEND.enabled);
  EXPECT_EQ(BLEND.src_rgb, sw::BlendFactor::ONE);
  EXPECT_EQ(BLEND.dst_rgb, sw::BlendFactor::ZERO);
  EXPECT_EQ(BLEND.eq_rgb, sw::BlendEquation::ADD);
  EXPECT_EQ(BLEND.src_alpha, sw::BlendFactor::ONE);
  EXPECT_EQ(BLEND.dst_alpha, sw::BlendFactor::ZERO);
  EXPECT_EQ(BLEND.eq_alpha, sw::BlendEquation::ADD);
  EXPECT_EQ(BLEND.constant_color, sw::Vector4F{});
}

TEST(PipelineState, defaults_follow_opengl)
{
  constexpr sw::PipelineState STATE{};

  // Viewport / depth-range.
  EXPECT_EQ(STATE.viewport.width, 0);
  EXPECT_EQ(STATE.viewport.height, 0);
  EXPECT_EQ(STATE.depth_range.z_near, sw::single_precision{0});
  EXPECT_EQ(STATE.depth_range.z_far, sw::single_precision{1});

  // Culling off, CCW front-facing (GL defaults).
  EXPECT_EQ(STATE.cull_mode, sw::CullMode::NONE);
  EXPECT_EQ(STATE.front_face, sw::FrontFace::COUNTER_CLOCKWISE);

  // Depth: tested + written, LESS, clear to far (1).
  EXPECT_TRUE(STATE.depth_test_enabled);
  EXPECT_TRUE(STATE.depth_write_enabled);
  EXPECT_EQ(STATE.depth_func, sw::DepthFunc::LESS);
  EXPECT_EQ(STATE.depth_clear_value, sw::single_precision{1});

  // Blend disabled, full color mask, scissor disabled.
  EXPECT_FALSE(STATE.blend.enabled);
  EXPECT_TRUE(STATE.color_mask.red);
  EXPECT_TRUE(STATE.color_mask.alpha);
  EXPECT_FALSE(STATE.scissor.enabled);
}

TEST(PipelineState, fields_are_mutable)
{
  sw::PipelineState state{};
  state.viewport = sw::Viewport{0, 0, 640, 480};
  state.cull_mode = sw::CullMode::BACK;
  state.front_face = sw::FrontFace::CLOCKWISE;
  state.depth_func = sw::DepthFunc::GEQUAL;
  state.depth_clear_value = sw::single_precision{0};
  state.blend.enabled = true;
  state.blend.src_rgb = sw::BlendFactor::SRC_ALPHA;
  state.blend.dst_rgb = sw::BlendFactor::ONE_MINUS_SRC_ALPHA;
  state.color_mask.alpha = false;
  state.scissor = sw::Scissor{10, 20, 100, 50, true};

  EXPECT_EQ(state.viewport.width, 640);
  EXPECT_EQ(state.cull_mode, sw::CullMode::BACK);
  EXPECT_EQ(state.front_face, sw::FrontFace::CLOCKWISE);
  EXPECT_EQ(state.depth_func, sw::DepthFunc::GEQUAL);
  EXPECT_EQ(state.depth_clear_value, sw::single_precision{0});
  EXPECT_TRUE(state.blend.enabled);
  EXPECT_EQ(state.blend.src_rgb, sw::BlendFactor::SRC_ALPHA);
  EXPECT_EQ(state.blend.dst_rgb, sw::BlendFactor::ONE_MINUS_SRC_ALPHA);
  EXPECT_FALSE(state.color_mask.alpha);
  EXPECT_TRUE(state.scissor.enabled);
  EXPECT_EQ(state.scissor.width, 100);
}

TEST(PipelineState, enums_use_uint8_underlying_type)
{
  static_assert(std::is_same_v<std::underlying_type_t<sw::CullMode>, std::uint8_t>);
  static_assert(std::is_same_v<std::underlying_type_t<sw::FrontFace>, std::uint8_t>);
  static_assert(std::is_same_v<std::underlying_type_t<sw::DepthFunc>, std::uint8_t>);
  static_assert(std::is_same_v<std::underlying_type_t<sw::BlendFactor>, std::uint8_t>);
  static_assert(std::is_same_v<std::underlying_type_t<sw::BlendEquation>, std::uint8_t>);

  // Reversed-Z as pure state: GREATER + clear 0.
  sw::PipelineState reversed_z{};
  reversed_z.depth_range = sw::DepthRange{1, 0};
  reversed_z.cull_mode = sw::CullMode::BACK;
  reversed_z.front_face = sw::FrontFace::COUNTER_CLOCKWISE;
  reversed_z.depth_func = sw::DepthFunc::GREATER;
  EXPECT_EQ(reversed_z.depth_range.z_near, sw::single_precision{1});
  EXPECT_EQ(reversed_z.depth_range.z_far, sw::single_precision{0});
  EXPECT_EQ(reversed_z.depth_func, sw::DepthFunc::GREATER);
}
