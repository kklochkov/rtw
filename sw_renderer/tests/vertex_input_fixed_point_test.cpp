#include "sw_renderer/precision.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex_layout.h"
#include "sw_renderer/vertex_stream.h"

#include "multiprecision/fixed_point.h"
#include "stl/span.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

// Fixed-point coverage for the vertex-input primitives. Built via cc_test_with_fixed_point (see
// BUILD), whose config transition compiles this target with RTW_USE_FIXED_POINT so single_precision is
// FixedPoint16. It runs the typed stream and the raw decode path in fixed-point mode and checks the
// decoded values; the float vertex_layout_test / vertex_stream_test targets carry the exhaustive
// assertions.
namespace
{

namespace sw = rtw::sw_renderer;
namespace stl = rtw::stl;

static_assert(rtw::multiprecision::IS_FIXED_POINT_V<sw::single_precision>,
              "This target must be built with RTW_USE_FIXED_POINT (built via cc_test_with_fixed_point; see BUILD)");

constexpr double TOLERANCE = 1.0e-3;

/// Construct a fixed-point scalar from a float literal.
sw::single_precision scalar(const float value) { return sw::single_precision{value}; }

/// The typed path never inspects component types, so it works with fixed-point attributes.
struct FixedVertex
{
  sw::Vector2F position;
};

TEST(VertexInputFixedPoint, typed_stream_views_fixed_point_vertices)
{
  const std::array<FixedVertex, 2U> vertices{FixedVertex{{scalar(1.5F), scalar(2.5F)}},
                                             FixedVertex{{scalar(3.5F), scalar(4.5F)}}};
  const sw::TypedVertexStream<FixedVertex> stream{stl::make_span(vertices)};

  EXPECT_EQ(stream.size(), 2U);
  EXPECT_NEAR(static_cast<double>(stream[1].position.x()), 3.5, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(stream[1].position.y()), 4.5, TOLERANCE);
}

TEST(VertexInputFixedPoint, raw_stream_decodes_into_fixed_point_vec4)
{
  std::vector<std::byte> bytes(8U);
  const std::array<float, 2U> position{1.5F, 2.5F};
  std::memcpy(&bytes[0U], position.data(), sizeof(position));

  const std::vector<sw::VertexAttribute> attributes{sw::VertexAttribute{0U, 0U, sw::ComponentType::FLOAT32, 2U}};
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, 8U}, stl::make_span(bytes)};

  const auto decoded = stream[0U].attribute(0U);
  EXPECT_NEAR(static_cast<double>(decoded.x()), 1.5, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(decoded.y()), 2.5, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(decoded.z()), 0.0, TOLERANCE);
  EXPECT_NEAR(static_cast<double>(decoded.w()), 1.0, TOLERANCE);
}

TEST(VertexInputFixedPoint, index_buffer_widens_to_u32)
{
  const sw::IndexBuffer indices{std::vector<std::uint32_t>{0U, 1U, 2U}};
  EXPECT_EQ(indices.size(), 3U);
  EXPECT_EQ(indices[1U], 1U);
}

} // namespace
