#include "sw_renderer/programmable_pipeline/pipeline.h"

#include <algorithm>

namespace rtw::sw_renderer
{

namespace details
{

constexpr Vector4F clip_to_window(const Vector4F& clip_position, const Viewport& viewport,
                                  const DepthRange& depth_range) noexcept
{
  constexpr single_precision ONE{1};
  constexpr single_precision HALF{0.5F};

  const auto inv_w = ONE / clip_position.w();
  const auto ndc_x = clip_position.x() * inv_w;
  const auto ndc_y = clip_position.y() * inv_w;
  const auto ndc_z = clip_position.z() * inv_w;

  const auto window_x = static_cast<single_precision>(viewport.x)
                      + ((ndc_x + ONE) * HALF * static_cast<single_precision>(viewport.width - 1));
  const auto window_y = static_cast<single_precision>(viewport.y)
                      + ((ONE - ndc_y) * HALF * static_cast<single_precision>(viewport.height - 1));
  const auto window_z = depth_range.z_near + ((depth_range.z_far - depth_range.z_near) * ((HALF * ndc_z) + HALF));

  return Vector4F{window_x, window_y, window_z, inv_w};
}

constexpr bool depth_test_passes(const DepthFunc func, const single_precision incoming,
                                 const single_precision stored) noexcept
{
  switch (func)
  {
  case DepthFunc::NEVER:
    return false;
  case DepthFunc::LESS:
    return incoming < stored;
  case DepthFunc::EQUAL:
    return incoming == stored;
  case DepthFunc::LEQUAL:
    return incoming <= stored;
  case DepthFunc::GREATER:
    return incoming > stored;
  case DepthFunc::NOTEQUAL:
    return incoming != stored;
  case DepthFunc::GEQUAL:
    return incoming >= stored;
  case DepthFunc::ALWAYS:
    return true;
  }
  return false;
}

constexpr bool inside_scissor(const Scissor& scissor, const std::int32_t x, const std::int32_t y) noexcept
{
  return (x >= scissor.x) && (x < (scissor.x + scissor.width)) && (y >= scissor.y)
      && (y < (scissor.y + scissor.height));
}

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

constexpr void write_color(ColorBuffer& color_buffer, const std::size_t x, const std::size_t y, const Vector4F& source,
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

void Pipeline::transform_vertices(const IShaderProgram& program, const RawVertexStream& vertices)
{
  const auto count = vertices.size();
  transformed_.resize(count);
  for (std::size_t i = 0U; i < count; ++i)
  {
    const VertexContext context{static_cast<std::uint32_t>(i), 0U};
    const auto output = program.vertex(vertices[i], context);
    transformed_[i] = ClipVertex<single_precision>{output.position, output.varyings};
  }
}

void Pipeline::process_triangle(const IShaderProgram& program, const ClipVertex<single_precision>& v0,
                                const ClipVertex<single_precision>& v1, const ClipVertex<single_precision>& v2,
                                const std::uint32_t primitive_id, const PipelineState& state, FrameBuffer& framebuffer,
                                RenderStats& stats)
{
  ++stats.triangles_submitted;

  const auto polygon = clip(v0, v1, v2);
  const auto triangles = triangulate(polygon);
  if (triangles.triangle_count == 0U)
  {
    ++stats.triangles_clipped;
    return;
  }

  for (std::size_t i = 0U; i < triangles.triangle_count; ++i)
  {
    const auto& triangle = triangles.triangles[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto& cv0 = triangle[0U];
    const auto& cv1 = triangle[1U];
    const auto& cv2 = triangle[2U];

    const auto w0 = details::clip_to_window(cv0.position, state.viewport, state.depth_range);
    const auto w1 = details::clip_to_window(cv1.position, state.viewport, state.depth_range);
    const auto w2 = details::clip_to_window(cv2.position, state.viewport, state.depth_range);

    const auto edge1 = (w1.xy() - w0.xy()).cast<double_precision>();
    const auto edge2 = (w2.xy() - w0.xy()).cast<double_precision>();
    const auto area = math::cross(edge1, edge2);
    if (area == double_precision{0})
    {
      ++stats.triangles_culled;
      continue;
    }

    const bool ccw = area > double_precision{0};
    const bool front_facing = (state.front_face == FrontFace::COUNTER_CLOCKWISE) ? ccw : !ccw;

    if ((state.cull_mode == CullMode::FRONT_AND_BACK) || ((state.cull_mode == CullMode::FRONT) && front_facing)
        || ((state.cull_mode == CullMode::BACK) && !front_facing))
    {
      ++stats.triangles_culled;
      continue;
    }

    fill_triangle_bbox(
        w0, w1, w2, cv0.varyings, cv1.varyings, cv2.varyings,
        [&](const Point2I& p, const RegisterFile<single_precision, MAX_VARYING_COUNT>& varyings,
            const single_precision window_z, const single_precision inv_w)
        {
          if (state.scissor.enabled && !details::inside_scissor(state.scissor, p.x(), p.y()))
          {
            return;
          }

          auto& depth_buffer = framebuffer.depth_buffer();
          const auto x = static_cast<std::size_t>(p.x());
          const auto y = static_cast<std::size_t>(p.y());
          const auto stored_z = depth_buffer.depth(x, y);
          if (state.depth_test_enabled && !details::depth_test_passes(state.depth_func, window_z, stored_z))
          {
            return;
          }

          const FragmentContext context{Vector4F{static_cast<single_precision>(p.x()) + 0.5F,
                                                 static_cast<single_precision>(p.y()) + 0.5F, window_z, inv_w},
                                        primitive_id, front_facing};
          const auto fragment = program.fragment(varyings, context);
          if (fragment.discard)
          {
            return;
          }

          const auto depth = fragment.depth.value_or(window_z);
          if (state.depth_test_enabled && !details::depth_test_passes(state.depth_func, depth, stored_z))
          {
            return;
          }
          if (state.depth_write_enabled)
          {
            depth_buffer.set_depth(x, y, depth);
          }

          details::write_color(framebuffer.color_buffer(), x, y, fragment.color, state.blend, state.color_mask);
        });

    ++stats.triangles_rendered;
  }
}

void Pipeline::draw_arrays(const IShaderProgram& program, const RawVertexStream& vertices, const PipelineState& state,
                           FrameBuffer& framebuffer, RenderStats& stats)
{
  transform_vertices(program, vertices);
  for (std::size_t i = 0U, primitive = 0U; (i + 2U) < transformed_.size(); i += 3U, ++primitive)
  {
    process_triangle(program, transformed_[i], transformed_[i + 1U], transformed_[i + 2U],
                     static_cast<std::uint32_t>(primitive), state, framebuffer, stats);
  }
}

void Pipeline::draw_elements(const IShaderProgram& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                             const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats)
{
  transform_vertices(program, vertices);
  for (std::size_t i = 0U, primitive = 0U; (i + 2U) < indices.size(); i += 3U, ++primitive)
  {
    process_triangle(program, transformed_[indices[i]], transformed_[indices[i + 1U]], transformed_[indices[i + 2U]],
                     static_cast<std::uint32_t>(primitive), state, framebuffer, stats);
  }
}

} // namespace rtw::sw_renderer
