#include "sw_renderer/programmable_pipeline/pipeline.h"

#include <algorithm>

namespace rtw::sw_renderer
{

namespace details
{

constexpr Vector4F resolve_blend_factor(const BlendFactor factor, const Vector4F& source, const Vector4F& dest,
                                        const Vector4F& constant) noexcept
{
  constexpr single_precision ZERO{0};
  constexpr single_precision ONE{1};
  switch (factor)
  {
  case BlendFactor::ZERO:
    return Vector4F{ZERO, ZERO, ZERO, ZERO};
  case BlendFactor::ONE:
    return Vector4F{ONE, ONE, ONE, ONE};
  case BlendFactor::SRC_COLOR:
    return source;
  case BlendFactor::ONE_MINUS_SRC_COLOR:
    return Vector4F{ONE - source.x(), ONE - source.y(), ONE - source.z(), ONE - source.w()};
  case BlendFactor::DST_COLOR:
    return dest;
  case BlendFactor::ONE_MINUS_DST_COLOR:
    return Vector4F{ONE - dest.x(), ONE - dest.y(), ONE - dest.z(), ONE - dest.w()};
  case BlendFactor::SRC_ALPHA:
    return Vector4F{source.w(), source.w(), source.w(), source.w()};
  case BlendFactor::ONE_MINUS_SRC_ALPHA:
  {
    const auto factor_value = ONE - source.w();
    return Vector4F{factor_value, factor_value, factor_value, factor_value};
  }
  case BlendFactor::DST_ALPHA:
    return Vector4F{dest.w(), dest.w(), dest.w(), dest.w()};
  case BlendFactor::ONE_MINUS_DST_ALPHA:
  {
    const auto factor_value = ONE - dest.w();
    return Vector4F{factor_value, factor_value, factor_value, factor_value};
  }
  case BlendFactor::CONSTANT_COLOR:
    return constant;
  case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
    return Vector4F{ONE - constant.x(), ONE - constant.y(), ONE - constant.z(), ONE - constant.w()};
  case BlendFactor::CONSTANT_ALPHA:
    return Vector4F{constant.w(), constant.w(), constant.w(), constant.w()};
  case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
  {
    const auto factor_value = ONE - constant.w();
    return Vector4F{factor_value, factor_value, factor_value, factor_value};
  }
  case BlendFactor::SRC_ALPHA_SATURATE:
  {
    const auto factor_value = std::min(source.w(), ONE - dest.w());
    return Vector4F{factor_value, factor_value, factor_value, ONE};
  }
  }
  return Vector4F{ONE, ONE, ONE, ONE};
}

constexpr single_precision combine_blend(const BlendEquation equation, const single_precision source,
                                         const single_precision dest, const single_precision source_factor,
                                         const single_precision dest_factor) noexcept
{
  switch (equation)
  {
  case BlendEquation::ADD:
    return (source * source_factor) + (dest * dest_factor);
  case BlendEquation::SUBTRACT:
    return (source * source_factor) - (dest * dest_factor);
  case BlendEquation::REVERSE_SUBTRACT:
    return (dest * dest_factor) - (source * source_factor);
  case BlendEquation::MIN:
    return std::min(source, dest);
  case BlendEquation::MAX:
    return std::max(source, dest);
  }
  return (source * source_factor) + (dest * dest_factor);
}

constexpr Vector4F blend_fragment(const BlendState& blend, const Vector4F& source, const Vector4F& dest) noexcept
{
  const auto src_rgb = resolve_blend_factor(blend.src_rgb, source, dest, blend.constant_color);
  const auto dst_rgb = resolve_blend_factor(blend.dst_rgb, source, dest, blend.constant_color);
  const auto src_alpha = resolve_blend_factor(blend.src_alpha, source, dest, blend.constant_color);
  const auto dst_alpha = resolve_blend_factor(blend.dst_alpha, source, dest, blend.constant_color);

  const auto r = combine_blend(blend.eq_rgb, source.x(), dest.x(), src_rgb.x(), dst_rgb.x());
  const auto g = combine_blend(blend.eq_rgb, source.y(), dest.y(), src_rgb.y(), dst_rgb.y());
  const auto b = combine_blend(blend.eq_rgb, source.z(), dest.z(), src_rgb.z(), dst_rgb.z());
  const auto a = combine_blend(blend.eq_alpha, source.w(), dest.w(), src_alpha.w(), dst_alpha.w());

  return Vector4F{r, g, b, a};
}

void write_color(ColorBuffer& color_buffer, const std::size_t x, const std::size_t y, const Vector4F& source,
                 const BlendState& blend, const ColorMask& mask)
{
  Vector4F color = source;
  if (blend.enabled)
  {
    const auto dest = static_cast<Vector4F>(color_buffer.pixel(x, y));
    color = blend_fragment(blend, source, dest);
  }

  const Color src{color};
  if (mask.red && mask.green && mask.blue && mask.alpha)
  {
    color_buffer.set_pixel(x, y, src);
    return;
  }

  Color result = color_buffer.pixel(x, y);
  if (mask.red)
  {
    result.set_r(src.r());
  }
  if (mask.green)
  {
    result.set_g(src.g());
  }
  if (mask.blue)
  {
    result.set_b(src.b());
  }
  if (mask.alpha)
  {
    result.set_a(src.a());
  }
  color_buffer.set_pixel(x, y, result);
}

} // namespace details

void Pipeline::draw_arrays(const IShaderProgram& program, const RawVertexStream& vertices, const PipelineState& state,
                           FrameBuffer& framebuffer, RenderStats& stats)
{
  draw_arrays_impl(program, vertices, state, framebuffer, stats);
}

void Pipeline::draw_elements(const IShaderProgram& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                             const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats)
{
  draw_elements_impl(program, vertices, indices, state, framebuffer, stats);
}

} // namespace rtw::sw_renderer
