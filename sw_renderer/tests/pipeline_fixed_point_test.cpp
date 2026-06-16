#include "sw_renderer/pipeline.h"

#include "sw_renderer/color.h"
#include "sw_renderer/frame_buffer.h"
#include "sw_renderer/pipeline_state.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/shader.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex_layout.h"
#include "sw_renderer/vertex_stream.h"

#include "multiprecision/fixed_point.h"
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

// This target is built via cc_test_with_fixed_point (see BUILD), whose config transition recompiles the
// whole dependency graph -- including the //sw_renderer library (pipeline.cpp) -- with
// RTW_USE_FIXED_POINT, so single_precision is FixedPoint16 and double_precision is FixedPoint32. It
// exercises the pipeline core (clip-space clip, perspective divide / viewport transform with the
// FixedPoint32-widened winding area, depth test and colour write) end-to-end in fixed-point mode and
// checks the rasterised colour and depth output.
static_assert(multiprecision::IS_FIXED_POINT_V<single_precision>,
              "This test must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

constexpr std::size_t WIDTH{8U};
constexpr std::size_t HEIGHT{8U};
constexpr std::uint32_t POSITION_LOCATION{0U};

struct Vertex
{
  std::array<float, 4> position;
};

static_assert(sizeof(Vertex) == 16U, "Vertex must be tightly packed for the byte-offset layout");

const Vector4F RED{single_precision{1}, single_precision{0}, single_precision{0}, single_precision{1}};
const Vector4F GREEN{single_precision{0}, single_precision{1}, single_precision{0}, single_precision{1}};
const Vector4F WHITE{single_precision{1}, single_precision{1}, single_precision{1}, single_precision{1}};

VertexLayout make_layout()
{
  return VertexLayout{{VertexAttribute{POSITION_LOCATION, 0U, ComponentType::FLOAT32, 4U}}, sizeof(Vertex)};
}

/// Non-owning raw stream over `vertices`; the caller keeps `vertices` alive.
RawVertexStream make_stream(const std::vector<Vertex>& vertices)
{
  return RawVertexStream{make_layout(), stl::as_bytes(stl::make_span(vertices))};
}

/// A counter-clockwise full-screen triangle: after clipping it covers every pixel in the 8x8 target.
std::vector<Vertex> full_screen_triangle()
{
  return {Vertex{{-1.0F, -1.0F, 0.0F, 1.0F}}, Vertex{{3.0F, -1.0F, 0.0F, 1.0F}}, Vertex{{-1.0F, 3.0F, 0.0F, 1.0F}}};
}

PipelineState make_state()
{
  PipelineState state;
  state.viewport = Viewport{0, 0, static_cast<std::int32_t>(WIDTH), static_cast<std::int32_t>(HEIGHT)};
  return state;
}

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

TEST(PipelineFixedPoint, draw_arrays_fills_color_and_depth)
{
  const auto vertices = full_screen_triangle();
  const auto stream = make_stream(vertices);

  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, single_precision{1});

  const ConstantColorProgram program{RED};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, make_state(), framebuffer, stats);

  // The ConstantColorProgram does no interpolation, so every covered pixel quantises exactly to red.
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{0xFF'00'00'FFU});
  EXPECT_EQ(framebuffer.color_buffer().pixel(1U, 1U), Color{0xFF'00'00'FFU});
  EXPECT_EQ(framebuffer.color_buffer().pixel(6U, 6U), Color{0xFF'00'00'FFU});
  // Depth is barycentric-interpolated, so allow a small fixed-point slack around the expected 0.5.
  EXPECT_NEAR(static_cast<float>(framebuffer.depth_buffer().depth(4U, 4U)), 0.5F, 1.0e-3F);
  EXPECT_EQ(stats.triangles_submitted, 1U);
  EXPECT_GE(stats.triangles_rendered, 1U);
  EXPECT_EQ(stats.triangles_clipped, 0U);
}

TEST(PipelineFixedPoint, blending_mixes_translucent_source_over_destination)
{
  const auto vertices = full_screen_triangle();
  const auto stream = make_stream(vertices);

  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{GREEN}, single_precision{1}); // opaque green destination

  // Classic source-over alpha blend, with the FixedPoint16 multiplies running inside pipeline.cpp.
  auto state = make_state();
  state.blend.enabled = true;
  state.blend.src_rgb = BlendFactor::SRC_ALPHA;
  state.blend.dst_rgb = BlendFactor::ONE_MINUS_SRC_ALPHA;

  const ConstantColorProgram program{
      Vector4F{single_precision{1}, single_precision{0}, single_precision{0}, single_precision{0.5F}}};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  // rgb = red * 0.5 + green * 0.5 = (0.5, 0.5, 0); a = 0.5 * 1 + 1 * 0 = 0.5; each 0.5 -> 127.
  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U),
            (Color{std::uint8_t{127}, std::uint8_t{127}, std::uint8_t{0}, std::uint8_t{127}}));
}

TEST(PipelineFixedPoint, additive_blending_sums_source_and_destination)
{
  const auto vertices = full_screen_triangle();
  const auto stream = make_stream(vertices);

  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{GREEN}, single_precision{1}); // opaque green destination

  // Additive blend (src * 1 + dst * 1) for both RGB and alpha.
  auto state = make_state();
  state.blend.enabled = true;
  state.blend.src_rgb = BlendFactor::ONE;
  state.blend.dst_rgb = BlendFactor::ONE;
  state.blend.src_alpha = BlendFactor::ONE;
  state.blend.dst_alpha = BlendFactor::ONE;

  // Magenta added to green yields white; alpha 1 + 1 = 2 saturates (FixedPoint16 holds 2.0 exactly).
  const ConstantColorProgram program{
      Vector4F{single_precision{1}, single_precision{0}, single_precision{1}, single_precision{1}}};
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

  EXPECT_EQ(framebuffer.color_buffer().pixel(4U, 4U), Color{WHITE});
}

} // namespace
} // namespace rtw::sw_renderer
