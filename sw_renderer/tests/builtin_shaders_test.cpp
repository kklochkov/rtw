#include "sw_renderer/builtin_shaders.h"

#include "sw_renderer/color.h"
#include "sw_renderer/frame_buffer.h"
#include "sw_renderer/pipeline.h"
#include "sw_renderer/pipeline_state.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/sampler.h"
#include "sw_renderer/shader.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex_layout.h"
#include "sw_renderer/vertex_stream.h"

#include "math/matrix.h"

#include "stl/span.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
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

const Vector4F RED{1.0F, 0.0F, 0.0F, 1.0F};
const Vector4F GREEN{0.0F, 1.0F, 0.0F, 1.0F};
const Vector4F BLUE{0.0F, 0.0F, 1.0F, 1.0F};
const Vector4F WHITE{1.0F, 1.0F, 1.0F, 1.0F};

/// A test vertex carrying every attribute the built-in shaders read, tightly packed so the
/// `VertexLayout` below can address each one via `offsetof`.
struct Vertex
{
  std::array<float, 4> position;
  std::array<float, 4> normal;
  std::array<float, 4> uv;
  std::array<float, 4> color;
};

VertexLayout make_layout()
{
  return VertexLayout{
      {VertexAttribute{attribute_location::POSITION, offsetof(Vertex, position), ComponentType::FLOAT32, 4U},
       VertexAttribute{attribute_location::NORMAL, offsetof(Vertex, normal), ComponentType::FLOAT32, 4U},
       VertexAttribute{attribute_location::UV, offsetof(Vertex, uv), ComponentType::FLOAT32, 4U},
       VertexAttribute{attribute_location::COLOR, offsetof(Vertex, color), ComponentType::FLOAT32, 4U}},
      sizeof(Vertex)};
}

constexpr Vector4F clip_position(const float x, const float y, const float z = 0.0F) { return Vector4F{x, y, z, 1.0F}; }

Vertex make_vertex(const Vector4F& position, const Vector3F& normal, const Vector2F& uv, const Vector4F& color)
{
  return Vertex{{position.x(), position.y(), position.z(), position.w()},
                {normal.x(), normal.y(), normal.z(), 0.0F},
                {uv.x(), uv.y(), 0.0F, 0.0F},
                {color.x(), color.y(), color.z(), color.w()}};
}

/// A counter-clockwise (front-facing) triangle that covers the whole NDC square; with the `(dim - 1)`
/// viewport scale it fills every pixel whose centre lies in `[0, dim - 1]`.
std::vector<Vertex> full_screen_triangle(const Vector4F& color, const Vector3F& normal = {0.0F, 0.0F, 1.0F},
                                         const Vector2F& uv = {0.0F, 0.0F})
{
  return {make_vertex(clip_position(-1.0F, -1.0F), normal, uv, color),
          make_vertex(clip_position(3.0F, -1.0F), normal, uv, color),
          make_vertex(clip_position(-1.0F, 3.0F), normal, uv, color)};
}

PipelineState make_state()
{
  PipelineState state;
  state.viewport = Viewport{0, 0, static_cast<std::int32_t>(WIDTH), static_cast<std::int32_t>(HEIGHT)};
  return state;
}

void expect_vec4_eq(const Vector4F& actual, const Vector4F& expected)
{
  EXPECT_FLOAT_EQ(actual.x(), expected.x());
  EXPECT_FLOAT_EQ(actual.y(), expected.y());
  EXPECT_FLOAT_EQ(actual.z(), expected.z());
  EXPECT_FLOAT_EQ(actual.w(), expected.w());
}

// --- FlatColorShader --------------------------------------------------------

TEST(FlatColorShader, vertex_passes_position_through_with_identity_mvp)
{
  const std::vector<Vertex> vertices{
      make_vertex(clip_position(2.0F, 3.0F, 4.0F), {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}, RED)};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  const FlatColorShader shader;
  const auto out = shader.vertex(stream[0], VertexContext{});

  expect_vec4_eq(out.position, clip_position(2.0F, 3.0F, 4.0F));
}

TEST(FlatColorShader, vertex_applies_the_mvp_matrix)
{
  const std::vector<Vertex> vertices{
      make_vertex(clip_position(2.0F, 3.0F, 4.0F), {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}, RED)};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  FlatColorShader shader;
  // A pure translation by (10, 20, 30).
  shader.set_mvp_matrix(Matrix4x4F{math::FROM_ROW_MAJOR, 1.0F, 0.0F, 0.0F, 10.0F, 0.0F, 1.0F, 0.0F, 20.0F, 0.0F, 0.0F,
                                   1.0F, 30.0F, 0.0F, 0.0F, 0.0F, 1.0F});

  const auto out = shader.vertex(stream[0], VertexContext{});

  expect_vec4_eq(out.position, clip_position(12.0F, 23.0F, 34.0F));
}

TEST(FlatColorShader, fragment_emits_the_uniform_colour)
{
  FlatColorShader shader;
  shader.set_color(BLUE);

  const IShaderProgram::DynamicVaryings varyings;
  const auto out = shader.fragment(varyings, FragmentContext{});

  expect_vec4_eq(out.color, BLUE);
  EXPECT_FALSE(out.discard);
}

TEST(FlatColorShader, fills_a_triangle_through_the_pipeline)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  FlatColorShader shader;
  shader.set_color(BLUE);
  const auto vertices = full_screen_triangle(BLUE);
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{BLUE});
  EXPECT_EQ(framebuffer.color_buffer().pixel(1U, 1U), Color{BLUE});
}

// --- VertexColorShader ------------------------------------------------------

TEST(VertexColorShader, vertex_forwards_the_colour_attribute_to_a_varying)
{
  const std::vector<Vertex> vertices{make_vertex(clip_position(0.0F, 0.0F), {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}, GREEN)};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  const VertexColorShader shader;
  const auto out = shader.vertex(stream[0], VertexContext{});

  expect_vec4_eq(out.varyings[0U], GREEN);
}

TEST(VertexColorShader, fragment_emits_the_interpolated_varying)
{
  IShaderProgram::DynamicVaryings varyings;
  varyings[0U] = GREEN;

  const VertexColorShader shader;
  const auto out = shader.fragment(varyings, FragmentContext{});

  expect_vec4_eq(out.color, GREEN);
}

TEST(VertexColorShader, interpolates_vertex_colours_across_the_triangle)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const VertexColorShader shader;
  std::vector<Vertex> vertices = full_screen_triangle(RED);
  vertices[1].color = {GREEN.x(), GREEN.y(), GREEN.z(), GREEN.w()};
  vertices[2].color = {BLUE.x(), BLUE.y(), BLUE.z(), BLUE.w()};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  // An interior pixel blends all three vertex colours, so no channel is saturated or fully off.
  const auto center = framebuffer.color_buffer().pixel(4U, 4U);
  EXPECT_GT(center.r(), 0U);
  EXPECT_LT(center.r(), 255U);
  EXPECT_GT(center.g(), 0U);
  EXPECT_LT(center.g(), 255U);
  EXPECT_GT(center.b(), 0U);
  EXPECT_LT(center.b(), 255U);
}

// --- TexturedShader ---------------------------------------------------------

TEST(TexturedShader, vertex_forwards_the_uv_attribute_to_a_varying)
{
  const std::vector<Vertex> vertices{make_vertex(clip_position(0.0F, 0.0F), {0.0F, 0.0F, 1.0F}, {0.25F, 0.75F}, RED)};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  const TexturedShader shader;
  const auto out = shader.vertex(stream[0], VertexContext{});

  EXPECT_FLOAT_EQ(out.varyings[0U].x(), 0.25F);
  EXPECT_FLOAT_EQ(out.varyings[0U].y(), 0.75F);
}

TEST(TexturedShader, fragment_samples_the_bound_texture_at_the_varying_uv)
{
  std::vector<std::uint32_t> texels{
      Color{std::uint8_t{10}, std::uint8_t{20}, std::uint8_t{30}, std::uint8_t{255}}.rgba,
      Color{std::uint8_t{40}, std::uint8_t{50}, std::uint8_t{60}, std::uint8_t{255}}.rgba,
      Color{std::uint8_t{70}, std::uint8_t{80}, std::uint8_t{90}, std::uint8_t{255}}.rgba,
      Color{std::uint8_t{100}, std::uint8_t{110}, std::uint8_t{120}, std::uint8_t{255}}.rgba};
  const Texture texture{texels.data(), 2U, 2U};
  const Sampler2D sampler{texture};

  TexturedShader shader;
  shader.set_sampler(sampler);

  IShaderProgram::DynamicVaryings varyings;
  varyings[0U] = Vector4F{0.75F, 0.25F, 0.0F, 0.0F};
  const auto out = shader.fragment(varyings, FragmentContext{});

  expect_vec4_eq(out.color, sampler.sample(Vector2F{0.75F, 0.25F}));
}

TEST(TexturedShader, fills_a_triangle_from_a_solid_texture)
{
  // A solid blue texture round-trips exactly (only 0 / 255 channels).
  std::vector<std::uint32_t> texels(4U, Color{BLUE}.rgba);
  const Texture texture{texels.data(), 2U, 2U};

  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  TexturedShader shader;
  shader.set_sampler(Sampler2D{texture});
  const auto vertices = full_screen_triangle(WHITE, {0.0F, 0.0F, 1.0F}, {0.5F, 0.5F});
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{BLUE});
}

// --- LitShader --------------------------------------------------------------

TEST(LitShader, vertex_transforms_the_normal_as_a_direction)
{
  const std::vector<Vertex> vertices{make_vertex(clip_position(0.0F, 0.0F), {1.0F, 1.0F, 1.0F}, {0.0F, 0.0F}, WHITE)};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  LitShader shader;
  // Scale by (2, 3, 4) with a translation that must be ignored for a direction (w = 0).
  shader.set_normal_matrix(Matrix4x4F{math::FROM_ROW_MAJOR, 2.0F, 0.0F, 0.0F, 5.0F, 0.0F, 3.0F, 0.0F, 6.0F, 0.0F, 0.0F,
                                      4.0F, 7.0F, 0.0F, 0.0F, 0.0F, 1.0F});

  const auto out = shader.vertex(stream[0], VertexContext{});

  expect_vec4_eq(out.varyings[0U], Vector4F{2.0F, 3.0F, 4.0F, 0.0F});
}

TEST(LitShader, fragment_is_fully_lit_when_the_normal_faces_the_light)
{
  LitShader shader; // default light direction (0, 0, -1)
  shader.set_color(Vector4F{0.6F, 0.8F, 1.0F, 1.0F});

  IShaderProgram::DynamicVaryings varyings;
  varyings[0U] = Vector4F{0.0F, 0.0F, 1.0F, 0.0F}; // normal points at the viewer / light source

  const auto out = shader.fragment(varyings, FragmentContext{});

  expect_vec4_eq(out.color, Vector4F{0.6F, 0.8F, 1.0F, 1.0F});
}

TEST(LitShader, fragment_is_dark_when_the_normal_faces_away_from_the_light)
{
  LitShader shader;
  shader.set_color(WHITE);

  IShaderProgram::DynamicVaryings varyings;
  varyings[0U] = Vector4F{0.0F, 0.0F, -1.0F, 0.0F}; // normal points away from the light

  const auto out = shader.fragment(varyings, FragmentContext{});

  expect_vec4_eq(out.color, Vector4F{0.0F, 0.0F, 0.0F, 1.0F});
}

TEST(LitShader, fragment_scales_colour_by_the_lambert_term)
{
  LitShader shader;
  shader.set_color(WHITE);

  IShaderProgram::DynamicVaryings varyings;
  varyings[0U] = Vector4F{1.0F, 0.0F, 1.0F, 0.0F}; // 45 degrees: intensity = 1 / sqrt(2)

  const auto out = shader.fragment(varyings, FragmentContext{});

  const auto expected = 1.0F / std::sqrt(2.0F);
  EXPECT_FLOAT_EQ(out.color.x(), expected);
  EXPECT_FLOAT_EQ(out.color.y(), expected);
  EXPECT_FLOAT_EQ(out.color.z(), expected);
  EXPECT_FLOAT_EQ(out.color.w(), 1.0F);
}

TEST(LitShader, shades_a_triangle_through_the_pipeline)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  LitShader shader;
  shader.set_color(WHITE); // fully lit white -> white
  const auto vertices = full_screen_triangle(WHITE, {0.0F, 0.0F, 1.0F});
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{WHITE});
}

// --- Simple built-ins (gl_FragCoord / gl_FrontFacing / gl_FragDepth / gl_VertexID) ------------------

/// Emits a colour that depends on whether the fragment is front-facing.
class FrontFacingProbe : public IShaderProgram
{
public:
  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(attribute_location::POSITION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& /*input*/, const FragmentContext& context) const override
  {
    FragmentShaderOutput out;
    out.color = context.front_facing ? GREEN : RED;
    return out;
  }
};

TEST(Builtins, front_facing_reflects_the_triangle_winding)
{
  const FrontFacingProbe shader;
  const auto vertices = full_screen_triangle(WHITE); // counter-clockwise

  const auto render = [&](const FrontFace front_face)
  {
    FrameBuffer framebuffer{WIDTH, HEIGHT};
    framebuffer.clear(Color{}, 1.0F);
    auto state = make_state();
    state.front_face = front_face;
    state.cull_mode = CullMode::NONE; // keep the triangle regardless of facing
    const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
    RenderStats stats;
    Pipeline pipeline;
    pipeline.draw_arrays(shader, stream, state, framebuffer, stats);
    return framebuffer.color_buffer().pixel(4U, 4U);
  };

  EXPECT_EQ(render(FrontFace::COUNTER_CLOCKWISE), Color{GREEN}); // front-facing
  EXPECT_EQ(render(FrontFace::CLOCKWISE), Color{RED});           // same winding now reads as back-facing
}

/// Keeps only the fragment whose window centre lies in pixel column 4 (`gl_FragCoord.x` in (4, 5)).
class FragCoordColumnProbe : public IShaderProgram
{
public:
  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(attribute_location::POSITION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& /*input*/, const FragmentContext& context) const override
  {
    FragmentShaderOutput out;
    out.color = RED;
    const auto x = context.frag_coord.x();
    out.discard = !((x > 4.0F) && (x < 5.0F));
    return out;
  }
};

TEST(Builtins, frag_coord_carries_the_pixel_centre)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const FragCoordColumnProbe shader;
  const auto vertices = full_screen_triangle(RED);
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  // Only column 4 survives the discard; neighbouring columns keep the cleared background.
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 2U), Color{RED});
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 5U), Color{RED});
  EXPECT_EQ(framebuffer.color_buffer().pixel(3U, 2U), Color{});
  EXPECT_EQ(framebuffer.color_buffer().pixel(5U, 2U), Color{});
}

/// Overrides the fragment depth (`gl_FragDepth`) with a constant.
class FragDepthProbe : public IShaderProgram
{
public:
  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(attribute_location::POSITION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& /*input*/, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = RED;
    out.depth = 0.25F;
    return out;
  }
};

TEST(Builtins, frag_depth_overrides_the_interpolated_window_depth)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, 1.0F);

  const FragDepthProbe shader;
  const auto vertices = full_screen_triangle(RED); // geometry at NDC z = 0 -> window z = 0.5
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  // The shader-written depth (0.25) replaces the interpolated 0.5.
  EXPECT_FLOAT_EQ(framebuffer.depth_buffer().depth(4U, 4U), 0.25F);
}

/// Writes the vertex ID into a varying so the test can read it back from the vertex stage.
class VertexIdProbe : public IShaderProgram
{
public:
  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& context) const override
  {
    VertexShaderOutput out;
    out.position = input.attribute(attribute_location::POSITION);
    out.varyings[0U] = Vector4F{static_cast<single_precision>(context.vertex_id), 0.0F, 0.0F, 0.0F};
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& input, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = input[0U];
    return out;
  }
};

TEST(Builtins, vertex_id_is_supplied_to_the_vertex_stage)
{
  const std::vector<Vertex> vertices{make_vertex(clip_position(0.0F, 0.0F), {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}, WHITE)};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  const VertexIdProbe shader;
  VertexContext context;
  context.vertex_id = 7U;
  const auto out = shader.vertex(stream[0], context);

  EXPECT_FLOAT_EQ(out.varyings[0U].x(), 7.0F);
}

} // namespace
} // namespace rtw::sw_renderer
