#include "sw_renderer/programmable_pipeline/shader.h"

#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/vertex_layout.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/types.h"

#include "multiprecision/fixed_point.h"
#include "stl/span.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

// Fixed-point coverage for the shader interface. Built via cc_test_with_fixed_point (see BUILD),
// whose config transition compiles this target with RTW_USE_FIXED_POINT so single_precision is
// FixedPoint16. It runs the stage contexts / outputs and the dynamic IShaderProgram hook in fixed-point
// mode and checks the interpolated vertex / fragment outputs; the float shader_test target carries the
// exhaustive assertions.
namespace
{

namespace sw = rtw::sw_renderer;
namespace stl = rtw::stl;

static_assert(rtw::multiprecision::IS_FIXED_POINT_V<sw::single_precision>,
              "This target must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

constexpr double TOLERANCE = 1.0e-3;

class SolidColorProgram : public sw::IShaderProgram
{
public:
  sw::IShaderProgram::VertexShaderOutput vertex(const sw::AttributeView& input,
                                                const sw::VertexContext& ctx) const override
  {
    sw::IShaderProgram::VertexShaderOutput out;
    out.position = input.attribute(0U);
    out.varyings[0U] = sw::Vector4F{static_cast<sw::single_precision>(ctx.vertex_id), sw::single_precision{0},
                                    sw::single_precision{0}, sw::single_precision{1}};
    return out;
  }

  sw::FragmentShaderOutput fragment(const sw::IShaderProgram::DynamicVaryings& input,
                                    const sw::FragmentContext& ctx) const override
  {
    sw::FragmentShaderOutput out;
    out.color = input[0U];
    out.discard = !ctx.front_facing;
    return out;
  }
};

TEST(ShaderFixedPoint, dynamic_program_instantiates_and_runs)
{
  std::vector<std::byte> bytes(sizeof(float) * 3U);
  const std::array<float, 3U> position{1.5F, 2.5F, 3.5F};
  std::memcpy(bytes.data(), position.data(), sizeof(position));

  const std::vector<sw::VertexAttribute> attributes{sw::VertexAttribute{0U, 0U, sw::ComponentType::FLOAT32, 3U}};
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, sizeof(float) * 3U}, stl::make_span(bytes)};

  const SolidColorProgram program;
  const sw::IShaderProgram& shader = program;

  const auto vout = shader.vertex(stream[0U], sw::VertexContext{2U, 0U});
  EXPECT_NEAR(static_cast<double>(vout.position.x()), 1.5, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(vout.varyings[0U].x()), 2.0, TOLERANCE);

  const auto fout = shader.fragment(vout.varyings, sw::FragmentContext{sw::Vector4F{}, 0U, true});
  EXPECT_NEAR(static_cast<double>(fout.color.x()), 2.0, TOLERANCE);
  EXPECT_FALSE(fout.discard);
}

TEST(ShaderFixedPoint, stage_outputs_instantiate)
{
  sw::VertexShaderOutput<sw::IShaderProgram::DynamicVaryings> vout;
  vout.position =
      sw::Vector4F{sw::single_precision{1}, sw::single_precision{2}, sw::single_precision{3}, sw::single_precision{1}};
  EXPECT_NEAR(static_cast<double>(vout.position.z()), 3.0, TOLERANCE);

  sw::FragmentShaderOutput fout;
  fout.color =
      sw::Vector4F{sw::single_precision{1}, sw::single_precision{0}, sw::single_precision{0}, sw::single_precision{1}};
  fout.depth = sw::single_precision{0.5F};
  EXPECT_FALSE(fout.discard);
  ASSERT_TRUE(fout.depth.has_value());
  EXPECT_NEAR(static_cast<double>(*fout.depth), 0.5, TOLERANCE);
}

} // namespace
