#pragma once

#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/types.h"

#include <cstdint>
#include <optional>

namespace rtw::sw_renderer
{

struct VertexContext
{
  std::uint32_t vertex_id{0U};
  std::uint32_t instance_id{0U};
};

struct FragmentContext
{
  Vector4F frag_coord;
  std::uint32_t primitive_id{0U};
  bool front_facing{false};
};

template <typename VaryingsT>
struct VertexShaderOutput
{
  Vector4F position;
  VaryingsT varyings;
  // GL gl_PointSize: square footprint in pixels when PolygonMode::POINT is active.
  // FILL and LINE ignore it, and the default keeps existing shaders at single-pixel points.
  single_precision point_size{1};
};

struct FragmentShaderOutput
{
  Vector4F color;
  std::optional<single_precision> depth;
  bool discard{false};
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

  void set_mvp_matrix(const Matrix4x4F& mvp) noexcept { mvp_matrix_ = mvp; }
  Matrix4x4F& get_mvp_matrix() noexcept { return mvp_matrix_; }
  const Matrix4x4F& get_mvp_matrix() const noexcept { return mvp_matrix_; }

  virtual VertexShaderOutput vertex(const AttributeView& input, const VertexContext& context) const = 0;
  virtual FragmentShaderOutput fragment(const DynamicVaryings& input, const FragmentContext& context) const = 0;

private:
  Matrix4x4F mvp_matrix_{Matrix4x4F::identity()};
};

using IShaderProgram = IShaderProgramGeneric<MAX_VARYING_COUNT>;

} // namespace rtw::sw_renderer
