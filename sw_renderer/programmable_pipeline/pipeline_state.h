#pragma once

#include "sw_renderer/types.h"

#include <cstdint>

namespace rtw::sw_renderer
{

enum class CullMode : std::uint8_t
{
  NONE = 0U,
  FRONT,
  BACK,
  FRONT_AND_BACK,
};

enum class FrontFace : std::uint8_t
{
  COUNTER_CLOCKWISE = 0U,
  CLOCKWISE,
};

enum class DepthFunc : std::uint8_t
{
  NEVER = 0U,
  LESS,
  EQUAL,
  LEQUAL,
  GREATER,
  NOTEQUAL,
  GEQUAL,
  ALWAYS,
};

enum class BlendFactor : std::uint8_t
{
  ZERO = 0U,
  ONE,
  SRC_COLOR,
  ONE_MINUS_SRC_COLOR,
  DST_COLOR,
  ONE_MINUS_DST_COLOR,
  SRC_ALPHA,
  ONE_MINUS_SRC_ALPHA,
  DST_ALPHA,
  ONE_MINUS_DST_ALPHA,
  CONSTANT_COLOR,
  ONE_MINUS_CONSTANT_COLOR,
  CONSTANT_ALPHA,
  ONE_MINUS_CONSTANT_ALPHA,
  SRC_ALPHA_SATURATE,
};

enum class BlendEquation : std::uint8_t
{
  ADD = 0U,
  SUBTRACT,
  REVERSE_SUBTRACT,
  MIN,
  MAX,
};

struct Viewport
{
  std::int32_t x{0};
  std::int32_t y{0};
  std::int32_t width{0};
  std::int32_t height{0};
};

struct DepthRange
{
  single_precision z_near{0};
  single_precision z_far{1};
};

struct Scissor
{
  std::int32_t x{0};
  std::int32_t y{0};
  std::int32_t width{0};
  std::int32_t height{0};
  bool enabled{false};
};

struct ColorMask
{
  bool red{true};
  bool green{true};
  bool blue{true};
  bool alpha{true};
};

struct BlendState
{
  BlendFactor src_rgb{BlendFactor::ONE};
  BlendFactor dst_rgb{BlendFactor::ZERO};
  BlendEquation eq_rgb{BlendEquation::ADD};
  BlendFactor src_alpha{BlendFactor::ONE};
  BlendFactor dst_alpha{BlendFactor::ZERO};
  BlendEquation eq_alpha{BlendEquation::ADD};
  Vector4F constant_color;
  bool enabled{false};
};

struct PipelineState
{
  Viewport viewport{};
  DepthRange depth_range{};
  CullMode cull_mode{CullMode::NONE};
  FrontFace front_face{FrontFace::COUNTER_CLOCKWISE};
  BlendState blend{};
  ColorMask color_mask{};
  Scissor scissor{};
  DepthFunc depth_func{DepthFunc::LESS};
  single_precision depth_clear_value{1};
  bool depth_test_enabled{true};
  bool depth_write_enabled{true};
};

} // namespace rtw::sw_renderer
