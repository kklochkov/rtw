#pragma once

#include "stl/utility.h"

#include "sw_renderer/clipping.h"
#include "sw_renderer/color.h"
#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/clip_space.h"
#include "sw_renderer/programmable_pipeline/frame_buffer.h"
#include "sw_renderer/programmable_pipeline/pipeline_rasterisation.h"
#include "sw_renderer/programmable_pipeline/pipeline_state.h"
#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/programmable_pipeline/shader.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/types.h"

#include "math/vector.h"
#include "math/vector_operations.h"

#include <cstddef>
#include <cstdint>
#include <vector>

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

void write_color(ColorBuffer& color_buffer, std::size_t x, std::size_t y, const Vector4F& source,
                 const BlendState& blend, const ColorMask& mask);

} // namespace details

class Pipeline
{
public:
  void draw_arrays(const IShaderProgram& program, const RawVertexStream& vertices, const PipelineState& state,
                   FrameBuffer& framebuffer, RenderStats& stats);

  void draw_arrays_instanced(const IShaderProgram& program, const RawVertexStream& vertices,
                             const std::uint32_t instance_count, const PipelineState& state, FrameBuffer& framebuffer,
                             RenderStats& stats);

  void draw_elements(const IShaderProgram& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                     const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats);

  void draw_elements_instanced(const IShaderProgram& program, const RawVertexStream& vertices,
                               const IndexBuffer& indices, const std::uint32_t instance_count,
                               const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats);

  template <typename ShaderT>
  void draw_arrays(const ShaderT& program, const RawVertexStream& vertices, const PipelineState& state,
                   FrameBuffer& framebuffer, RenderStats& stats)
  {
    draw_arrays_impl(program, vertices, 1U, state, framebuffer, stats);
  }

  template <typename ShaderT>
  void draw_arrays_instanced(const ShaderT& program, const RawVertexStream& vertices,
                             const std::uint32_t instance_count, const PipelineState& state, FrameBuffer& framebuffer,
                             RenderStats& stats)
  {
    draw_arrays_impl(program, vertices, instance_count, state, framebuffer, stats);
  }

  template <typename ShaderT>
  void draw_elements(const ShaderT& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                     const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats)
  {
    draw_elements_impl(program, vertices, indices, 1U, state, framebuffer, stats);
  }

  template <typename ShaderT>
  void draw_elements_instanced(const ShaderT& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                               const std::uint32_t instance_count, const PipelineState& state, FrameBuffer& framebuffer,
                               RenderStats& stats)
  {
    draw_elements_impl(program, vertices, indices, instance_count, state, framebuffer, stats);
  }

private:
  template <typename ShaderT>
  void draw_arrays_impl(const ShaderT& program, const RawVertexStream& vertices, const std::uint32_t instance_count,
                        const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats)
  {
    for (std::uint32_t instance_id = 0U; instance_id < instance_count; ++instance_id)
    {
      transform_vertices(program, vertices, instance_id);
      for (std::size_t i = 0U, primitive = 0U; (i + 2U) < transformed_.size(); i += 3U, ++primitive)
      {
        process_triangle(program, stl::at_unsafe(transformed_, i), stl::at_unsafe(transformed_, i + 1U),
                         stl::at_unsafe(transformed_, i + 2U), static_cast<std::uint32_t>(primitive), state,
                         framebuffer, stats);
      }
    }
  }

  template <typename ShaderT>
  void draw_elements_impl(const ShaderT& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                          const std::uint32_t instance_count, const PipelineState& state, FrameBuffer& framebuffer,
                          RenderStats& stats)
  {
    for (std::uint32_t instance_id = 0U; instance_id < instance_count; ++instance_id)
    {
      transform_vertices(program, vertices, instance_id);
      for (std::size_t i = 0U, primitive = 0U; (i + 2U) < indices.size(); i += 3U, ++primitive)
      {
        process_triangle(program, stl::at_unsafe(transformed_, indices.at(i)),
                         stl::at_unsafe(transformed_, indices.at(i + 1U)),
                         stl::at_unsafe(transformed_, indices.at(i + 2U)), static_cast<std::uint32_t>(primitive), state,
                         framebuffer, stats);
      }
    }
  }

  template <typename ShaderT>
  void transform_vertices(const ShaderT& program, const RawVertexStream& vertices, std::uint32_t instance_id)
  {
    const auto count = vertices.size();
    transformed_.resize(count);
    for (std::size_t i = 0U; i < count; ++i)
    {
      const VertexContext context{static_cast<std::uint32_t>(i), instance_id};
      const auto output = program.vertex(vertices.at(i), context);
      stl::at_unsafe(transformed_, i) =
          ClipVertex<single_precision>{output.position, output.varyings, output.point_size};
    }
  }

  template <typename ShaderT>
  static void process_triangle(const ShaderT& program, const ClipVertex<single_precision>& v0,
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

    const math::BoundingBoxI bounds{
        0,
        0,
        static_cast<std::int32_t>(framebuffer.width()) - 1,
        static_cast<std::int32_t>(framebuffer.height()) - 1,
    };
    for (std::size_t i = 0U; i < triangles.triangle_count; ++i)
    {
      const auto& triangle = stl::at_unsafe(triangles.triangles, i);
      const auto& cv0 = stl::at_unsafe(triangle, 0U);
      const auto& cv1 = stl::at_unsafe(triangle, 1U);
      const auto& cv2 = stl::at_unsafe(triangle, 2U);

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

      const auto shade_fragment = [&](const Point2I& p,
                                      const RegisterFile<single_precision, MAX_VARYING_COUNT>& varyings,
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

        const FragmentContext context{
            Vector4F{static_cast<single_precision>(p.x()) + 0.5F, static_cast<single_precision>(p.y()) + 0.5F, window_z,
                     inv_w},
            primitive_id,
            front_facing,
        };
        const auto fragment = program.fragment(varyings, context);
        if (fragment.discard)
        {
          return;
        }

        const auto depth = fragment.depth.value_or(window_z);
        if (fragment.depth.has_value() && state.depth_test_enabled
            && !details::depth_test_passes(state.depth_func, depth, stored_z))
        {
          return;
        }
        if (state.depth_write_enabled)
        {
          depth_buffer.set_depth(x, y, depth);
        }

        details::write_color(framebuffer.color_buffer(), x, y, fragment.color, state.blend, state.color_mask);
      };

      switch (state.polygon_mode)
      {
      case PolygonMode::FILL:
        fill_triangle_bbox(w0, w1, w2, cv0.varyings, cv1.varyings, cv2.varyings, bounds, shade_fragment);
        break;
      case PolygonMode::LINE:
        draw_line_varyings(w0, w1, cv0.varyings, cv1.varyings, bounds, shade_fragment);
        draw_line_varyings(w1, w2, cv1.varyings, cv2.varyings, bounds, shade_fragment);
        draw_line_varyings(w2, w0, cv2.varyings, cv0.varyings, bounds, shade_fragment);
        break;
      case PolygonMode::POINT:
        draw_point_varyings(w0, cv0.varyings, bounds, shade_fragment, cv0.point_size);
        draw_point_varyings(w1, cv1.varyings, bounds, shade_fragment, cv1.point_size);
        draw_point_varyings(w2, cv2.varyings, bounds, shade_fragment, cv2.point_size);
        break;
      }

      ++stats.triangles_rendered;
    }
  }

  std::vector<ClipVertex<single_precision>> transformed_;
};

} // namespace rtw::sw_renderer
