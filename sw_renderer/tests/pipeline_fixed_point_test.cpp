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

class ConstantColorProgram : public IShaderProgram
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
    out.color = Vector4F{single_precision{1}, single_precision{0}, single_precision{0}, single_precision{1}};
    return out;
  }
};

TEST(PipelineFixedPoint, draw_arrays_fills_color_and_depth)
{
  const std::vector<Vertex> vertices{Vertex{{-1.0F, -1.0F, 0.0F, 1.0F}}, Vertex{{3.0F, -1.0F, 0.0F, 1.0F}},
                                     Vertex{{-1.0F, 3.0F, 0.0F, 1.0F}}};
  const VertexLayout layout{{VertexAttribute{POSITION_LOCATION, 0U, ComponentType::FLOAT32, 4U}}, sizeof(Vertex)};
  const RawVertexStream stream{layout, stl::as_bytes(stl::make_span(vertices))};

  FrameBuffer framebuffer{WIDTH, HEIGHT};
  framebuffer.clear(Color{}, single_precision{1});

  PipelineState state;
  state.viewport = Viewport{0, 0, static_cast<std::int32_t>(WIDTH), static_cast<std::int32_t>(HEIGHT)};

  const ConstantColorProgram program;
  RenderStats stats;
  Pipeline pipeline;

  pipeline.draw_arrays(program, stream, state, framebuffer, stats);

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

} // namespace
} // namespace rtw::sw_renderer
