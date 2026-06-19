#include "sw_renderer/programmable_pipeline/builtin_shaders.h"

#include "sw_renderer/color.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/frame_buffer.h"
#include "sw_renderer/programmable_pipeline/pipeline.h"
#include "sw_renderer/programmable_pipeline/pipeline_state.h"
#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/programmable_pipeline/shader.h"
#include "sw_renderer/programmable_pipeline/vertex_layout.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include "multiprecision/fixed_point.h"
#include "stl/span.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>

// Fixed-point coverage for built-in shaders. Built via cc_test_with_fixed_point (see BUILD),
// whose config transition recompiles the dependency graph with RTW_USE_FIXED_POINT so single_precision
// is FixedPoint16 (Q15.16). It runs the header-only shaders in fixed-point mode -- a compile smoke test
// plus numeric checks of each stage and a pipeline fill; the float test file
// (builtin_shaders_test.cpp) carries the exhaustive assertions. Tolerances sit comfortably above the
// FixedPoint16 resolution (~2^-16).
namespace rtw::sw_renderer
{
namespace
{

static_assert(multiprecision::IS_FIXED_POINT_V<single_precision>,
              "This test must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

constexpr double TOLERANCE = 1.0e-3;
constexpr std::size_t WIDTH{8U};
constexpr std::size_t HEIGHT{8U};

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

Vertex make_vertex(const Vector4F& position, const Vector3F& normal, const Vector2F& uv, const Vector4F& color)
{
  return Vertex{{static_cast<float>(position.x()), static_cast<float>(position.y()), static_cast<float>(position.z()),
                 static_cast<float>(position.w())},
                {static_cast<float>(normal.x()), static_cast<float>(normal.y()), static_cast<float>(normal.z()), 0.0F},
                {static_cast<float>(uv.x()), static_cast<float>(uv.y()), 0.0F, 0.0F},
                {static_cast<float>(color.x()), static_cast<float>(color.y()), static_cast<float>(color.z()),
                 static_cast<float>(color.w())}};
}

std::vector<Vertex> full_screen_triangle()
{
  const Vector3F normal{0.0F, 0.0F, 1.0F};
  const Vector2F uv{0.0F, 0.0F};
  const Vector4F white{1.0F, 1.0F, 1.0F, 1.0F};
  return {make_vertex(Vector4F{-1.0F, -1.0F, 0.0F, 1.0F}, normal, uv, white),
          make_vertex(Vector4F{3.0F, -1.0F, 0.0F, 1.0F}, normal, uv, white),
          make_vertex(Vector4F{-1.0F, 3.0F, 0.0F, 1.0F}, normal, uv, white)};
}

PipelineState make_state()
{
  PipelineState state;
  state.viewport = Viewport{0, 0, static_cast<std::int32_t>(WIDTH), static_cast<std::int32_t>(HEIGHT)};
  return state;
}

template <typename VectorT>
void expect_vector_near(const VectorT& actual, std::initializer_list<double> expected)
{
  ASSERT_EQ(static_cast<std::size_t>(actual.size()), expected.size());
  std::uint16_t index = 0U;
  for (const double value : expected)
  {
    EXPECT_NEAR(static_cast<double>(actual[index]), value, TOLERANCE) << "component " << index;
    ++index;
  }
}

// --- Stage smoke / numeric checks -------------------------------------------

TEST(BuiltinShadersFixedPoint, flat_color_fragment_emits_the_uniform_colour)
{
  FlatColorShader shader;
  shader.set_color(Vector4F{0.25F, 0.5F, 0.75F, 1.0F});

  const IShaderProgram::DynamicVaryings varyings;
  expect_vector_near(shader.fragment(varyings, FragmentContext{}).color, {0.25, 0.5, 0.75, 1.0});
}

TEST(BuiltinShadersFixedPoint, vertex_color_shader_forwards_the_colour)
{
  const std::vector<Vertex> vertices{make_vertex(Vector4F{0.0F, 0.0F, 0.0F, 1.0F}, Vector3F{0.0F, 0.0F, 1.0F},
                                                 Vector2F{0.0F, 0.0F}, Vector4F{0.2F, 0.4F, 0.6F, 1.0F})};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  const VertexColorShader shader;
  expect_vector_near(shader.vertex(stream[0], VertexContext{}).varyings[0U], {0.2, 0.4, 0.6, 1.0});
}

TEST(BuiltinShadersFixedPoint, textured_shader_samples_the_bound_texture)
{
  std::array<std::uint32_t, 1U> texels{0xFF'00'00'FFU}; // single red texel
  const Texture texture{texels.data(), 1U, 1U};

  TexturedShader shader;
  shader.set_sampler(Sampler2D{texture, WrapMode::CLAMP_TO_EDGE, FilterMode::NEAREST});

  IShaderProgram::DynamicVaryings varyings;
  varyings[0U] = Vector4F{0.5F, 0.5F, 0.0F, 0.0F};
  expect_vector_near(shader.fragment(varyings, FragmentContext{}).color, {1.0, 0.0, 0.0, 1.0});
}

TEST(BuiltinShadersFixedPoint, lit_shader_is_fully_lit_when_facing_the_light)
{
  const std::vector<Vertex> vertices{make_vertex(Vector4F{0.0F, 0.0F, 0.0F, 1.0F}, Vector3F{0.0F, 0.0F, 1.0F},
                                                 Vector2F{0.0F, 0.0F}, Vector4F{1.0F, 1.0F, 1.0F, 1.0F})};
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};

  const LitShader shader; // default light direction (0, 0, -1)
  // Exercises the fixed-point normalize + dot now living in the vertex stage; the normal faces the light so the
  // per-vertex intensity saturates at 1 in the varying's x channel.
  expect_vector_near(shader.vertex(stream[0], VertexContext{}).varyings[LitShader::INTENSITY_VARYING],
                     {1.0, 0.0, 0.0, 0.0});
}

// --- Pipeline fill ----------------------------------------------------------

TEST(BuiltinShadersFixedPoint, flat_color_shader_fills_a_triangle)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, single_precision{1});

  FlatColorShader shader;
  shader.set_color(Vector4F{1.0F, 0.0F, 0.0F, 1.0F});
  const auto vertices = full_screen_triangle();
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{0xFF'00'00'FFU});
  EXPECT_EQ(framebuffer.color_buffer().pixel(1U, 1U), Color{0xFF'00'00'FFU});
}

TEST(BuiltinShadersFixedPoint, standard_shader_composes_texture_vertex_colour_and_lighting)
{
  std::array<std::uint32_t, 1U> texels{0xFF'00'00'FFU}; // single red texel
  const Texture texture{texels.data(), 1U, 1U};

  StandardShader shader;
  shader.set_base_color(Vector4F{1.0F, 1.0F, 1.0F, 1.0F});
  shader.set_sampler(Sampler2D{texture, WrapMode::CLAMP_TO_EDGE, FilterMode::NEAREST});
  shader.set_use_texture(true);
  shader.set_use_vertex_color(true);
  shader.set_use_lighting(true);

  IShaderProgram::DynamicVaryings varyings;
  varyings[StandardShader::UV_VARYING] = Vector4F{0.5F, 0.5F, 0.0F, 0.0F};
  varyings[StandardShader::COLOR_VARYING] = Vector4F{0.5F, 0.5F, 0.5F, 1.0F};
  varyings[StandardShader::LIGHT_VARYING] = Vector4F{1.0F, 0.0F, 0.0F, 0.0F}; // full per-vertex intensity

  // White base * red texture * grey vertex colour, intensity 1 -> (0.5, 0, 0, 1).
  expect_vector_near(shader.fragment(varyings, FragmentContext{}).color, {0.5, 0.0, 0.0, 1.0});
}

TEST(BuiltinShadersFixedPoint, standard_shader_fills_a_triangle_with_the_base_colour)
{
  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, single_precision{1});

  StandardShader shader; // all terms off -> plain base colour
  shader.set_base_color(Vector4F{1.0F, 0.0F, 0.0F, 1.0F});
  const auto vertices = full_screen_triangle();
  const RawVertexStream stream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(shader, stream, make_state(), framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{0xFF'00'00'FFU});
}

} // namespace
} // namespace rtw::sw_renderer
