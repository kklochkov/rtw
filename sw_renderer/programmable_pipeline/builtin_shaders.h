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

class LitShader : public IShaderProgram
{
public:
  constexpr static std::uint32_t NORMAL_VARYING{0U};

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
    out.varyings[NORMAL_VARYING] =
        Vector4F{world_normal.x(), world_normal.y(), world_normal.z(), single_precision{0.0F}};
    return out;
  }

  FragmentShaderOutput fragment(const DynamicVaryings& input, const FragmentContext& /*context*/) const override
  {
    const auto normal = math::normalize(input[NORMAL_VARYING].xyz());
    const auto intensity = saturate(-math::dot(normal, light_direction_));
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

} // namespace rtw::sw_renderer
