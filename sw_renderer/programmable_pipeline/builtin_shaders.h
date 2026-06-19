#pragma once

#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/programmable_pipeline/shader.h"
#include "sw_renderer/programmable_pipeline/shader_builtins.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/types.h"

#include "math/vector.h"
#include "math/vector_operations.h"

#include <cstdint>

namespace rtw::sw_renderer
{

namespace attribute_location
{
constexpr inline std::uint32_t POSITION{0U};
constexpr inline std::uint32_t NORMAL{1U};
constexpr inline std::uint32_t UV{2U};
constexpr inline std::uint32_t COLOR{3U};
} // namespace attribute_location

class FlatColorShader : public IShaderProgram
{
public:
  void set_color(const Vector4F& color) noexcept { color_ = color; }

  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = get_mvp_matrix() * input.attribute(attribute_location::POSITION);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& /*varyings*/, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = color_;
    return out;
  }

private:
  Vector4F color_{1.0F, 1.0F, 1.0F, 1.0F};
};

class VertexColorShader : public IShaderProgram
{
public:
  constexpr static std::uint32_t COLOR_VARYING{0U};

  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = get_mvp_matrix() * input.attribute(attribute_location::POSITION);
    out.varyings[COLOR_VARYING] = input.attribute(attribute_location::COLOR);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& varyings, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = varyings[COLOR_VARYING];
    return out;
  }
};

class TexturedShader : public IShaderProgram
{
public:
  constexpr static std::uint32_t UV_VARYING{0U};

  void set_sampler(const Sampler2D& sampler) noexcept { sampler_ = sampler; }

  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = get_mvp_matrix() * input.attribute(attribute_location::POSITION);
    out.varyings[UV_VARYING] = input.attribute(attribute_location::UV);
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& varyings, const FragmentContext& /*context*/) const override
  {
    FragmentShaderOutput out;
    out.color = texture(sampler_, varyings[UV_VARYING].xy());
    return out;
  }

private:
  Sampler2D sampler_;
};

// Per-vertex (Gouraud) Lambert diffuse. The lighting term -- a normalize + dot -- is evaluated once per vertex and
// the resulting scalar intensity is interpolated across the triangle, instead of being recomputed for every
// fragment. For per-face (flat) normals the interpolated intensity is constant, so the result is identical to
// per-fragment shading while replacing a per-pixel square root (a software Newton iteration in the fixed-point
// build) with a single multiply; smooth-normal meshes get conventional Gouraud shading.
class LitShader : public IShaderProgram
{
public:
  constexpr static std::uint32_t INTENSITY_VARYING{0U};

  void set_normal_matrix(const Matrix4x4F& normal_matrix) noexcept { normal_matrix_ = normal_matrix; }
  void set_light_direction(const Vector3F& light_direction) noexcept { light_direction_ = light_direction; }
  void set_color(const Vector4F& color) noexcept { color_ = color; }

  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = get_mvp_matrix() * input.attribute(attribute_location::POSITION);

    const auto normal = input.attribute(attribute_location::NORMAL).xyz();
    const auto world_normal =
        (normal_matrix_ * Vector4F{normal.x(), normal.y(), normal.z(), single_precision{0.0F}}).xyz();
    const auto intensity = saturate(-math::dot(math::normalize(world_normal), light_direction_));
    out.varyings[INTENSITY_VARYING] =
        Vector4F{intensity, single_precision{0.0F}, single_precision{0.0F}, single_precision{0.0F}};
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& input, const FragmentContext& /*context*/) const override
  {
    const auto intensity = input[INTENSITY_VARYING].x();
    const auto rgb = color_.xyz() * intensity;

    FragmentShaderOutput out;
    out.color = Vector4F{rgb.x(), rgb.y(), rgb.z(), color_.w()};
    return out;
  }

private:
  Matrix4x4F normal_matrix_{Matrix4x4F::identity()};
  Vector3F light_direction_{0.0F, 0.0F, -1.0F};
  Vector4F color_{1.0F, 1.0F, 1.0F, 1.0F};
};

// A single composable shader that subsumes the four shaders above: it multiplies a base colour by any subset of a
// texture sample, an interpolated per-vertex colour, and per-vertex (Gouraud) Lambert diffuse lighting. Each term
// is independently toggled, so the same program covers flat colour (all off), textured, vertex-coloured, lit, and
// any combination -- matching glPolygonMode's orthogonality, every effect composes with every render mode. The
// varying slots are fixed (a disabled term simply ignores its slot) so the layout is stable regardless of which
// terms are active; they are ordered UV, light, colour so the common textured+lit path keeps to two active varyings
// (see fill_triangle_bbox's active-varying scan).
class StandardShader : public IShaderProgram
{
public:
  constexpr static std::uint32_t UV_VARYING{0U};
  constexpr static std::uint32_t LIGHT_VARYING{1U};
  constexpr static std::uint32_t COLOR_VARYING{2U};

  void set_base_color(const Vector4F& color) noexcept { base_color_ = color; }
  void set_sampler(const Sampler2D& sampler) noexcept { sampler_ = sampler; }
  void set_normal_matrix(const Matrix4x4F& normal_matrix) noexcept { normal_matrix_ = normal_matrix; }
  void set_light_direction(const Vector3F& light_direction) noexcept { light_direction_ = light_direction; }

  void set_use_texture(const bool enabled) noexcept { use_texture_ = enabled; }
  void set_use_vertex_color(const bool enabled) noexcept { use_vertex_color_ = enabled; }
  void set_use_lighting(const bool enabled) noexcept { use_lighting_ = enabled; }
  void set_point_size(const single_precision point_size) noexcept { point_size_ = point_size; }

  VertexShaderOutput vertex(const AttributeView& input, const VertexContext& /*context*/) const override
  {
    VertexShaderOutput out;
    out.position = get_mvp_matrix() * input.attribute(attribute_location::POSITION);
    out.point_size = point_size_;

    if (use_texture_)
    {
      out.varyings[UV_VARYING] = input.attribute(attribute_location::UV);
    }
    if (use_lighting_)
    {
      const auto normal = input.attribute(attribute_location::NORMAL).xyz();
      const auto world_normal =
          (normal_matrix_ * Vector4F{normal.x(), normal.y(), normal.z(), single_precision{0.0F}}).xyz();
      const auto intensity = saturate(-math::dot(math::normalize(world_normal), light_direction_));
      out.varyings[LIGHT_VARYING] =
          Vector4F{intensity, single_precision{0.0F}, single_precision{0.0F}, single_precision{0.0F}};
    }
    if (use_vertex_color_)
    {
      out.varyings[COLOR_VARYING] = input.attribute(attribute_location::COLOR);
    }
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& varyings, const FragmentContext& /*context*/) const override
  {
    auto color = base_color_;

    if (use_texture_)
    {
      color = math::hadamard(color, texture(sampler_, varyings[UV_VARYING].xy()));
    }
    if (use_vertex_color_)
    {
      color = math::hadamard(color, varyings[COLOR_VARYING]);
    }
    if (use_lighting_)
    {
      const auto intensity = varyings[LIGHT_VARYING].x();
      const auto rgb = color.xyz() * intensity;
      color = Vector4F{rgb.x(), rgb.y(), rgb.z(), color.w()};
    }

    FragmentShaderOutput out;
    out.color = color;
    return out;
  }

private:
  Vector4F base_color_{1.0F, 1.0F, 1.0F, 1.0F};
  Sampler2D sampler_;
  Matrix4x4F normal_matrix_{Matrix4x4F::identity()};
  Vector3F light_direction_{0.0F, 0.0F, -1.0F};
  bool use_texture_{false};
  bool use_vertex_color_{false};
  bool use_lighting_{false};
  single_precision point_size_{1};
};

} // namespace rtw::sw_renderer
