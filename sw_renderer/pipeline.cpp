#include "sw_renderer/pipeline.h"

namespace rtw::sw_renderer
{

namespace details
{

/// Perspective divide + viewport / depth-range transform of a single clip-space position.
///
/// Carries over the renderer's window mapping (`projection.h` `make_screen_space_matrix`):
/// the y axis is flipped (NDC y-up -> window y-down, origin top-left) and the scale/offset use `(dim - 1) / 2`.
/// Depth maps NDC z in [-1, 1] to `[z_near, z_far]` via `DepthRange`. The returned `w` caches `1 / w_clip`,
/// which the rasteriser consumes for perspective-correct interpolation.
///
/// @param[in] clip_position The clip-space position (clipping guarantees `w > 0`, so the divide is safe).
/// @param[in] viewport The window rectangle NDC x/y map into.
/// @param[in] depth_range The window-depth mapping for NDC z.
/// @return The window-space position `{x_px, y_px, window_z, 1 / w_clip}`.
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

/// Evaluates the depth comparison `compare(incoming, stored)` selected by `func`.
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

/// Tests whether a window-space pixel lies inside the scissor rectangle.
constexpr bool inside_scissor(const Scissor& scissor, const std::int32_t x, const std::int32_t y) noexcept
{
  return (x >= scissor.x) && (x < (scissor.x + scissor.width)) && (y >= scissor.y)
      && (y < (scissor.y + scissor.height));
}

/// Writes the fragment colour, honouring the per-channel `ColorMask`.
constexpr void write_color(ColorBuffer& color_buffer, const std::size_t x, const std::size_t y, const Vector4F& source,
                           const ColorMask& mask)
{
  const Color src{source};
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

  // Clip against the six clip-space half-spaces, then fan-triangulate the resulting polygon.
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

    // Perspective divide + viewport / depth-range transform; w holds 1 / w_clip.
    const auto w0 = details::clip_to_window(cv0.position, state.viewport, state.depth_range);
    const auto w1 = details::clip_to_window(cv1.position, state.viewport, state.depth_range);
    const auto w2 = details::clip_to_window(cv2.position, state.viewport, state.depth_range);

    // Signed area (window space) decides winding; zero area is degenerate -> reject.
    const auto edge1 = (w1.xy() - w0.xy()).cast<double_precision>();
    const auto edge2 = (w2.xy() - w0.xy()).cast<double_precision>();
    const auto area = math::cross(edge1, edge2);
    if (area == double_precision{0})
    {
      ++stats.triangles_culled;
      continue;
    }

    const bool is_counter_clockwise = area > double_precision{0};
    const bool is_front_facing =
        (state.front_face == FrontFace::COUNTER_CLOCKWISE) ? is_counter_clockwise : !is_counter_clockwise;

    bool culled = false;
    switch (state.cull_mode)
    {
    case CullMode::NONE:
      break;
    case CullMode::FRONT:
      culled = is_front_facing;
      break;
    case CullMode::BACK:
      culled = !is_front_facing;
      break;
    case CullMode::FRONT_AND_BACK:
      culled = true;
      break;
    }

    if (culled)
    {
      ++stats.triangles_culled;
      continue;
    }

    ++stats.triangles_rendered;

    const auto width = static_cast<std::int32_t>(framebuffer.width());
    const auto height = static_cast<std::int32_t>(framebuffer.height());

    fill_triangle_bbox(w0, w1, w2, cv0.varyings, cv1.varyings, cv2.varyings,
                       [&program, &state, &framebuffer, primitive_id, is_front_facing, width,
                        height](const Point2I& point, const RegisterFile<single_precision, MAX_VARYING_COUNT>& varyings,
                                const single_precision window_z, const single_precision inv_w)
                       {
                         const auto x = point.x();
                         const auto y = point.y();

                         // The bounding box (and an offset viewport) can address pixels outside the framebuffer.
                         if ((x < 0) || (y < 0) || (x >= width) || (y >= height))
                         {
                           return;
                         }

                         // Scissor reject before the depth test (§10 step 7).
                         if (state.scissor.enabled && !details::inside_scissor(state.scissor, x, y))
                         {
                           return;
                         }

                         const auto px = static_cast<std::size_t>(x);
                         const auto py = static_cast<std::size_t>(y);

                         // Depth test against the affine window-z.
                         auto& depth_buffer = framebuffer.depth_buffer();
                         if (state.depth_test_enabled
                             && !details::depth_test_passes(state.depth_func, window_z, depth_buffer.depth(px, py)))
                         {
                           return;
                         }

                         // Fragment shader.
                         FragmentContext context{};
                         context.frag_coord =
                             Vector4F{static_cast<single_precision>(x) + single_precision{0.5F},
                                      static_cast<single_precision>(y) + single_precision{0.5F}, window_z, inv_w};
                         context.primitive_id = primitive_id;
                         context.front_facing = is_front_facing;
                         const auto fragment = program.fragment(varyings, context);
                         if (fragment.discard)
                         {
                           return;
                         }

                         // Depth write (optional gl_FragDepth override; the early test above still uses window_z).
                         if (state.depth_write_enabled)
                         {
                           depth_buffer.set_depth(px, py, fragment.depth.value_or(window_z));
                         }

                         // Colour write (opaque) honouring the colour mask.
                         details::write_color(framebuffer.color_buffer(), px, py, fragment.color, state.color_mask);
                       });
  }
}

void Pipeline::draw_arrays(const IShaderProgram& program, const RawVertexStream& vertices, const PipelineState& state,
                           FrameBuffer& framebuffer, RenderStats& stats)
{
  transform_vertices(program, vertices);

  const auto triangle_count = transformed_.size() / 3U;
  for (std::size_t triangle = 0U; triangle < triangle_count; ++triangle)
  {
    const auto base = triangle * 3U;
    process_triangle(program, transformed_[base], transformed_[base + 1U], transformed_[base + 2U],
                     static_cast<std::uint32_t>(triangle), state, framebuffer, stats);
  }
}

void Pipeline::draw_elements(const IShaderProgram& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                             const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats)
{
  transform_vertices(program, vertices);

  const auto triangle_count = indices.size() / 3U;
  for (std::size_t triangle = 0U; triangle < triangle_count; ++triangle)
  {
    const auto base = triangle * 3U;
    process_triangle(program, transformed_[indices[base]], transformed_[indices[base + 1U]],
                     transformed_[indices[base + 2U]], static_cast<std::uint32_t>(triangle), state, framebuffer, stats);
  }
}

} // namespace rtw::sw_renderer
