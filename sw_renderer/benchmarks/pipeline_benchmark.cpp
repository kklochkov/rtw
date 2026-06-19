#include "sw_renderer/color.h"
#include "sw_renderer/fixed_pipeline/renderer.h"
#include "sw_renderer/programmable_pipeline/builtin_shaders.h"
#include "sw_renderer/programmable_pipeline/frame_buffer.h"
#include "sw_renderer/programmable_pipeline/pipeline.h"
#include "sw_renderer/programmable_pipeline/pipeline_state.h"
#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/programmable_pipeline/vertex_layout.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex.h"

#include "stl/span.h"

#include <benchmark/benchmark.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{

using rtw::sw_renderer::single_precision;

constexpr std::size_t WIDTH{256U};
constexpr std::size_t HEIGHT{256U};

struct BenchVertex
{
  std::array<float, 4> position;
  std::array<float, 3> normal;
  std::array<float, 2> uv;
};

rtw::sw_renderer::VertexLayout make_layout()
{
  using rtw::sw_renderer::ComponentType;
  using rtw::sw_renderer::VertexAttribute;
  namespace loc = rtw::sw_renderer::attribute_location;
  return rtw::sw_renderer::VertexLayout{
      {VertexAttribute{loc::POSITION, offsetof(BenchVertex, position), ComponentType::FLOAT32, 4U},
       VertexAttribute{loc::NORMAL, offsetof(BenchVertex, normal), ComponentType::FLOAT32, 3U},
       VertexAttribute{loc::UV, offsetof(BenchVertex, uv), ComponentType::FLOAT32, 2U}},
      sizeof(BenchVertex)};
}

std::vector<BenchVertex> full_screen_triangle()
{
  return {BenchVertex{{-1.0F, -1.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}},
          BenchVertex{{3.0F, -1.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {2.0F, 0.0F}},
          BenchVertex{{-1.0F, 3.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 2.0F}}};
}

std::vector<std::uint32_t> make_checker(const std::size_t size)
{
  std::vector<std::uint32_t> texels(size * size);
  for (std::size_t y = 0U; y < size; ++y)
  {
    for (std::size_t x = 0U; x < size; ++x)
    {
      const bool even = ((x ^ y) & 0x4U) == 0U;
      texels[(y * size) + x] = even ? 0xFF'30'A0'E0U : 0xFF'E0'70'20U;
    }
  }
  return texels;
}

rtw::sw_renderer::PipelineState make_state()
{
  rtw::sw_renderer::PipelineState state;
  state.viewport =
      rtw::sw_renderer::Viewport{0, 0, static_cast<std::int32_t>(WIDTH), static_cast<std::int32_t>(HEIGHT)};
  state.depth_test_enabled = false;
  return state;
}

template <typename ShaderT>
void run(benchmark::State& state, const ShaderT& shader, const bool virtual_path)
{
  const auto vertices = full_screen_triangle();
  const rtw::sw_renderer::RawVertexStream stream{make_layout(), rtw::stl::as_bytes(rtw::stl::make_span(vertices))};
  const auto pipeline_state = make_state();
  rtw::sw_renderer::FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(rtw::sw_renderer::Color{}, single_precision{1});
  rtw::sw_renderer::Pipeline pipeline;
  rtw::sw_renderer::RenderStats stats;

  const rtw::sw_renderer::IShaderProgram& base = shader;
  for (auto _ : state)
  {
    if (virtual_path)
    {
      pipeline.draw_arrays(base, stream, pipeline_state, framebuffer, stats);
    }
    else
    {
      pipeline.draw_arrays(shader, stream, pipeline_state, framebuffer, stats);
    }
    benchmark::DoNotOptimize(framebuffer.color_buffer().data());
    benchmark::ClobberMemory();
  }
}

rtw::sw_renderer::LitShader make_lit_shader()
{
  rtw::sw_renderer::LitShader shader;
  shader.set_color(rtw::sw_renderer::Vector4F{1.0F, 0.5F, 0.2F, 1.0F});
  shader.set_light_direction(rtw::sw_renderer::Vector3F{0.0F, 0.0F, -1.0F});
  return shader;
}

void bm_pipeline_lit_virtual(benchmark::State& state) { run(state, make_lit_shader(), true); }
void bm_pipeline_lit_templated(benchmark::State& state) { run(state, make_lit_shader(), false); }

void bm_pipeline_textured_virtual(benchmark::State& state)
{
  const auto texels = make_checker(64U);
  rtw::sw_renderer::Texture texture{const_cast<std::uint32_t*>(texels.data()), 64U, 64U};
  rtw::sw_renderer::TexturedShader shader;
  shader.set_sampler(
      rtw::sw_renderer::Sampler2D{texture, rtw::sw_renderer::WrapMode::REPEAT, rtw::sw_renderer::FilterMode::LINEAR});
  run(state, shader, true);
}

void bm_pipeline_textured_templated(benchmark::State& state)
{
  const auto texels = make_checker(64U);
  rtw::sw_renderer::Texture texture{const_cast<std::uint32_t*>(texels.data()), 64U, 64U};
  rtw::sw_renderer::TexturedShader shader;
  shader.set_sampler(
      rtw::sw_renderer::Sampler2D{texture, rtw::sw_renderer::WrapMode::REPEAT, rtw::sw_renderer::FilterMode::LINEAR});
  run(state, shader, false);
}

rtw::sw_renderer::FlatColorShader make_flat_shader()
{
  rtw::sw_renderer::FlatColorShader shader;
  shader.set_color(rtw::sw_renderer::Vector4F{1.0F, 0.5F, 0.2F, 1.0F});
  return shader;
}

void bm_pipeline_flat_templated(benchmark::State& state) { run(state, make_flat_shader(), false); }

void bm_pipeline_textured_nearest_templated(benchmark::State& state)
{
  const auto texels = make_checker(64U);
  rtw::sw_renderer::Texture texture{const_cast<std::uint32_t*>(texels.data()), 64U, 64U};
  rtw::sw_renderer::TexturedShader shader;
  shader.set_sampler(
      rtw::sw_renderer::Sampler2D{texture, rtw::sw_renderer::WrapMode::REPEAT, rtw::sw_renderer::FilterMode::NEAREST});
  run(state, shader, false);
}

void bm_pipeline_standard_textured_nearest(benchmark::State& state)
{
  const auto texels = make_checker(64U);
  rtw::sw_renderer::Texture texture{const_cast<std::uint32_t*>(texels.data()), 64U, 64U};
  rtw::sw_renderer::StandardShader shader;
  shader.set_use_texture(true);
  shader.set_sampler(
      rtw::sw_renderer::Sampler2D{texture, rtw::sw_renderer::WrapMode::REPEAT, rtw::sw_renderer::FilterMode::NEAREST});
  run(state, shader, false);
}

void bm_pipeline_standard_textured_lit(benchmark::State& state)
{
  const auto texels = make_checker(64U);
  rtw::sw_renderer::Texture texture{const_cast<std::uint32_t*>(texels.data()), 64U, 64U};
  rtw::sw_renderer::StandardShader shader;
  shader.set_use_texture(true);
  shader.set_sampler(
      rtw::sw_renderer::Sampler2D{texture, rtw::sw_renderer::WrapMode::REPEAT, rtw::sw_renderer::FilterMode::NEAREST});
  shader.set_use_lighting(true);
  shader.set_light_direction(rtw::sw_renderer::Vector3F{0.0F, 0.0F, -1.0F});
  run(state, shader, false);
}

std::array<rtw::sw_renderer::VertexF, 4U> fullscreen_quad()
{
  constexpr float MAX_X = static_cast<float>(WIDTH) - 1.0F;
  constexpr float MAX_Y = static_cast<float>(HEIGHT) - 1.0F;
  using rtw::sw_renderer::Point4F;
  using rtw::sw_renderer::TexCoordF;
  using rtw::sw_renderer::VertexF;
  return {
      VertexF{Point4F{0.0F, 0.0F, 1.0F, 1.0F}, TexCoordF{0.0F, 0.0F}},
      VertexF{Point4F{MAX_X, 0.0F, 1.0F, 1.0F}, TexCoordF{1.0F, 0.0F}},
      VertexF{Point4F{MAX_X, MAX_Y, 1.0F, 1.0F}, TexCoordF{1.0F, 1.0F}},
      VertexF{Point4F{0.0F, MAX_Y, 1.0F, 1.0F}, TexCoordF{0.0F, 1.0F}},
  };
}

void bm_fixed_clear_only(benchmark::State& state)
{
  rtw::sw_renderer::Renderer renderer{WIDTH, HEIGHT};
  for (auto _ : state)
  {
    renderer.clear(rtw::sw_renderer::Color{});
    benchmark::DoNotOptimize(renderer.data());
    benchmark::ClobberMemory();
  }
}

void bm_fixed_flat(benchmark::State& state)
{
  rtw::sw_renderer::Renderer renderer{WIDTH, HEIGHT};
  const auto quad = fullscreen_quad();
  constexpr auto COLOR = rtw::sw_renderer::Color{0xFF'80'33'FFU};
  for (auto _ : state)
  {
    renderer.clear(rtw::sw_renderer::Color{});
    renderer.fill_triangle_bbox(quad[0U], quad[1U], quad[2U], COLOR, 1.0F);
    renderer.fill_triangle_bbox(quad[0U], quad[2U], quad[3U], COLOR, 1.0F);
    benchmark::DoNotOptimize(renderer.data());
    benchmark::ClobberMemory();
  }
}

void bm_fixed_textured_nearest(benchmark::State& state)
{
  const auto texels = make_checker(64U);
  rtw::sw_renderer::Texture texture{const_cast<std::uint32_t*>(texels.data()), 64U, 64U};
  rtw::sw_renderer::Renderer renderer{WIDTH, HEIGHT};
  const auto quad = fullscreen_quad();
  for (auto _ : state)
  {
    renderer.clear(rtw::sw_renderer::Color{});
    renderer.fill_triangle_bbox(quad[0U], quad[1U], quad[2U], texture, 1.0F);
    renderer.fill_triangle_bbox(quad[0U], quad[2U], quad[3U], texture, 1.0F);
    benchmark::DoNotOptimize(renderer.data());
    benchmark::ClobberMemory();
  }
}

} // namespace

BENCHMARK(bm_pipeline_lit_virtual);
BENCHMARK(bm_pipeline_lit_templated);
BENCHMARK(bm_pipeline_textured_virtual);
BENCHMARK(bm_pipeline_textured_templated);
BENCHMARK(bm_pipeline_flat_templated);
BENCHMARK(bm_pipeline_textured_nearest_templated);
BENCHMARK(bm_pipeline_standard_textured_nearest);
BENCHMARK(bm_pipeline_standard_textured_lit);

BENCHMARK(bm_fixed_clear_only);
BENCHMARK(bm_fixed_flat);
BENCHMARK(bm_fixed_textured_nearest);

BENCHMARK_MAIN();
