#pragma once

#include "sw_renderer/types.h"

#include <cstdint>

namespace rtw::sw_renderer
{

/// Face-culling mode.
enum class CullMode : std::uint8_t
{
  NONE = 0U,      ///< Culling disabled.
  FRONT,          ///< Cull front-facing triangles.
  BACK,           ///< Cull back-facing triangles.
  FRONT_AND_BACK, ///< Cull every triangle (no fill).
};

// Winding order that defines a front-facing triangle.
enum class FrontFace : std::uint8_t
{
  COUNTER_CLOCKWISE = 0U, ///< CCW winding is front-facing.
  CLOCKWISE,              ///< CW winding is front-facing.
};

/// Depth-comparison function. The fragment passes when `compare(window_z, stored_z)` is true.
enum class DepthFunc : std::uint8_t
{
  NEVER = 0U, ///< Never passes.
  LESS,       ///< Passes if incoming < stored (GL default).
  EQUAL,      ///< Passes if incoming == stored.
  LEQUAL,     ///< Passes if incoming <= stored.
  GREATER,    ///< Passes if incoming > stored.
  NOTEQUAL,   ///< Passes if incoming != stored.
  GEQUAL,     ///< Passes if incoming >= stored.
  ALWAYS,     ///< Always passes.
};

/// Source/destination scaling factor for blending.
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

/// Blend equation combining the scaled source and destination.
enum class BlendEquation : std::uint8_t
{
  ADD = 0U,         ///< src * srcFactor + dst * dstFactor.
  SUBTRACT,         ///< src * srcFactor - dst * dstFactor.
  REVERSE_SUBTRACT, ///< dst * dstFactor - src * srcFactor.
  MIN,              ///< component-wise min(src, dst) (factors ignored).
  MAX,              ///< component-wise max(src, dst) (factors ignored).
};

/// Window-space rectangle that NDC x/y map into.
/// The y-flip and `(dim - 1) / 2` scale are applied by the pipeline's viewport stage.
struct Viewport
{
  std::int32_t x{0};      ///< Lower-left x in pixels.
  std::int32_t y{0};      ///< Lower-left y in pixels.
  std::int32_t width{0};  ///< Width in pixels.
  std::int32_t height{0}; ///< Height in pixels.
};

/// Maps NDC z in [-1, 1] to window depth:
/// `window_z = z_near + (z_far - z_near) * (0.5 * z_ndc + 0.5)`.
/// Standard depth is {0, 1}; reversed-Z is {1, 0}.
struct DepthRange
{
  single_precision z_near{0}; ///< Window depth mapped from z_ndc = -1.
  single_precision z_far{1};  ///< Window depth mapped from z_ndc = +1.
};

/// Scissor rectangle. When enabled, fragments outside the rectangle are rejected before the depth test.
struct Scissor
{
  std::int32_t x{0};      ///< Lower-left x in pixels.
  std::int32_t y{0};      ///< Lower-left y in pixels.
  std::int32_t width{0};  ///< Width in pixels.
  std::int32_t height{0}; ///< Height in pixels.
  bool enabled{false};    ///< Disabled by default (full framebuffer).
};

/// Per-channel framebuffer write enable.
/// Default: all channels written.
struct ColorMask
{
  bool red{true};
  bool green{true};
  bool blue{true};
  bool alpha{true};
};

/// Full single-attachment blend configuration with separate RGB / alpha factors and equations.
/// Disabled by default. `constant_color` backs the CONSTANT_* factors.
struct BlendState
{
  BlendFactor src_rgb{BlendFactor::ONE};      ///< Source RGB factor.
  BlendFactor dst_rgb{BlendFactor::ZERO};     ///< Destination RGB factor.
  BlendEquation eq_rgb{BlendEquation::ADD};   ///< RGB blend equation.
  BlendFactor src_alpha{BlendFactor::ONE};    ///< Source alpha factor.
  BlendFactor dst_alpha{BlendFactor::ZERO};   ///< Destination alpha factor.
  BlendEquation eq_alpha{BlendEquation::ADD}; ///< Alpha blend equation.
  Vector4F constant_color;                    ///< Constant RGBA for CONSTANT_* factors; zero by default.
  bool enabled{false};                        ///< Blending disabled by default (opaque writes).
};

struct PipelineState
{
  Viewport viewport{};                                ///< NDC -> window mapping.
  DepthRange depth_range{};                           ///< Window-depth mapping.
  CullMode cull_mode{CullMode::NONE};                 ///< Culling off by default.
  FrontFace front_face{FrontFace::COUNTER_CLOCKWISE}; ///< CCW front-facing.

  BlendState blend{};     ///< Blend configuration.
  ColorMask color_mask{}; ///< Per-channel write enable.
  Scissor scissor{};      ///< Scissor rectangle.

  DepthFunc depth_func{DepthFunc::LESS}; ///< Depth comparison.
  single_precision depth_clear_value{1}; ///< Depth clear value (1 standard, 0 reversed-Z).
  bool depth_test_enabled{true};         ///< Enable depth testing.
  bool depth_write_enabled{true};        ///< Enable depth writes.
};

} // namespace rtw::sw_renderer
