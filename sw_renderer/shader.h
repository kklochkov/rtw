#pragma once

#include "sw_renderer/precision.h"
#include "sw_renderer/register_file.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex_stream.h"

#include <cstdint>
#include <optional>

namespace rtw::sw_renderer
{

struct VertexContext
{
  std::uint32_t vertex_id{0U};   ///< The vertex ID, as passed to the shader.
  std::uint32_t instance_id{0U}; ///< The instance ID, as passed to the shader.
};

struct FragmentContext
{
  Vector4F frag_coord;            ///< Window-space xyz + 1/w in w, as passed to the shader.
  std::uint32_t primitive_id{0U}; ///< The primitive ID, as passed to the shader.
  bool front_facing{false}; ///< Whether the fragment belongs to a front-facing primitive, as passed to the shader.
};

template <typename VaryingsT>
struct VertexShaderOutput
{
  Vector4F position;  ///< The vertex's clip-space position, as returned by the shader.
  VaryingsT varyings; ///< The vertex's varyings, as returned by the shader.
};

struct FragmentShaderOutput
{
  Vector4F color;                        ///< The fragment's output color, as returned by the shader.
  std::optional<single_precision> depth; ///< The fragment's output depth, as returned by the shader.
  bool discard{false};                   ///< Whether the fragment is discarded, as returned by the shader.
};

template <std::uint16_t CAPACITY>
class IShaderProgramGeneric
{
public:
  using DynamicVaryings = RegisterFile<single_precision, CAPACITY>;
  using VertexShaderOutput = sw_renderer::VertexShaderOutput<DynamicVaryings>;

  IShaderProgramGeneric() = default;
  IShaderProgramGeneric(const IShaderProgramGeneric&) = default;
  IShaderProgramGeneric(IShaderProgramGeneric&&) = default;
  IShaderProgramGeneric& operator=(const IShaderProgramGeneric&) = default;
  IShaderProgramGeneric& operator=(IShaderProgramGeneric&&) = default;
  virtual ~IShaderProgramGeneric() = default;

  virtual VertexShaderOutput vertex(const AttributeView& input, const VertexContext& context) const = 0;
  virtual FragmentShaderOutput fragment(const DynamicVaryings& input, const FragmentContext& context) const = 0;
};

using IShaderProgram = IShaderProgramGeneric<16U>;

} // namespace rtw::sw_renderer
