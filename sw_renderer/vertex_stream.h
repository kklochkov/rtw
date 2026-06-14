#pragma once

#include "sw_renderer/precision.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex_layout.h"

#include "stl/span.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace rtw::sw_renderer
{

/// A read-only view of one vertex's raw bytes, decoding individual attributes on demand.
class AttributeView
{
public:
  AttributeView(const VertexLayout& layout, const stl::Span<const std::byte> vertex_data)
      : layout_{&layout}, vertex_data_{vertex_data}
  {
  }

  Vector4F attribute(const std::uint32_t location) const
  {
    Vector4F result{0.0F, 0.0F, 0.0F, 1.0F};

    const auto maybe_attribute = layout_->find_attribute(location);
    if (!maybe_attribute)
    {
      return result;
    }

    const auto component_size = component_byte_size(maybe_attribute->component_type);
    const auto count = std::min(maybe_attribute->component_count, static_cast<std::uint8_t>(4U));
    for (std::uint8_t i = 0; i < count; ++i)
    {
      const auto index = maybe_attribute->offset + i * component_size;
      result[i] = decode(maybe_attribute->component_type, index, vertex_data_);
    }

    return result;
  }

private:
  template <typename T>
  constexpr static T read(const std::size_t index, const stl::Span<const std::byte> data) noexcept
  {
    T value{};
    std::memcpy(&value, &data[index], sizeof(T));
    return value;
  }

  template <typename T>
  constexpr static single_precision decode_int(const std::size_t index, const stl::Span<const std::byte> data) noexcept
  {
    return static_cast<single_precision>(read<T>(index, data));
  }

  /// Decodes a `UNORM`/`SNORM` integer to a normalized float:
  /// - unsigned maps `[0, MAX]` to `[0, 1]` (clamping the most-positive value)
  /// - signed maps `[-MAX, MAX]` to `[-1, 1]` (clamping the most-negative value)
  template <typename T>
  constexpr static single_precision decode_norm(const std::size_t index, const stl::Span<const std::byte> data) noexcept
  {
    const auto value = decode_int<T>(index, data);
    constexpr auto MAX = static_cast<single_precision>(std::numeric_limits<T>::max());
    if constexpr (std::is_unsigned_v<T>)
    {
      return single_precision{value / MAX};
    }
    else
    {
      return single_precision{std::max(value / MAX, single_precision{-1})};
    }
  }

  constexpr static single_precision decode(const ComponentType type, const std::size_t index,
                                           const stl::Span<const std::byte> data) noexcept
  {
    switch (type)
    {
    case ComponentType::UNORM8:
      return decode_norm<std::uint8_t>(index, data);
    case ComponentType::SNORM8:
      return decode_norm<std::int8_t>(index, data);
    case ComponentType::UINT8:
      return decode_int<std::uint8_t>(index, data);
    case ComponentType::SINT8:
      return decode_int<std::int8_t>(index, data);
    case ComponentType::UNORM16:
      return decode_norm<std::uint16_t>(index, data);
    case ComponentType::SNORM16:
      return decode_norm<std::int16_t>(index, data);
    case ComponentType::UINT16:
      return decode_int<std::uint16_t>(index, data);
    case ComponentType::SINT16:
      return decode_int<std::int16_t>(index, data);
    case ComponentType::UINT32:
      return decode_int<std::uint32_t>(index, data);
    case ComponentType::SINT32:
      return decode_int<std::int32_t>(index, data);
    case ComponentType::FLOAT32:
      return read<float>(index, data);
    case ComponentType::FLOAT64:
      break;
    }
    return static_cast<single_precision>(read<double>(index, data));
  }

  const VertexLayout* layout_;
  stl::Span<const std::byte> vertex_data_;
};

/// A raw vertex buffer: a byte image plus the `VertexLayout` describing it. Indexing yields an
/// `AttributeView` over one vertex. The bytes are referenced, not owned, so the backing storage must
/// outlive the stream — a typed buffer feeds this path directly via `stl::as_bytes`.
class RawVertexStream
{
public:
  RawVertexStream(VertexLayout layout, const stl::Span<const std::byte> vertex_data)
      : layout_{std::move(layout)}, vertex_data_{vertex_data}
  {
  }

  std::size_t size() const noexcept
  {
    const auto stride = layout_.vertex_stride();
    return stride == 0U ? 0U : vertex_data_.size() / stride;
  }

  AttributeView operator[](const std::size_t index) const
  {
    const auto stride = layout_.vertex_stride();
    return AttributeView{layout_, vertex_data_.subspan(index * stride, stride)};
  }

  const VertexLayout& layout() const noexcept { return layout_; }

private:
  VertexLayout layout_;
  stl::Span<const std::byte> vertex_data_;
};

/// A typed vertex buffer: a non-owning view over an array of `V`. Indexing yields a `const V&` at zero
/// cost (no decode), which the typed vertex stage consumes directly. The same bytes can also feed the
/// raw path by pairing them with an explicit layout
/// (`RawVertexStream{stl::as_bytes(stl::make_span(vertices)), VertexLayout{...}}`).
///
/// @tparam VertexT The vertex struct type.
template <typename VertexT>
class TypedVertexStream
{
public:
  TypedVertexStream() = default;
  explicit TypedVertexStream(const stl::Span<const VertexT> vertices) : vertices_{vertices} {}

  std::size_t size() const noexcept { return vertices_.size(); }

  const VertexT& operator[](const std::size_t index) const { return vertices_[index]; }

private:
  stl::Span<const VertexT> vertices_;
};

class IndexBuffer
{
public:
  explicit IndexBuffer(std::vector<std::uint32_t> indices) : indices_{std::move(indices)} {}

  std::size_t size() const noexcept { return indices_.size(); }

  std::uint32_t operator[](const std::size_t index) const { return indices_[index]; }

private:
  std::vector<std::uint32_t> indices_;
};

} // namespace rtw::sw_renderer
