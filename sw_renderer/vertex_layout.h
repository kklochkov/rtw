#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace rtw::sw_renderer
{

enum class ComponentType : std::uint8_t
{
  UNORM8,
  SNORM8,
  UINT8,
  SINT8,
  UNORM16,
  SNORM16,
  UINT16,
  SINT16,
  UINT32,
  SINT32,
  FLOAT32,
  FLOAT64,
};

constexpr std::size_t component_byte_size(const ComponentType type) noexcept
{
  switch (type)
  {
  case ComponentType::UNORM8:
  case ComponentType::SNORM8:
  case ComponentType::UINT8:
  case ComponentType::SINT8:
    return sizeof(std::uint8_t);
  case ComponentType::UNORM16:
  case ComponentType::SNORM16:
  case ComponentType::UINT16:
  case ComponentType::SINT16:
    return sizeof(std::uint16_t);
  case ComponentType::UINT32:
  case ComponentType::SINT32:
    return sizeof(std::uint32_t);
  case ComponentType::FLOAT32:
    return sizeof(float);
  case ComponentType::FLOAT64:
    break;
  }
  return sizeof(double);
}

/// One vertex attribute: its shader location, byte offset within a vertex, and decoded format.
///
/// Layouts are built explicitly, so a caller hand-writes each attribute
/// and supplies the @ref offset itself (typically via `offsetof`):
///
/// @code
/// struct Vertex { Point3F position; Color color; };
/// const VertexLayout layout{{VertexAttribute{0U, offsetof(Vertex, position), ComponentType::FLOAT32, 3U},
///                            VertexAttribute{1U, offsetof(Vertex, color), ComponentType::UNORM8, 4U}},
///                           sizeof(Vertex)};
/// @endcode
struct VertexAttribute
{
  std::uint32_t location{0};                            ///< Shader location.
  std::uint32_t offset{0};                              ///< Byte offset from vertex start.
  ComponentType component_type{ComponentType::FLOAT32}; ///< Component data type.
  std::uint8_t component_count{0};                      ///< Number of components (1-4).
};

/// Describes how to interpret the raw bytes of a vertex buffer: an ordered set of attributes and the per-vertex stride.
class VertexLayout
{
public:
  VertexLayout() = default;
  VertexLayout(std::vector<VertexAttribute> attributes, std::size_t vertex_stride)
      : attributes_{std::move(attributes)}, vertex_stride_{vertex_stride}
  {
  }

  const std::vector<VertexAttribute>& attributes() const noexcept { return attributes_; }

  std::size_t vertex_stride() const noexcept { return vertex_stride_; }

  std::optional<VertexAttribute> find_attribute(std::uint32_t location) const noexcept
  {
    for (const auto& attr : attributes_)
    {
      if (attr.location == location)
      {
        return attr;
      }
    }
    return std::nullopt;
  }

private:
  std::vector<VertexAttribute> attributes_;
  std::size_t vertex_stride_{0U};
};

} // namespace rtw::sw_renderer
