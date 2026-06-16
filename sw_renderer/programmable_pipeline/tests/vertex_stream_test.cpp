#include "sw_renderer/programmable_pipeline/vertex_stream.h"

#include "sw_renderer/programmable_pipeline/vertex_layout.h"
#include "sw_renderer/types.h"

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

/// A standard-layout vertex of all-float attributes (unambiguous byte order for the raw path).
struct PosUv
{
  sw::Vector2F position;
  sw::TexCoordF uv;
};

// --- TypedVertexStream ------------------------------------------------------

TEST(TypedVertexStreamTest, views_backing_array_without_copy)
{
  const std::array<PosUv, 2U> vertices{PosUv{{1.0F, 2.0F}, {0.25F, 0.5F}}, PosUv{{3.0F, 4.0F}, {0.75F, 1.0F}}};
  const sw::TypedVertexStream<PosUv> stream{stl::make_span(vertices)};

  EXPECT_EQ(stream.size(), 2U);
  EXPECT_FLOAT_EQ(stream[0].position.x(), 1.0F);
  EXPECT_FLOAT_EQ(stream[1].uv.v(), 1.0F);
  // The stream is a view: indexing yields references into the backing array.
  EXPECT_EQ(&stream[1], &vertices[1]);
}

TEST(TypedVertexStreamTest, constructs_from_pointer_and_count)
{
  const std::array<PosUv, 2U> vertices{PosUv{{1.0F, 2.0F}, {0.25F, 0.5F}}, PosUv{{3.0F, 4.0F}, {0.75F, 1.0F}}};
  const sw::TypedVertexStream<PosUv> stream{stl::Span<const PosUv>{vertices.data(), vertices.size()}};

  EXPECT_EQ(stream.size(), 2U);
  EXPECT_FLOAT_EQ(stream[1].position.x(), 3.0F);
}

// --- RawVertexStream / AttributeView ----------------------------------------

TEST(RawVertexStreamTest, decodes_floats_and_normalized_bytes)
{
  // One vertex: 3 floats at offset 0, then 4 normalized unsigned bytes at offset 12. Stride 16.
  std::vector<std::byte> bytes(16U);
  const std::array<float, 3U> position{1.0F, 2.0F, 3.0F};
  std::memcpy(&bytes[0U], position.data(), sizeof(position));
  const std::array<std::uint8_t, 4U> rgba{255U, 128U, 0U, 255U};
  std::memcpy(&bytes[12U], rgba.data(), rgba.size());

  const std::vector<sw::VertexAttribute> attributes{
      sw::VertexAttribute{0U, 0U, sw::ComponentType::FLOAT32, 3U},
      sw::VertexAttribute{1U, 12U, sw::ComponentType::UNORM8, 4U},
  };
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, 16U}, stl::make_span(bytes)};

  ASSERT_EQ(stream.size(), 1U);
  const auto view = stream[0U];

  // vec3 attribute promotes to (x, y, z, 1): the missing w defaults to 1.
  const auto decoded_position = view.attribute(0U);
  EXPECT_FLOAT_EQ(decoded_position.x(), 1.0F);
  EXPECT_FLOAT_EQ(decoded_position.y(), 2.0F);
  EXPECT_FLOAT_EQ(decoded_position.z(), 3.0F);
  EXPECT_FLOAT_EQ(decoded_position.w(), 1.0F);

  // Normalized unsigned bytes map [0, 255] -> [0, 1].
  const auto decoded_color = view.attribute(1U);
  EXPECT_FLOAT_EQ(decoded_color.x(), 1.0F);
  EXPECT_FLOAT_EQ(decoded_color.y(), 128.0F / 255.0F);
  EXPECT_FLOAT_EQ(decoded_color.z(), 0.0F);
  EXPECT_FLOAT_EQ(decoded_color.w(), 1.0F);
}

TEST(RawVertexStreamTest, integer_formats_keep_their_exact_value)
{
  // UINT8 (not UNORM8): bytes decode to their literal value, not a [0, 1] ratio. With count == 4 every
  // component comes from the buffer, so w is the stored 0 rather than the (0,0,0,1) default.
  std::vector<std::byte> bytes(4U);
  const std::array<std::uint8_t, 4U> raw{255U, 128U, 1U, 0U};
  std::memcpy(&bytes[0U], raw.data(), raw.size());

  const std::vector<sw::VertexAttribute> attributes{sw::VertexAttribute{0U, 0U, sw::ComponentType::UINT8, 4U}};
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, 4U}, stl::make_span(bytes)};

  const auto decoded = stream[0U].attribute(0U);
  EXPECT_FLOAT_EQ(decoded.x(), 255.0F);
  EXPECT_FLOAT_EQ(decoded.y(), 128.0F);
  EXPECT_FLOAT_EQ(decoded.z(), 1.0F);
  EXPECT_FLOAT_EQ(decoded.w(), 0.0F);
}

TEST(RawVertexStreamTest, snorm_bytes_map_to_signed_unit_range)
{
  // SNORM8 maps [-127, 127] -> [-1, 1]; the most-negative -128 clamps to -1 (the GL convention).
  std::vector<std::byte> bytes(4U);
  const std::array<std::int8_t, 4U> raw{127, 0, -64, -128};
  std::memcpy(&bytes[0U], raw.data(), raw.size());

  const std::vector<sw::VertexAttribute> attributes{sw::VertexAttribute{0U, 0U, sw::ComponentType::SNORM8, 4U}};
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, 4U}, stl::make_span(bytes)};

  const auto decoded = stream[0U].attribute(0U);
  EXPECT_FLOAT_EQ(decoded.x(), 1.0F);
  EXPECT_FLOAT_EQ(decoded.y(), 0.0F);
  EXPECT_FLOAT_EQ(decoded.z(), -64.0F / 127.0F);
  EXPECT_FLOAT_EQ(decoded.w(), -1.0F);
}

TEST(RawVertexStreamTest, missing_attribute_defaults_to_0001)
{
  std::vector<std::byte> bytes(8U);
  const std::array<float, 2U> position{5.0F, 6.0F};
  std::memcpy(&bytes[0U], position.data(), sizeof(position));

  const std::vector<sw::VertexAttribute> attributes{sw::VertexAttribute{0U, 0U, sw::ComponentType::FLOAT32, 2U}};
  const sw::RawVertexStream stream{sw::VertexLayout{attributes, 8U}, stl::make_span(bytes)};

  const auto missing = stream[0U].attribute(7U);
  EXPECT_FLOAT_EQ(missing.x(), 0.0F);
  EXPECT_FLOAT_EQ(missing.y(), 0.0F);
  EXPECT_FLOAT_EQ(missing.z(), 0.0F);
  EXPECT_FLOAT_EQ(missing.w(), 1.0F);
}

TEST(RawVertexStreamTest, consumes_a_typed_buffer_via_an_explicit_layout)
{
  const std::array<PosUv, 2U> vertices{PosUv{{1.0F, 2.0F}, {0.25F, 0.5F}}, PosUv{{3.0F, 4.0F}, {0.75F, 1.0F}}};
  // The typed buffer's bytes feed the raw path when paired with a hand-written layout over the struct.
  const sw::VertexLayout layout{
      {
          sw::VertexAttribute{0U, offsetof(PosUv, position), sw::ComponentType::FLOAT32, 2U},
          sw::VertexAttribute{1U, offsetof(PosUv, uv), sw::ComponentType::FLOAT32, 2U},
      },
      sizeof(PosUv),
  };
  const sw::RawVertexStream stream{layout, stl::as_bytes(stl::make_span(vertices))};

  ASSERT_EQ(stream.size(), 2U);

  const auto position = stream[1U].attribute(0U);
  EXPECT_FLOAT_EQ(position.x(), 3.0F);
  EXPECT_FLOAT_EQ(position.y(), 4.0F);
  EXPECT_FLOAT_EQ(position.z(), 0.0F);
  EXPECT_FLOAT_EQ(position.w(), 1.0F);

  const auto uv = stream[1U].attribute(1U);
  EXPECT_FLOAT_EQ(uv.x(), 0.75F);
  EXPECT_FLOAT_EQ(uv.y(), 1.0F);
}

// --- IndexBuffer ------------------------------------------------------------

TEST(IndexBufferTest, stores_widened_16_bit_values_as_u32)
{
  const sw::IndexBuffer indices{std::vector<std::uint32_t>{0U, 1U, 2U, 3U}};
  EXPECT_EQ(indices.size(), 4U);
  EXPECT_EQ(indices[2U], 2U);
}

TEST(IndexBufferTest, stores_32_bit_indices)
{
  const sw::IndexBuffer indices{std::vector<std::uint32_t>{100'000U, 2U, 3U}};
  EXPECT_EQ(indices.size(), 3U);
  EXPECT_EQ(indices[0U], 100'000U);
}

} // namespace
