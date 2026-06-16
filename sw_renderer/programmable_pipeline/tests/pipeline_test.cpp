#include "sw_renderer/programmable_pipeline/pipeline.h"

#include "sw_renderer/color.h"
#include "sw_renderer/programmable_pipeline/frame_buffer.h"
#include "sw_renderer/programmable_pipeline/pipeline_state.h"
#include "sw_renderer/programmable_pipeline/shader.h"
#include "sw_renderer/programmable_pipeline/vertex_layout.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/types.h"

#include "stl/span.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{
namespace
{

// --- Fixtures and helpers ---------------------------------------------------

constexpr std::size_t WIDTH{8U};
constexpr std::size_t HEIGHT{8U};

constexpr std::uint32_t POSITION_LOCATION{0U};
constexpr std::uint32_t COLOR_LOCATION{1U};

const Vector4F RED{1.0F, 0.0F, 0.0F, 1.0F};
const Vector4F GREEN{0.0F, 1.0F, 0.0F, 1.0F};
const Vector4F BLUE{0.0F, 0.0F, 1.0F, 1.0F};
const Vector4F WHITE{1.0F, 1.0F, 1.0F, 1.0F};

/// A plain-float test vertex: position (xyzw) followed by an RGBA colour, tightly packed so the
/// `VertexLayout` below can address both attributes via byte offsets.
struct Vertex
{
  std::array<float, 4> position;
  std::array<float, 4> color;
};

static_assert(sizeof(Vertex) == 32U, "Vertex must be tightly packed for the byte-offset layout");

VertexLayout make_layout()
{
  return VertexLayout{{VertexAttribute{POSITION_LOCATION, 0U, ComponentType::FLOAT32, 4U},
                       VertexAttribute{COLOR_LOCATION, 16U, ComponentType::FLOAT32, 4U}},
                      sizeof(Vertex)};
}

/// Builds a non-owning raw stream over `vertices`; the caller must keep `vertices` alive.
RawVertexStream make_stream(const std::vector<Vertex>& vertices)
{
  return RawVertexStream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
}

Vertex make_vertex(const float x, const float y, const float z, const Vector4F& color)
{
  return Vertex{{x, y, z, 1.0F}, {color.x(), color.y(), color.z(), color.w()}};
}

/// A counter-clockwise (front-facing) triangle that, after frustum clipping, covers the whole NDC
/// square. With the `(dim - 1)` viewport scale it fills every pixel whose centre lies in `[0, dim - 1]`,
/// i.e. rows / columns `0 .. dim - 2`.
std::vector<Vertex> full_screen_triangle(const Vector4F& color, const float z = 0.0F)
{
  return {make_vertex(-1.0F, -1.0F, z, color), make_vertex(3.0F, -1.0F, z, color), make_vertex(-1.0F, 3.0F, z, color)};
}

/// The full-screen triangle with a distinct colour per vertex (for interpolation tests).
std::vector<Vertex> full_screen_triangle_rgb()
{
  return {make_vertex(-1.0F, -1.0F, 0.0F, RED), make_vertex(3.0F, -1.0F, 0.0F, GREEN),
          make_vertex(-1.0F, 3.0F, 0.0F, BLUE)};
}

/// A counter-clockwise (front-facing) triangle fully inside the frustum: clipping leaves it as a single
/// triangle, so the render statistics are exact.
std::vector<Vertex> inside_triangle(const float z = 0.0F)
{
  return {make_vertex(-0.5F, -0.5F, z, RED), make_vertex(0.5F, -0.5F, z, RED), make_vertex(0.0F, 0.5F, z, RED)};
}

/// A triangle entirely behind the near plane (NDC z < -1): the clipper rejects it completely.
std::vector<Vertex> behind_near_triangle()
{
  return {make_vertex(-0.5F, -0.5F, -2.0F, RED), make_vertex(0.0F, 0.5F, -2.0F, RED),
          make_vertex(0.5F, -0.5F, -2.0F, RED)};
}

PipelineState make_state()
{
  PipelineState state;
  state.viewport = Viewport{0, 0, static_cast<std::int32_t>(WIDTH), static_cast<std::int32_t>(HEIGHT)};
  return state;
}

/// A shader that writes a fixed colour, ignoring its inputs.
class ConstantColorProgram : public IShaderProgram
{
public:
  explicit ConstantColorProgram(const Vector4F& color) : color_{color} {}

  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(POSITION_LOCATION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& /*varyings*/, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = color_;
    return out;
  }

private:
  Vector4F color_;
};

/// A shader that forwards the per-vertex colour through a varying slot (perspective-correct interpolation).
class VaryingColorProgram : public IShaderProgram
{
public:
  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(POSITION_LOCATION);
    out.varyings[0U] = input.attribute(COLOR_LOCATION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& varyings, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = varyings[0U];
    return out;
  }
};

/// A shader that discards every fragment.
class DiscardProgram : public IShaderProgram
{
public:
  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(POSITION_LOCATION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& /*varyings*/, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = WHITE;
    out.discard = true;
    return out;
  }
};

// --- Tests ------------------------------------------------------------------

TEST(Pipeline, draw_arrays_fills_color_and_depth)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const ConstantColorProgram program{RED};
  const auto vertices = full_screen_triangle(RED);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{RED});
  EXPECT_EQ(framebuffer.color_buffer().pixel(1U, 1U), Color{RED});
  EXPECT_EQ(framebuffer.color_buffer().pixel(6U, 6U), Color{RED});
  EXPECT_FLOAT_EQ(framebuffer.depth_buffer().depth(4U, 4U), 0.5F);
  EXPECT_EQ(stats.triangles_submitted, 1U);
  EXPECT_GE(stats.triangles_rendered, 1U);
  EXPECT_EQ(stats.triangles_clipped, 0U);
}

TEST(Pipeline, varying_is_interpolated_across_the_triangle)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const VaryingColorProgram program;
  const auto vertices = full_screen_triangle_rgb();
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, make_state(), framebuffer, stats);

  // An interior pixel blends all three vertex colours, so no channel is fully saturated or fully off.
  const auto center = framebuffer.color_buffer().pixel(4U, 4U);
  EXPECT_GT(center.r(), 0U);
  EXPECT_LT(center.r(), 255U);
  EXPECT_GT(center.g(), 0U);
  EXPECT_LT(center.g(), 255U);
  EXPECT_GT(center.b(), 0U);
  EXPECT_LT(center.b(), 255U);
}

TEST(Pipeline, nearer_fragment_wins_depth_test)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  Pipeline pipeline;
  RenderStats stats;
  const auto state = make_state();

  // Far (window_z 0.75) then near (window_z 0.25): the near fragment passes LESS and overwrites.
  const ConstantColorProgram far_program{RED};
  const auto far_vertices = full_screen_triangle(RED, 0.5F);
  const auto far_stream = make_stream(far_vertices);
  pipeline.draw_arrays(far_program, far_stream, state, framebuffer, stats);

  const ConstantColorProgram near_program{GREEN};
  const auto near_vertices = full_screen_triangle(GREEN, -0.5F);
  const auto near_stream = make_stream(near_vertices);
  pipeline.draw_arrays(near_program, near_stream, state, framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{GREEN});
  EXPECT_FLOAT_EQ(framebuffer.depth_buffer().depth(4U, 4U), 0.25F);
}

TEST(Pipeline, farther_fragment_fails_depth_test)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  Pipeline pipeline;
  RenderStats stats;
  const auto state = make_state();

  // Near first, then far: the far fragment fails LESS and is rejected.
  const ConstantColorProgram near_program{GREEN};
  const auto near_vertices = full_screen_triangle(GREEN, -0.5F);
  const auto near_stream = make_stream(near_vertices);
  pipeline.draw_arrays(near_program, near_stream, state, framebuffer, stats);

  const ConstantColorProgram far_program{RED};
  const auto far_vertices = full_screen_triangle(RED, 0.5F);
  const auto far_stream = make_stream(far_vertices);
  pipeline.draw_arrays(far_program, far_stream, state, framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{GREEN});
  EXPECT_FLOAT_EQ(framebuffer.depth_buffer().depth(4U, 4U), 0.25F);
}

TEST(Pipeline, depth_write_can_be_disabled)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  auto state = make_state();
  state.depth_write_enabled = false;

  const ConstantColorProgram program{RED};
  const auto vertices = full_screen_triangle(RED);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  // The colour is written but the depth buffer keeps its cleared value.
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{RED});
  EXPECT_FLOAT_EQ(framebuffer.depth_buffer().depth(4U, 4U), 1.0F);
}

TEST(Pipeline, scissor_rejects_fragments_outside_the_rectangle)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  auto state = make_state();
  state.scissor = Scissor{0, 0, 4, 4, true};

  const ConstantColorProgram program{RED};
  const auto vertices = full_screen_triangle(RED);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(2U, 2U), Color{RED}); // inside the scissor box
  EXPECT_EQ(framebuffer.color_buffer().pixel(5U, 5U), Color{});    // covered, but scissored out
}

TEST(Pipeline, color_mask_disables_channel_writes)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  auto state = make_state();
  state.color_mask = ColorMask{false, true, true, true}; // red disabled

  const ConstantColorProgram program{WHITE};
  const auto vertices = full_screen_triangle(WHITE);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  // Red keeps the cleared value (0); green/blue/alpha are written from the white fragment.
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U),
            (Color{std::uint8_t{0}, std::uint8_t{255}, std::uint8_t{255}, std::uint8_t{255}}));
}

TEST(Pipeline, blending_mixes_translucent_source_over_destination)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{GREEN}, 1.0F); // opaque green destination

  // Classic "source-over" alpha blend: src * srcAlpha + dst * (1 - srcAlpha).
  auto state = make_state();
  state.blend.enabled = true;
  state.blend.src_rgb = BlendFactor::SRC_ALPHA;
  state.blend.dst_rgb = BlendFactor::ONE_MINUS_SRC_ALPHA;

  // A half-transparent red fragment laid over the green background.
  const ConstantColorProgram program{Vector4F{1.0F, 0.0F, 0.0F, 0.5F}};
  const auto vertices = full_screen_triangle(RED);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  // rgb = red * 0.5 + green * 0.5 = (0.5, 0.5, 0); a = 0.5 * 1 + 1 * 0 = 0.5; each 0.5 -> 127.
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U),
            (Color{std::uint8_t{127}, std::uint8_t{127}, std::uint8_t{0}, std::uint8_t{127}}));
}

TEST(Pipeline, additive_blending_sums_source_and_destination)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{GREEN}, 1.0F); // opaque green destination

  // Additive blend (src * 1 + dst * 1) for both RGB and alpha.
  auto state = make_state();
  state.blend.enabled = true;
  state.blend.src_rgb = BlendFactor::ONE;
  state.blend.dst_rgb = BlendFactor::ONE;
  state.blend.src_alpha = BlendFactor::ONE;
  state.blend.dst_alpha = BlendFactor::ONE;

  // Magenta added to green yields white; alpha 1 + 1 = 2 saturates to 255.
  const ConstantColorProgram program{Vector4F{1.0F, 0.0F, 1.0F, 1.0F}};
  const auto vertices = full_screen_triangle(RED);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{WHITE});
}

TEST(Pipeline, discarded_fragments_leave_the_framebuffer_untouched)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const DiscardProgram program;
  const auto vertices = full_screen_triangle(WHITE);
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{});    // colour untouched
  EXPECT_FLOAT_EQ(framebuffer.depth_buffer().depth(4U, 4U), 1.0F); // depth untouched
  EXPECT_GE(stats.triangles_rendered, 1U);                         // but the triangle was rasterised
}

TEST(Pipeline, inside_triangle_produces_exact_stats)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const ConstantColorProgram program{RED};
  const auto vertices = inside_triangle();
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(stats.triangles_submitted, 1U);
  EXPECT_EQ(stats.triangles_clipped, 0U);
  EXPECT_EQ(stats.triangles_culled, 0U);
  EXPECT_EQ(stats.triangles_rendered, 1U);
}

TEST(Pipeline, triangle_behind_near_plane_is_clipped_away)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const ConstantColorProgram program{RED};
  const auto vertices = behind_near_triangle();
  const auto stream = make_stream(vertices);
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(stats.triangles_submitted, 1U);
  EXPECT_EQ(stats.triangles_clipped, 1U);
  EXPECT_EQ(stats.triangles_rendered, 0U);
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{});
}

TEST(Pipeline, back_face_culling_removes_front_facing_triangle_only_for_matching_mode)
{
  const ConstantColorProgram program{RED};
  const auto vertices = inside_triangle(); // counter-clockwise -> front-facing
  const auto stream = make_stream(vertices);

  const auto render = [&](const CullMode cull_mode, const FrontFace front_face)
  {
    FrameBuffer framebuffer{WIDTH, HEIGHT};
    framebuffer.clear(Color{}, 1.0F);
    auto state = make_state();
    state.cull_mode = cull_mode;
    state.front_face = front_face;
    RenderStats stats;
    Pipeline pipeline;
    pipeline.draw_arrays(program, stream, state, framebuffer, stats);
    return stats;
  };

  // No culling and back-face culling both keep a front-facing (CCW) triangle.
  EXPECT_EQ(render(CullMode::NONE, FrontFace::COUNTER_CLOCKWISE).triangles_rendered, 1U);
  EXPECT_EQ(render(CullMode::BACK, FrontFace::COUNTER_CLOCKWISE).triangles_rendered, 1U);

  // Front-face culling (and cull-everything) removes it.
  EXPECT_EQ(render(CullMode::FRONT, FrontFace::COUNTER_CLOCKWISE).triangles_rendered, 0U);
  EXPECT_EQ(render(CullMode::FRONT, FrontFace::COUNTER_CLOCKWISE).triangles_culled, 1U);
  EXPECT_EQ(render(CullMode::FRONT_AND_BACK, FrontFace::COUNTER_CLOCKWISE).triangles_rendered, 0U);

  // Flipping the front-face winding makes the same triangle back-facing, so BACK now culls it.
  EXPECT_EQ(render(CullMode::BACK, FrontFace::CLOCKWISE).triangles_rendered, 0U);
  EXPECT_EQ(render(CullMode::FRONT, FrontFace::CLOCKWISE).triangles_rendered, 1U);
}

TEST(Pipeline, draw_elements_matches_draw_arrays)
{
  const ConstantColorProgram program{RED};
  const auto state = make_state();

  FrameBuffer arrays_fb{WIDTH, HEIGHT};
  arrays_fb.clear(Color{}, 1.0F);
  const auto vertices = full_screen_triangle(RED);
  const auto stream = make_stream(vertices);
  RenderStats arrays_stats;
  Pipeline pipeline;
  pipeline.draw_arrays(program, stream, state, arrays_fb, arrays_stats);

  FrameBuffer elements_fb{WIDTH, HEIGHT};
  elements_fb.clear(Color{}, 1.0F);
  const IndexBuffer indices{std::vector<std::uint32_t>{0U, 1U, 2U}};
  RenderStats elements_stats;
  pipeline.draw_elements(program, stream, indices, state, elements_fb, elements_stats);

  for (std::size_t y = 0U; y < HEIGHT; ++y)
  {
    for (std::size_t x = 0U; x < WIDTH; ++x)
    {
      EXPECT_EQ(arrays_fb.color_buffer().pixel(x, y), elements_fb.color_buffer().pixel(x, y));
      EXPECT_FLOAT_EQ(arrays_fb.depth_buffer().depth(x, y), elements_fb.depth_buffer().depth(x, y));
    }
  }
  EXPECT_EQ(arrays_stats.triangles_rendered, elements_stats.triangles_rendered);
}

} // namespace
} // namespace rtw::sw_renderer
