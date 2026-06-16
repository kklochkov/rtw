#include "sw_renderer/programmable_pipeline/vertex_layout.h"

#include "sw_renderer/color.h"
#include "sw_renderer/types.h"

#include "math/point.h"
#include "math/vector.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

namespace
{

namespace sw = rtw::sw_renderer;

/// A standard-layout vertex used to exercise explicit `VertexLayout` construction with `offsetof`.
struct TestVertex
{
  rtw::math::Point3F position;
  rtw::math::Vector3F normal;
  sw::TexCoordF uv;
  sw::Color color;
};

/// Builds the canonical explicit layout for `TestVertex`: one attribute per field, offsets via
/// `offsetof`, stride `sizeof(TestVertex)`. This is the supported way to describe a layout (there is no
/// member-pointer / offset-derivation magic).
sw::VertexLayout make_test_layout()
{
  return sw::VertexLayout{
      {
          sw::VertexAttribute{0U, offsetof(TestVertex, position), sw::ComponentType::FLOAT32, 3U},
          sw::VertexAttribute{1U, offsetof(TestVertex, normal), sw::ComponentType::FLOAT32, 3U},
          sw::VertexAttribute{2U, offsetof(TestVertex, uv), sw::ComponentType::FLOAT32, 2U},
          sw::VertexAttribute{3U, offsetof(TestVertex, color), sw::ComponentType::UNORM8, 4U},
      },
      sizeof(TestVertex),
  };
}

// --- component_byte_size ----------------------------------------------------

static_assert(sw::component_byte_size(sw::ComponentType::UNORM8) == 1U);
static_assert(sw::component_byte_size(sw::ComponentType::SNORM8) == 1U);
static_assert(sw::component_byte_size(sw::ComponentType::UINT8) == 1U);
static_assert(sw::component_byte_size(sw::ComponentType::SINT8) == 1U);
static_assert(sw::component_byte_size(sw::ComponentType::UNORM16) == 2U);
static_assert(sw::component_byte_size(sw::ComponentType::SNORM16) == 2U);
static_assert(sw::component_byte_size(sw::ComponentType::UINT16) == 2U);
static_assert(sw::component_byte_size(sw::ComponentType::SINT16) == 2U);
static_assert(sw::component_byte_size(sw::ComponentType::UINT32) == 4U);
static_assert(sw::component_byte_size(sw::ComponentType::SINT32) == 4U);
static_assert(sw::component_byte_size(sw::ComponentType::FLOAT32) == 4U);
static_assert(sw::component_byte_size(sw::ComponentType::FLOAT64) == 8U);

// --- VertexLayout -----------------------------------------------------------

TEST(VertexLayoutTest, stores_attributes_and_stride)
{
  const auto layout = make_test_layout();

  EXPECT_EQ(layout.vertex_stride(), sizeof(TestVertex));
  ASSERT_EQ(layout.attributes().size(), 4U);
  const auto& attributes = layout.attributes();

  EXPECT_EQ(attributes[0].location, 0U);
  EXPECT_EQ(attributes[0].component_type, sw::ComponentType::FLOAT32);
  EXPECT_EQ(+attributes[0].component_count, 3);
  EXPECT_EQ(attributes[0].offset, offsetof(TestVertex, position));

  EXPECT_EQ(attributes[1].location, 1U);
  EXPECT_EQ(attributes[1].offset, offsetof(TestVertex, normal));

  EXPECT_EQ(attributes[2].location, 2U);
  EXPECT_EQ(+attributes[2].component_count, 2);
  EXPECT_EQ(attributes[2].offset, offsetof(TestVertex, uv));

  EXPECT_EQ(attributes[3].location, 3U);
  EXPECT_EQ(attributes[3].component_type, sw::ComponentType::UNORM8);
  EXPECT_EQ(+attributes[3].component_count, 4);
  EXPECT_EQ(attributes[3].offset, offsetof(TestVertex, color));
}

TEST(VertexLayoutTest, find_returns_attribute_by_location)
{
  const auto layout = make_test_layout();

  const auto uv = layout.find_attribute(2U);
  ASSERT_TRUE(uv.has_value());
  EXPECT_EQ(+uv->component_count, 2);
  EXPECT_EQ(uv->offset, offsetof(TestVertex, uv));

  EXPECT_FALSE(layout.find_attribute(99U).has_value());
}

TEST(VertexLayoutTest, default_layout_is_empty)
{
  const sw::VertexLayout layout;
  EXPECT_TRUE(layout.attributes().empty());
  EXPECT_EQ(layout.vertex_stride(), 0U);
  EXPECT_FALSE(layout.find_attribute(0U).has_value());
}

} // namespace
