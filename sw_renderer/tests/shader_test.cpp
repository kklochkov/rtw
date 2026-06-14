#include "sw_renderer/shader.h"

#include "sw_renderer/register_file.h"
#include "sw_renderer/types.h"
#include "sw_renderer/varyings.h"
#include "sw_renderer/vertex_layout.h"
#include "sw_renderer/vertex_stream.h"

#include "stl/span.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace
{

namespace sw = rtw::sw_renderer;
namespace stl = rtw::stl;

/// A typed varyings overlay: one vec4 slot exposed as an RGB color.
struct TestVaryings : sw::VaryingsBase<TestVaryings, sw::single_precision, 2>
{
  sw::Vector3F color() const { return regs[0].xyz(); }
  void set_color(const sw::Vector3F& value)
  {
    regs[0] = sw::Vector4F{value.x(), value.y(), value.z(), sw::single_precision{0}};
  }
};

/// A dynamic-path program over the raw register file and `AttributeView` input: the Phase 2 hook shape.
class TestDynamicProgram : public sw::IShaderProgram
{
public:
  sw::VertexShaderOutput<sw::IShaderProgram::DynamicVaryings> vertex(const sw::AttributeView& vertex_input,
                                                                     const sw::VertexContext& ctx) const override
  {
    sw::VertexShaderOutput<sw::IShaderProgram::DynamicVaryings> out;
    out.position = vertex_input.attribute(0U);
    out.varyings[0U] = sw::Vector4F{static_cast<sw::single_precision>(ctx.vertex_id), sw::single_precision{0},
                                    sw::single_precision{0}, sw::single_precision{1}};
    return out;
  }

  sw::FragmentShaderOutput fragment(const sw::IShaderProgram::DynamicVaryings& varyings,
                                    const sw::FragmentContext& ctx) const override
  {
    sw::FragmentShaderOutput out;
    out.color = varyings[0U];
    out.discard = !ctx.front_facing;
    return out;
  }
};

// --- Contexts ---------------------------------------------------------------

TEST(ShaderContexts, vertex_context_defaults_and_aggregate_init)
{
  const sw::VertexContext defaults;
  EXPECT_EQ(defaults.vertex_id, 0U);
  EXPECT_EQ(defaults.instance_id, 0U);

  const sw::VertexContext ctx{7U, 3U};
  EXPECT_EQ(ctx.vertex_id, 7U);
  EXPECT_EQ(ctx.instance_id, 3U);
}

TEST(ShaderContexts, fragment_context_defaults_and_aggregate_init)
{
  const sw::FragmentContext defaults;
  EXPECT_EQ(defaults.frag_coord, sw::Vector4F{});
  EXPECT_FALSE(defaults.front_facing);
  EXPECT_EQ(defaults.primitive_id, 0U);

  const sw::FragmentContext ctx{sw::Vector4F{1.0F, 2.0F, 3.0F, 4.0F}, 5U, true};
  EXPECT_EQ(ctx.frag_coord, (sw::Vector4F{1.0F, 2.0F, 3.0F, 4.0F}));
  EXPECT_TRUE(ctx.front_facing);
  EXPECT_EQ(ctx.primitive_id, 5U);
}

// --- Stage outputs ----------------------------------------------------------

TEST(ShaderOutputs, vertex_output_carries_position_and_varyings)
{
  sw::VertexShaderOutput<TestVaryings> out;
  EXPECT_EQ(out.position, sw::Vector4F{});

  out.position = sw::Vector4F{1.0F, 2.0F, 3.0F, 1.0F};
  out.varyings.set_color(sw::Vector3F{0.1F, 0.2F, 0.3F});
  EXPECT_EQ(out.position, (sw::Vector4F{1.0F, 2.0F, 3.0F, 1.0F}));
  EXPECT_EQ(out.varyings.color(), (sw::Vector3F{0.1F, 0.2F, 0.3F}));
}

TEST(ShaderOutputs, fragment_output_defaults)
{
  const sw::FragmentShaderOutput out;
  EXPECT_EQ(out.color, sw::Vector4F{});
  EXPECT_FALSE(out.discard);
  EXPECT_FALSE(out.depth.has_value());
}

TEST(DynamicProgram, dispatches_through_base_interface)
{
  std::vector<std::byte> bytes(sizeof(float) * 3U);
  const std::array<float, 3U> position{1.0F, 2.0F, 3.0F};
  std::memcpy(bytes.data(), position.data(), sizeof(position));

  const std::vector<sw::VertexAttribute> attributes{sw::VertexAttribute{0U, 0U, sw::ComponentType::FLOAT32, 3U}};
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, sizeof(float) * 3U}, stl::make_span(bytes)};

  const TestDynamicProgram program;
  const sw::IShaderProgram& shader = program;

  const auto vout = shader.vertex(stream[0U], sw::VertexContext{4U, 0U});
  EXPECT_EQ(vout.position, (sw::Vector4F{1.0F, 2.0F, 3.0F, 1.0F}));
  EXPECT_EQ(vout.varyings[0U], (sw::Vector4F{4.0F, 0.0F, 0.0F, 1.0F}));

  const auto fout = shader.fragment(vout.varyings, sw::FragmentContext{sw::Vector4F{}, 0U, true});
  EXPECT_EQ(fout.color, (sw::Vector4F{4.0F, 0.0F, 0.0F, 1.0F}));
  EXPECT_FALSE(fout.discard);
}

} // namespace
